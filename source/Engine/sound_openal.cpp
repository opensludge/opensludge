/* 
 * OpenSLUDGE - sound_openal.cpp
 * Copyright (C) 2000 - 2010 Tim Furnish, Rikard Peterson, Tobias Hansen
 * OpenAL version created 2010 by Tobias Hansen <tobias.han@gmx.de>
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

#include <stdint.h>
#include <stdio.h>

#include "AL/alure.h"
#include <dumb.h>

#include "ALLFILES.H"
#include "NEWFATAL.H"
#include "sound.h"
#include "MOREIO.H"
#include "FILESET.H"

#define MAX_SAMPLES 8
#define MAX_MODS 3
#define NUM_BUFS 3

bool soundOK = false;
bool cacheLoopySound = false;
int modStartOrder;

struct soundThing {
	alureStream *stream;
	ALuint playingOnSource;
	bool playing;
	int fileLoaded, vol;	//Used for sounds only.
	bool looping;		//Used for sounds only.
	DUH *duh;		//Used for MODs only.
};

soundThing soundCache[MAX_SAMPLES];
soundThing modCache[MAX_MODS];
int intpointers[MAX_SAMPLES];

int defVol = 128;
int defSoundVol = 255;

/*
 * Functions for Alure to access DUMB:
 */

						// Values to choose from:
const float DUMB_volume = 0.95f;		// between 0.0f and 1.0f
const int DUMB_freq = 44100;
const int DUMB_n_channels = 2;			// 1 or 2
const int DUMB_depth = 16;			// 8 or 16
const int DUMB_unsign = 0;
const ALenum DUMB_format = AL_FORMAT_STEREO16;	// Match this to DUMB_depth
						// and DUMB_n_channels.

void * DUMBopen_memory(const ALubyte *data, ALuint length) {
	DUMBFILE *df;
	DUH *duh;
	DUH_SIGRENDERER *sr;
	int i;

	for (i = 0; i < 4; i++) {
		df = dumbfile_open_memory((const char *)data, length);
		switch (i) {
			case 0:
				duh = dumb_read_xm_quick(df);
				break;
			case 1:
				duh = dumb_read_it_quick(df);
				break;
			case 2:
				duh = dumb_read_s3m_quick(df);
				break;
			case 3:
				duh = dumb_read_mod_quick(df);
				break;
			default:
				break;
		}
		if (duh)
			break;
		dumbfile_close(df);

		if (i == 3) {
			return NULL;
		}
	}

	sr = dumb_it_start_at_order(duh, DUMB_n_channels, modStartOrder);

	return (void *) sr;
}
	
ALboolean DUMBget_format(void *instance, ALenum *format, 
				ALuint *samplerate, ALuint *blocksize) {
	*format = DUMB_format;
	*samplerate = DUMB_freq;
	*blocksize = 19200;
	return AL_TRUE;
}

ALuint DUMBdecode(void *instance, ALubyte *data, ALuint bytes) {
	float delta;
	int bufsize;

	delta = 65536.0f / DUMB_freq;
	bufsize = DUMB_depth == 16 ? bytes/2 : bytes;
	bufsize /= DUMB_n_channels;

	int l = duh_render((DUH_SIGRENDERER *)instance, DUMB_depth, DUMB_unsign,
				DUMB_volume, delta, bufsize, data);
	
	l *= DUMB_n_channels;
	l = DUMB_depth == 16 ? l*2 : l;

	return l;
}

ALboolean DUMBrewind(void *instance) {
	return AL_FALSE;
}

void DUMBclose(void *instance) {
	duh_end_sigrenderer((DUH_SIGRENDERER *)instance);
}

/*
 * Set up, tear down:
 */

bool initSoundStuff (HWND hwnd) {
	if(!alureInitDevice(NULL, NULL))
	{
		fprintf(stderr, "Failed to open OpenAL device: %s\n", 
						alureGetErrorString());
		return 1;
	}

	int a;
	for (a = 0; a < MAX_SAMPLES; a ++) {
		soundCache[a].stream = NULL;
		soundCache[a].playing = false;
		soundCache[a].fileLoaded = -1;
		soundCache[a].looping = false;
		intpointers[a] = a;
	}

	for (a = 0; a < MAX_MODS; a ++) {
		modCache[a].stream = NULL;
		modCache[a].playing = false;
	}

	atexit(&dumb_exit);
	dumb_it_max_to_mix = 256;

	if (!alureInstallDecodeCallbacks(-1, NULL, &DUMBopen_memory, 
			&DUMBget_format, &DUMBdecode, &DUMBrewind, &DUMBclose)) {
		fprintf(stderr, "Failed to install DUMB callbacks: %s\n", 
							alureGetErrorString());
	}

	return soundOK = true;
}

void killSoundStuff () {
	for (int i = 0; i < MAX_SAMPLES; i ++) {
		if (soundCache[i].playing == true) {
			alureStopSource(soundCache[i].playingOnSource, AL_TRUE);
		}

		if (soundCache[i].stream != NULL) {
			alureDestroyStream(soundCache[i].stream, 0, NULL);
		}
	}

	for (int i = 0; i < MAX_MODS; i ++) {
		if (modCache[i].playing == true) {
			alureStopSource(modCache[i].playingOnSource, AL_TRUE);
		}

		if (modCache[i].stream != NULL) {
			alureDestroyStream(modCache[i].stream, 0, NULL);
		}
	}

	alureShutdownDevice();

	dumb_exit();
}

/*
 * Some setters:
 */

void setMusicVolume (int a, int v) {
	if (modCache[a].playing == true) {
		alSourcef (modCache[a].playingOnSource, AL_GAIN, (float) v / 256);
	}
}

void setDefaultMusicVolume (int v) {
	defVol = v;
}

void setSoundVolume (int a, int v) {
	if (soundOK) {
		int ch = findInSoundCache (a);
		if (ch != -1) {
			if (soundCache[ch].playing == true) {
				alSourcef (soundCache[ch].playingOnSource, 
						AL_GAIN, (float) v / 256);
			}
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
 * End of stream callbacks:
 */

static void sound_eos_callback(void *cacheIndex, ALuint source)
{
	int *a = (int*)cacheIndex;
	alDeleteSources(1, &source);
	soundCache[*a].playing = false;
	soundCache[*a].looping = false;
}

static void mod_eos_callback(void *cacheIndex, ALuint source)
{
	int *a = (int*)cacheIndex;
	alDeleteSources(1, &source);
	alureDestroyStream(modCache[*a].stream, 0, NULL);
	modCache[*a].stream = NULL;
	unload_duh(modCache[*a].duh);
	modCache[*a].duh = NULL;
	modCache[*a].playing = false;
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
	if (modCache[i].playing) {
		alureStopSource(modCache[i].playingOnSource, AL_TRUE);
	}
}

void huntKillSound (int filenum) {
	int gotSlot = findInSoundCache (filenum);
	if (gotSlot == -1) return;

	alureStopSource (soundCache[gotSlot].playingOnSource, AL_TRUE);
}

void freeSound (int a) {
	alureStopSource(soundCache[a].playingOnSource, AL_TRUE);
	alureDestroyStream(soundCache[a].stream, 0, NULL);
	soundCache[a].stream = NULL;
	soundCache[a].fileLoaded = -1;
}


void huntKillFreeSound (int filenum) {
	int gotSlot = findInSoundCache (filenum);
	if (gotSlot == -1) return;
	freeSound (gotSlot);
}

/*
 * Loading and playing:
 */

void playStream (int a, bool isMOD, bool loopy) {
	int err;
	ALuint src;
	soundThing *st;
	void (*eos_callback)(void *userdata, ALuint source);

	if (isMOD) {
		st = &modCache[a];
		eos_callback = mod_eos_callback;
	}
	else {
		st = &soundCache[a];
		eos_callback = sound_eos_callback;
	}

	alGenSources(1, &src);
	if(alGetError() != AL_NO_ERROR)
	{
		fprintf(stderr, "Failed to create OpenAL source!\n");
		return;
	}

	if (isMOD) {
		alSourcef (src, AL_GAIN, (float) defVol / 256);
	}

	if (loopy) {
		err = alurePlaySourceStream(src, (*st).stream, 
				NUM_BUFS, -1, eos_callback, &intpointers[a]);
	}
	else {
		err = alurePlaySourceStream(src, (*st).stream, 
				NUM_BUFS, 0, eos_callback, &intpointers[a]);
	}

	if(!err) {
		fprintf(stderr, "Failed to play stream: %s\n", alureGetErrorString());
		alDeleteSources(1, &src);
	}
	else {
		(*st).playingOnSource = src;
		(*st).playing = true;
	}
}

char * loadEntireFileToMemory (FILE * inputFile, uint32_t size) {
	char * allData = new char[size];
	if (! allData) return NULL;
	fread (allData, size, 1, inputFile);
	finishAccess ();

	return allData;
}

bool playMOD (int f, int a, int fromTrack) {
	stopMOD (a);

	setResourceForFatal (f);
	uint32_t length = openFileFromNum (f);
	if (length == 0) return NULL;

	const unsigned char * memImage;
	memImage = (const unsigned char *) loadEntireFileToMemory (bigDataFile, length);
	if (! memImage) return fatal (ERROR_MUSIC_MEMORY_LOW);

	modStartOrder = fromTrack;
	modCache[a].stream = alureCreateStreamFromMemory(memImage, length, 19200, 0, NULL);

	setMusicVolume (a, defVol);

	playStream (a, true, true);
	
	return true;
}

bool stillPlayingSound (int ch) {
	if (soundOK)
		if (ch != -1)
			if (soundCache[ch].fileLoaded != -1)
				if (soundCache[ch].playing == true)
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
		if (soundCache[emptySoundSlot].stream == NULL)
			return emptySoundSlot;
	}

	for (t = 0; t < MAX_SAMPLES; t ++) {
		emptySoundSlot ++;
		emptySoundSlot %= MAX_SAMPLES;
		if (soundCache[emptySoundSlot].playing == false)
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
	int chunkLength;

	setResourceForFatal (f);

	if (! soundOK) return 0;

	int a = findInSoundCache (f);
	if (a != -1) {
		alureStopSource(soundCache[a].playingOnSource, AL_TRUE);
		alureRewindStream (soundCache[a].stream);
		return a;
	}
	if (f == -2) return -1;
	a = findEmptySoundSlot ();
	freeSound (a);

	uint32_t length = openFileFromNum (f);
	if (! length) return -1;

	const unsigned char * memImage;

	bool tryAgain = true;

	while (tryAgain) {
		memImage = (const unsigned char*)loadEntireFileToMemory (bigDataFile, length);
		tryAgain = memImage == NULL;
		if (tryAgain) {
			if (! forceRemoveSound ()) {
				fatal (ERROR_SOUND_MEMORY_LOW);
				return -1;
			}
		}
	}

//	fprintf(stdout, "Creating stream from file with length = %i\n", length);

	// Small looping sounds need small chunklengths.
	chunkLength = 19200;
	if (cacheLoopySound) {
		if (length < 51200)
			chunkLength = 1920;
		if (length < 19200)
			chunkLength = 192;
	}
	cacheLoopySound = false;

	soundCache[a].stream = alureCreateStreamFromMemory(memImage, length, chunkLength, 0, NULL);
 
	if (soundCache[a].stream != NULL) {
		soundCache[a].fileLoaded = f;
		delete memImage;
		setResourceForFatal (-1);
		return a;
	}

	warning (ERROR_SOUND_ODDNESS);
	soundCache[a].stream = NULL;
	soundCache[a].playing = false;
	soundCache[a].fileLoaded = -1;
	soundCache[a].looping = false;
	return -1;
}

bool startSound (int f, bool loopy) {	
	if (soundOK) {
		cacheLoopySound = loopy;
		int a = cacheSound (f);
		if (a == -1) {
			fprintf(stderr, "Failed to cache sound!\n");
			return false;
		}
		soundCache[a].looping = loopy;
		soundCache[a].vol = defSoundVol;

		playStream (a, false, loopy);
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

