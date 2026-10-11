/*
 * OpenSLUDGE - sound_sdl.cpp
 * Copyright (C) 2000 - 2010 Tim Furnish, Rikard Peterson, Tobias Hansen
 * SDL3 version based on the OpenAL version by Tobias Hansen.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2.1, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
 * For every sound, music track and movie soundtrack that is playing, we bind
 * an SDL audio stream to the audio device, and SDL mixes the streams. On the
 * SDL audio thread, we fill each stream from a decoder for WAV, Ogg Vorbis,
 * tracker modules or movies. At the end of a decoder's data, we mark the
 * stream as finished. We handle the end of the sound later, on the main
 * thread, in updateSounds(), which we call once per frame.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <SDL3/SDL.h>
#include <dumb.h>
#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>

#include "allfiles.h"
#include "debug.h"
#include "newfatal.h"
#include "sound.h"
#include "moreio.h"
#include "fileset.h"

#define MAX_SAMPLES 8
#define MAX_MODS 3

bool soundOK = false;
bool cacheLoopySound = false;
bool SilenceIKillYou = false;

static SDL_AudioDeviceID audioDevice = 0;

int defVol = 128;
int defSoundVol = 255;
const float modLoudness = 0.95f;

/*
 * Decoders:
 */

class Decoder {
public:
	SDL_AudioSpec spec;
	virtual ~Decoder () {}
	// Fill buf with up to len bytes. Returns the number of bytes written,
	// or 0 at the end.
	virtual int read (Uint8 * buf, int len) = 0;
	// Go back to the beginning. Returns false when rewinding is not possible.
	virtual bool rewind () { return false; }
};

// We decode a WAV file completely when we open it.
class WavDecoder : public Decoder {
	Uint8 * pcm;
	Uint32 pcmLen, pos;
public:
	WavDecoder () : pcm (NULL), pcmLen (0), pos (0) {}
	~WavDecoder () { SDL_free (pcm); }
	bool open (const Uint8 * data, uint32_t length) {
		SDL_IOStream * io = SDL_IOFromConstMem (data, length);
		return io && SDL_LoadWAV_IO (io, true, &spec, &pcm, &pcmLen);
	}
	int read (Uint8 * buf, int len) {
		if ((Uint32) len > pcmLen - pos) len = pcmLen - pos;
		memcpy (buf, pcm + pos, len);
		pos += len;
		return len;
	}
	bool rewind () { pos = 0; return true; }
};

// We decode an Ogg Vorbis file a piece at a time, while the sound plays.
class OggDecoder : public Decoder {
	OggVorbis_File vf;
	bool isOpen;
	const Uint8 * data;
	size_t length, pos;

	static size_t readCallback (void * ptr, size_t size, size_t nmemb, void * source) {
		OggDecoder * d = (OggDecoder *) source;
		size_t bytes = size * nmemb;
		if (bytes > d->length - d->pos) bytes = d->length - d->pos;
		memcpy (ptr, d->data + d->pos, bytes);
		d->pos += bytes;
		return size ? bytes / size : 0;
	}
	static int seekCallback (void * source, ogg_int64_t offset, int whence) {
		OggDecoder * d = (OggDecoder *) source;
		ogg_int64_t newPos;
		switch (whence) {
			case SEEK_SET: newPos = offset; break;
			case SEEK_CUR: newPos = d->pos + offset; break;
			case SEEK_END: newPos = d->length + offset; break;
			default: return -1;
		}
		if (newPos < 0 || newPos > (ogg_int64_t) d->length) return -1;
		d->pos = (size_t) newPos;
		return 0;
	}
	static long tellCallback (void * source) {
		return (long) ((OggDecoder *) source)->pos;
	}

public:
	OggDecoder () : isOpen (false), data (NULL), length (0), pos (0) {}
	~OggDecoder () { if (isOpen) ov_clear (&vf); }
	bool open (const Uint8 * d, uint32_t len) {
		data = d;
		length = len;
		ov_callbacks callbacks = { readCallback, seekCallback, NULL, tellCallback };
		if (ov_open_callbacks (this, &vf, NULL, 0, callbacks) != 0) return false;
		isOpen = true;
		vorbis_info * info = ov_info (&vf, -1);
		if (! info) return false;
		spec.format = SDL_AUDIO_S16;
		spec.channels = info->channels;
		spec.freq = (int) info->rate;
		return true;
	}
	int read (Uint8 * buf, int len) {
		int bigEndian = (SDL_BYTEORDER == SDL_BIG_ENDIAN);
		int got = 0;
		while (got < len) {
			int bitstream;
			long r = ov_read (&vf, (char *) buf + got, len - got, bigEndian, 2, 1, &bitstream);
			if (r == OV_HOLE) continue;
			if (r <= 0) break;
			got += (int) r;
		}
		return got;
	}
	bool rewind () { return ov_raw_seek (&vf, 0) == 0; }
};

// DUMB renders tracker modules (XM, IT, S3M and MOD) a piece at a time,
// while the music plays. We loop the music forever.
class ModDecoder : public Decoder {
	DUH * duh;
	DUH_SIGRENDERER * sr;
public:
	ModDecoder () : duh (NULL), sr (NULL) {}
	~ModDecoder () {
		if (sr) duh_end_sigrenderer (sr);
		if (duh) unload_duh (duh);
	}
	bool open (const Uint8 * data, uint32_t length, int fromTrack) {
		for (int i = 0; i < 4 && ! duh; i++) {
			DUMBFILE * df = dumbfile_open_memory ((const char *) data, length);
			switch (i) {
				case 0: duh = dumb_read_xm_quick (df); break;
				case 1: duh = dumb_read_it_quick (df); break;
				case 2: duh = dumb_read_s3m_quick (df); break;
				case 3:
#if (DUMB_MAJOR_VERSION) >= 2
					duh = dumb_read_mod_quick (df, 0);
#else
					duh = dumb_read_mod_quick (df);
#endif
					break;
			}
			dumbfile_close (df);
		}
		if (! duh) return false;
		sr = dumb_it_start_at_order (duh, 2, fromTrack);
		if (! sr) return false;
		spec.format = SDL_AUDIO_S16;
		spec.channels = 2;
		spec.freq = 44100;
		return true;
	}
	int read (Uint8 * buf, int len) {
		const float volume = 0.95f;
		int samples = len / 4;	// 16 bit stereo
		int got = duh_render (sr, 16, 0, volume, 65536.0f / spec.freq, samples, buf);
		return got * 4;
	}
};

// We get movie soundtracks from the movie player in movie.cpp, through a
// callback.
class MovieDecoder : public Decoder {
	unsigned int (*callback) (void * userdata, unsigned char * data, unsigned int bytes);
public:
	MovieDecoder (unsigned int (*cb) (void *, unsigned char *, unsigned int), int channels, int samplerate) : callback (cb) {
		spec.format = SDL_AUDIO_S16;
		spec.channels = channels;
		spec.freq = samplerate;
	}
	int read (Uint8 * buf, int len) {
		return (int) callback (NULL, buf, (unsigned int) len);
	}
};

// Return a decoder for a sound file in memory, or NULL if we cannot decode it.
static Decoder * openSoundDecoder (const Uint8 * data, uint32_t length) {
	if (length >= 4 && memcmp (data, "OggS", 4) == 0) {
		OggDecoder * d = new OggDecoder;
		if (d->open (data, length)) return d;
		delete d;
		return NULL;
	}
	WavDecoder * d = new WavDecoder;
	if (d->open (data, length)) return d;
	debugOut ("Couldn't decode sound: %s\n", SDL_GetError ());
	delete d;
	return NULL;
}

/*
 * Channels, one for each audio stream that is playing:
 */

enum channelKind { SOUND_CHANNEL, MOD_CHANNEL, LIST_CHANNEL };

struct channel {
	SDL_AudioStream * stream;
	Decoder * decoder;
	channelKind kind;
	int index;				// Index into soundCache or modCache
	soundList * list;		// For LIST_CHANNEL
	bool loop;
	SDL_AtomicInt finished;	// We set this on the audio thread at the end of the decoder data
	SDL_AtomicInt paused;
};

struct soundThing {
	Uint8 * data;			// The sound file, as read from the game data
	uint32_t length;
	channel * playing;		// NULL if not playing
	int fileLoaded, vol;
	bool looping;
	// For movie soundtracks:
	unsigned int (*movieCallback) (void * userdata, unsigned char * data, unsigned int bytes);
	int movieChannels, movieSampleRate;
};

struct modThing {
	channel * playing;
};

soundThing soundCache[MAX_SAMPLES];
modThing modCache[MAX_MODS];

static void SDLCALL feedChannel (void * userdata, SDL_AudioStream * stream, int additional, int total) {
	channel * ch = (channel *) userdata;
	if (SDL_GetAtomicInt (&ch->paused) || SDL_GetAtomicInt (&ch->finished)) return;

	Uint8 buf[8192];
	int frameSize = SDL_AUDIO_FRAMESIZE (ch->decoder->spec);
	bool rewound = false;

	while (additional > 0) {
		int want = additional < (int) sizeof (buf) ? additional : (int) sizeof (buf);
		want -= want % frameSize;
		if (want <= 0) want = frameSize;
		int got = ch->decoder->read (buf, want);
		if (got <= 0) {
			// Loop, unless the sound is empty.
			if (ch->loop && ! rewound && ch->decoder->rewind ()) {
				rewound = true;
				continue;
			}
			SDL_FlushAudioStream (stream);
			SDL_SetAtomicInt (&ch->finished, 1);
			return;
		}
		rewound = false;
		SDL_PutAudioStreamData (stream, buf, got);
		additional -= got;
	}
}

static channel * startChannel (Decoder * decoder, channelKind kind, int index, bool loop, float gain, soundList * list = NULL) {
	SDL_AudioStream * stream = SDL_CreateAudioStream (&decoder->spec, NULL);
	if (! stream) {
		debugOut ("Failed to create audio stream: %s\n", SDL_GetError ());
		delete decoder;
		return NULL;
	}

	channel * ch = new channel;
	ch->stream = stream;
	ch->decoder = decoder;
	ch->kind = kind;
	ch->index = index;
	ch->list = list;
	ch->loop = loop;
	SDL_SetAtomicInt (&ch->finished, 0);
	SDL_SetAtomicInt (&ch->paused, 0);

	SDL_SetAudioStreamGain (stream, gain);
	SDL_SetAudioStreamGetCallback (stream, feedChannel, ch);
	if (! SDL_BindAudioStream (audioDevice, stream)) {
		debugOut ("Failed to play audio stream: %s\n", SDL_GetError ());
		SDL_DestroyAudioStream (stream);
		delete decoder;
		delete ch;
		return NULL;
	}
	return ch;
}

static void destroyChannel (channel * ch) {
	// After SDL_DestroyAudioStream returns, the audio thread no longer uses
	// the stream or the decoder.
	SDL_DestroyAudioStream (ch->stream);
	delete ch->decoder;
	delete ch;
}

static void soundListEnded (soundList * s, int a);

// We call this when a channel has finished playing, and when we stop it.
static void channelEnded (channel * ch) {
	int a = ch->index;
	switch (ch->kind) {
		case SOUND_CHANNEL:
			soundCache[a].playing = NULL;
			soundCache[a].looping = false;
			destroyChannel (ch);
			break;
		case MOD_CHANNEL:
			modCache[a].playing = NULL;
			destroyChannel (ch);
			break;
		case LIST_CHANNEL: {
			soundList * s = ch->list;
			soundCache[a].playing = NULL;
			soundCache[a].looping = false;
			destroyChannel (ch);
			soundListEnded (s, a);
			break;
		}
	}
}

void updateSounds () {
	if (! soundOK) return;
	for (int a = 0; a < MAX_SAMPLES; a ++) {
		channel * ch = soundCache[a].playing;
		if (ch && SDL_GetAtomicInt (&ch->finished) && SDL_GetAudioStreamQueued (ch->stream) == 0) {
			channelEnded (ch);
		}
	}
	for (int a = 0; a < MAX_MODS; a ++) {
		channel * ch = modCache[a].playing;
		if (ch && SDL_GetAtomicInt (&ch->finished) && SDL_GetAudioStreamQueued (ch->stream) == 0) {
			channelEnded (ch);
		}
	}
}

/*
 * Set up, tear down:
 */

bool initSoundStuff () {
	for (int a = 0; a < MAX_SAMPLES; a ++) {
		soundCache[a].data = NULL;
		soundCache[a].length = 0;
		soundCache[a].playing = NULL;
		soundCache[a].fileLoaded = -1;
		soundCache[a].looping = false;
		soundCache[a].movieCallback = NULL;
	}

	for (int a = 0; a < MAX_MODS; a ++) {
		modCache[a].playing = NULL;
	}

	if (! SDL_InitSubSystem (SDL_INIT_AUDIO)) {
		debugOut ("Failed to initialise audio: %s\n", SDL_GetError ());
		return false;
	}

	audioDevice = SDL_OpenAudioDevice (SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
	if (! audioDevice) {
		debugOut ("Failed to open audio device: %s\n", SDL_GetError ());
		SDL_QuitSubSystem (SDL_INIT_AUDIO);
		return false;
	}

	// SDL mixes the streams at full volume, so several loud sounds at once
	// can clip. We scale the mix down to avoid that.
	SDL_SetAudioDeviceGain (audioDevice, 0.57f);

	return soundOK = true;
}

void killSoundStuff () {
	if (! soundOK) return;

	SilenceIKillYou = true;
	for (int i = 0; i < MAX_SAMPLES; i ++) {
		if (soundCache[i].playing) channelEnded (soundCache[i].playing);
		delete [] soundCache[i].data;
		soundCache[i].data = NULL;
		soundCache[i].fileLoaded = -1;
	}
	for (int i = 0; i < MAX_MODS; i ++) {
		if (modCache[i].playing) channelEnded (modCache[i].playing);
	}
	SilenceIKillYou = false;

	SDL_CloseAudioDevice (audioDevice);
	audioDevice = 0;
	SDL_QuitSubSystem (SDL_INIT_AUDIO);
	soundOK = false;

	dumb_exit ();
}

/*
 * Some setters:
 */

void setMusicVolume (int a, int v) {
	if (! soundOK) return;

	if (modCache[a].playing) {
		SDL_SetAudioStreamGain (modCache[a].playing->stream, modLoudness * v / 255);
	}
}

void setDefaultMusicVolume (int v) {
	defVol = v;
}

void setSoundVolume (int a, int v) {
	if (! soundOK) return;
	int ch = findInSoundCache (a);
	if (ch != -1) {
		if (soundCache[ch].playing) {
			soundCache[ch].vol = v;
			SDL_SetAudioStreamGain (soundCache[ch].playing->stream, (float) v / 255);
		}
	}
}

void setDefaultSoundVolume (int v) {
	defSoundVol = v;
}

void setSoundLoop (int a, int s, int e) {
//#pragma unused (a,s,e)
}

/*
 * Stopping things:
 */

int findInSoundCache (int a) {
	int i;
	for (i = 0; i < MAX_SAMPLES; i ++) {
		if (soundCache[i].fileLoaded == a) {
			return i;
		}
	}
	return -1;
}

void stopMOD (int i) {
	if (! soundOK) return;
	if (modCache[i].playing) channelEnded (modCache[i].playing);
}

void huntKillSound (int filenum) {
	if (! soundOK) return;

	int gotSlot = findInSoundCache (filenum);
	if (gotSlot == -1) return;

	SilenceIKillYou = true;

	if (soundCache[gotSlot].playing) channelEnded (soundCache[gotSlot].playing);

	SilenceIKillYou = false;
}

void freeSound (int a) {
	if (! soundOK) return;

	SilenceIKillYou = true;

	if (soundCache[a].playing) channelEnded (soundCache[a].playing);
	delete [] soundCache[a].data;
	soundCache[a].data = NULL;
	soundCache[a].length = 0;
	soundCache[a].movieCallback = NULL;
	soundCache[a].fileLoaded = -1;

	SilenceIKillYou = false;
}


void huntKillFreeSound (int filenum) {
	if (! soundOK) return;
	int gotSlot = findInSoundCache (filenum);
	if (gotSlot == -1) return;
	freeSound (gotSlot);
}

/*
 * Loading and playing:
 */

static void playSound (int a, bool loopy, soundList * list = NULL) {
	if (! soundOK) return;
	Decoder * decoder = openSoundDecoder (soundCache[a].data, soundCache[a].length);
	if (! decoder) return;
	soundCache[a].playing = startChannel (decoder, list ? LIST_CHANNEL : SOUND_CHANNEL, a, loopy, (float) soundCache[a].vol / 255, list);
}

char * loadEntireFileToMemory (FILE * inputFile, uint32_t size) {
	char * allData = new char[size];
	if (! allData) return NULL;

	size_t bytes_read = fread (allData, size, 1, inputFile);
	if (bytes_read != 1 && ferror (inputFile)) {
		debugOut("Reading error in loadEntireFileToMemory.\n");
	}

	finishAccess ();

	return allData;
}

bool playMOD (int f, int a, int fromTrack) {
	if (! soundOK) return true;
	stopMOD (a);

	setResourceForFatal (f);
	uint32_t length = openFileFromNum (f);
	if (length == 0) {
		finishAccess();
		setResourceForFatal (-1);
		return false;
	}

	unsigned char * memImage;
	memImage = (unsigned char *) loadEntireFileToMemory (bigDataFile, length);
	if (! memImage) return fatal (ERROR_MUSIC_MEMORY_LOW);

	ModDecoder * decoder = new ModDecoder;
	bool ok = decoder->open (memImage, length, fromTrack);
	delete [] memImage;

	if (ok) {
		modCache[a].playing = startChannel (decoder, MOD_CHANNEL, a, true, modLoudness * defVol / 255);
	} else {
		delete decoder;
		debugOut("Failed to load MOD.\n");
		warning (ERROR_MUSIC_ODDNESS);
	}
	setResourceForFatal (-1);

	return true;
}

bool stillPlayingSound (int ch) {
	if (soundOK)
		if (ch != -1)
			if (soundCache[ch].fileLoaded != -1)
				if (soundCache[ch].playing)
					return true;

	return false;
}

bool forceRemoveSound () {
	for (int a = 0; a < MAX_SAMPLES; a ++) {
		if (soundCache[a].fileLoaded != -1 && ! stillPlayingSound (a)) {
//			soundWarning ("Deleting silent sound", a);
			freeSound (a);
			return 1;
		}
	}

	for (int a = 0; a < MAX_SAMPLES; a ++) {
		if (soundCache[a].fileLoaded != -1) {
//			soundWarning ("Deleting playing sound", a);
			freeSound (a);
			return 1;
		}
	}
//	soundWarning ("Cache is empty!", 0);
	return 0;
}

int emptySoundSlot = 0;

int findEmptySoundSlot () {
	int t;
	for (t = 0; t < MAX_SAMPLES; t ++) {
		emptySoundSlot ++;
		emptySoundSlot %= MAX_SAMPLES;
		if (soundCache[emptySoundSlot].fileLoaded == -1)
			return emptySoundSlot;
	}

	for (t = 0; t < MAX_SAMPLES; t ++) {
		emptySoundSlot ++;
		emptySoundSlot %= MAX_SAMPLES;
		if (!soundCache[emptySoundSlot].playing)
			return emptySoundSlot;
	}

	// Argh! They're all playing! Let's trash the oldest that's not looping...

	for (t = 0; t < MAX_SAMPLES; t ++) {
		emptySoundSlot ++;
		emptySoundSlot %= MAX_SAMPLES;
		if (! soundCache[emptySoundSlot].looping) return emptySoundSlot;
	}

	// Holy crap, they're all looping! What's this twat playing at?

	emptySoundSlot ++;
	emptySoundSlot %= MAX_SAMPLES;
	return emptySoundSlot;
}

int cacheSound (int f) {
	if (! soundOK) return -1;

	cacheLoopySound = false;

	setResourceForFatal (f);

	int a = findInSoundCache (f);
	if (a != -1) {
		// The sound is already loaded. We stop it, so that it starts again
		// from the beginning.
		if (soundCache[a].playing) channelEnded (soundCache[a].playing);
		return a;
	}
	if (f == -2) return -1;
	a = findEmptySoundSlot ();
	freeSound (a);

	uint32_t length = openFileFromNum (f);
	if (! length) return -1;

	unsigned char * memImage;

	bool tryAgain = true;

	while (tryAgain) {
		memImage = (unsigned char*)loadEntireFileToMemory (bigDataFile, length);
		tryAgain = memImage == NULL;
		if (tryAgain) {
			if (! forceRemoveSound ()) {
				fatal (ERROR_SOUND_MEMORY_LOW);
				return -1;
			}
		}
	}

	// Check that we can actually play it.
	Decoder * decoder = openSoundDecoder (memImage, length);
	if (! decoder) {
		delete [] memImage;
		warning (ERROR_SOUND_ODDNESS);
		return -1;
	}
	delete decoder;

	soundCache[a].data = memImage;
	soundCache[a].length = length;
	soundCache[a].fileLoaded = f;
	setResourceForFatal (-1);

	return a;
}

bool startSound (int f, bool loopy) {
	if (soundOK) {
		cacheLoopySound = loopy;
		int a = cacheSound (f);
		if (a == -1) {
			debugOut( "Failed to cache sound!\n");
			return false;
		}
		soundCache[a].looping = loopy;
		soundCache[a].vol = defSoundVol;

		playSound (a, loopy);
	}
	return true;
}

void saveSounds (FILE * fp) {
	if (soundOK) {
		for (int i = 0; i < MAX_SAMPLES; i ++) {
			if (soundCache[i].looping) {
				fputc (1, fp);
				put2bytes (soundCache[i].fileLoaded, fp);
				put2bytes (soundCache[i].vol, fp);
			}
		}
	}
	fputc (0, fp);
	put2bytes (defSoundVol, fp);
	put2bytes (defVol, fp);
}

void loadSounds (FILE * fp) {
	for (int i = 0; i < MAX_SAMPLES; i ++) freeSound (i);

	while (fgetc (fp)) {
		int fileLoaded = get2bytes (fp);
		defSoundVol = get2bytes (fp);
		startSound (fileLoaded, 1);
	}

	defSoundVol = get2bytes (fp);
	defVol = get2bytes (fp);
}

bool getSoundCacheStack (stackHandler * sH) {
	variable newFileHandle;
	newFileHandle.varType = SVT_NULL;

	for (int a = 0; a < MAX_SAMPLES; a ++) {
		if (soundCache[a].fileLoaded != -1) {
			setVariable (newFileHandle, SVT_FILE, soundCache[a].fileLoaded);
			if (! addVarToStackQuick (newFileHandle, sH -> first)) return false;
			if (sH -> last == NULL) sH -> last = sH -> first;
		}
	}
	return true;
}

soundList *deleteSoundFromList (soundList *s) {
	// Don't delete a playing sound.
	if (s->cacheIndex) return NULL;

	soundList * o = NULL;
	if (! s->next) {
		o = s->prev;
		if (o) o-> next = NULL;
		delete s;
		return o;
	}
	if (s != s->next) {
		o = s->next;
		o->prev = s->prev;
		if (o->prev) o->prev->next = o;
	}
	delete s;
	return o;
}

// Play the next sound in a sound list, after one has finished.
static void soundListEnded (soundList * s, int a) {
	s-> cacheIndex = false;
	if (SilenceIKillYou) {
		while ((s = deleteSoundFromList(s)));
	} else {
		if (s->next) {
			if (s->next == s) {
				int v = defSoundVol;
				defSoundVol = soundCache[a].vol;
				startSound (s->sound, true);
				defSoundVol = v;
				while ((s = deleteSoundFromList(s)));
				return;
			}
			s->next->vol = soundCache[a].vol;
			playSoundList(s->next);
		} else {
			while ((s = deleteSoundFromList(s)));
		}
	}
}

void playSoundList(soundList *s) {
	if (soundOK) {

		cacheLoopySound = true;
		int a = cacheSound (s->sound);
		if (a == -1) {
			debugOut("Failed to cache sound!\n");
			return;
		}
		soundCache[a].looping = false;
		if (s->vol < 0)
			soundCache[a].vol = defSoundVol;
		else
			soundCache[a].vol = s->vol;
		s-> cacheIndex = a;

		playSound (a, false, s);
	}
}

/*
 * Movie soundtracks:
 */

int initMovieSound(int f, int audioChannels, int samplerate,
				   unsigned int (*callback)(void *userdata, unsigned char *data, unsigned int bytes)) {
	if (! soundOK) return -1;

	int a = findEmptySoundSlot ();
	freeSound (a);

	soundCache[a].looping = false;
	soundCache[a].movieCallback = callback;
	soundCache[a].movieChannels = audioChannels;
	soundCache[a].movieSampleRate = samplerate;
	soundCache[a].fileLoaded = f;
	soundCache[a].vol = defSoundVol;

	return a;
}

void playMovieStream (int a) {
	if (! soundOK || a < 0 || ! soundCache[a].movieCallback) return;
	Decoder * decoder = new MovieDecoder (soundCache[a].movieCallback, soundCache[a].movieChannels, soundCache[a].movieSampleRate);
	soundCache[a].playing = startChannel (decoder, SOUND_CHANNEL, a, false, (float) soundCache[a].vol / 255);
}

void pauseMovieSound (int a, bool pause) {
	if (! soundOK || a < 0) return;
	if (soundCache[a].playing) SDL_SetAtomicInt (&soundCache[a].playing->paused, pause ? 1 : 0);
}
