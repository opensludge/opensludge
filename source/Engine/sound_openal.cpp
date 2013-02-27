/*
 * OpenSLUDGE - sound_openal.cpp
 * Copyright (C) Tim Furnish, Rikard Peterson, Tobias Hansen
 * This OpenAL version created 2012-13 by Rikard Peterson
 *  (partially based on earlier versions)
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
#include <stdlib.h>

#include "AL/alure.h"

#include "allfiles.h"
#include "debug.h"
#include "newfatal.h"
#include "sound.h"
#include "moreio.h"
#include "fileset.h"


#define MAX_SAMPLES 13
#define MAX_MODS 3
#define MAX_SOUNDQS 3
#define NUM_BUFS 3

// Variables from fileset.cpp, for accessing the datafile.
extern char * sludgeFile;
extern uint32_t startOfDataIndex;

bool soundOK = false;
bool SilenceIKillYou = false;

struct soundThing {
	alureStream *stream;
	ALuint playingOnSource;
	bool playing;
	int fileLoaded, vol;	//Used for sounds only.
	bool looping;			//Used for sounds only.
};

soundThing soundChannel[MAX_SAMPLES];
soundThing modChannel[MAX_MODS];
int intpointers[MAX_SAMPLES];

int defMusicVol = 128;
int defSoundVol = 255;
const float modLoudness = 0.95f;

// File IO Callbacks to enable Alure to read our big data file
struct file_forAlure {
	FILE *fp;
    int filenum;
    long l;
    long pos;
};


void * open_forAlure(const char *filename, ALuint mode) {
    fprintf (stderr, "Yay!\n");

    
    FILE * fp = fopen (sludgeFile, "rb");
    if (! fp) {
        fatal ("Can't open file ", sludgeFile);
        return 0;
    }
    
    int filenum = strtol(filename, NULL, 10);
    
	fseek (fp, startOfDataIndex + (filenum << 2), 0);
	fseek (fp, get4bytes (fp), 1);
    uint32_t length = get4bytes (fp);
    if (! length) {
        fclose (fp);
        return 0;
    }
    
    file_forAlure * h = new file_forAlure;
	if (! h) fatal("Out of memory while opening sound file.");
    h->fp = fp;
    h->l = length;
    h->filenum = filenum;
    h->pos = 0;

    fprintf (stderr, "Length: %d\n", length);

    return h;
}
void close_forAlure(void *handle) {
    file_forAlure *h = (file_forAlure *)handle;
    fclose(h->fp);
    delete h;
}
ALsizei read_forAlure(void *handle, ALubyte *buf, ALuint bytes) {
    file_forAlure *h = (file_forAlure *)handle;
    ALsizei l = 0;

    if (bytes > h->l - h->pos) bytes = h->l - h->pos;
    
    l = fread (buf, 1, bytes, h->fp);
    h->pos = h->pos + l;

    return l;
}
ALsizei write_forAlure(void *handle, const ALubyte *buf, ALuint bytes) {
    fprintf (stderr, "Trying to write!\n");
    return -1;
}
alureInt64 seek_forAlure(void *handle, alureInt64 offset, int whence) {
    file_forAlure *h = (file_forAlure *)handle;
    
    long pos;
    
    if (whence == SEEK_SET) {
        pos = offset;
    } else if (whence == SEEK_CUR) {
        pos = offset + h->pos;
    } else if (whence == SEEK_END) {
        pos = h->l + offset;
    } else {
        fprintf (stderr, "Unexpected seeking attempt!\n");
        return -1;
    }
    
   // fprintf (stderr, "Seeking to %ld of %ld.\n", pos, h->l);
    if (pos > h->l) {
        fprintf (stderr, "Seeking past end.\n");
        return -1;
    }

	fseek (h->fp, startOfDataIndex + (h->filenum << 2), 0);
	fseek (h->fp, get4bytes (h->fp)+4+pos, 1);
 
    h->pos = pos;
    
    return pos;
}


/*
 * Set up, tear down:
 */

bool initSoundStuff () {

	if(!alureInitDevice(NULL, NULL))
	{
		debugOut( "Failed to open OpenAL device: %s\n",
				alureGetErrorString());
		return 1;
	}
    
    if (!alureSetIOCallbacks(open_forAlure, close_forAlure, read_forAlure, write_forAlure, seek_forAlure)) {
		debugOut( "Failed to setAlure IO Callbacks: %s\n",
                 alureGetErrorString());
		return 1;
    }

	int a;
	for (a = 0; a < MAX_SAMPLES; a ++) {
		soundChannel[a].stream = NULL;
		soundChannel[a].playing = false;
		soundChannel[a].fileLoaded = -1;
		soundChannel[a].looping = false;
		intpointers[a] = a;
	}

	for (a = 0; a < MAX_MODS; a ++) {
		modChannel[a].stream = NULL;
		modChannel[a].playing = false;
	}

	if (! alureUpdateInterval(0.01))
	{
		debugOut("Failed to set Alure update interval: %s\n",
				alureGetErrorString());
		return 1;
	}
	return soundOK = true;
}

void killSoundStuff () {
	if (! soundOK) return;

	SilenceIKillYou = true;
	
	for (int i = 0; i < MAX_SAMPLES; i ++) {
		if (soundChannel[i].playing) {
			if (! alureStopSource(soundChannel[i].playingOnSource, AL_TRUE)) {
				debugOut( "Failed to stop source: %s\n",
							alureGetErrorString());
			}
		}

		if (soundChannel[i].stream != NULL) {
			if (! alureDestroyStream(soundChannel[i].stream, 0, NULL)) {
				debugOut("Failed to destroy stream: %s\n",
							alureGetErrorString());
			}
		}
	}

	for (int i = 0; i < MAX_MODS; i ++) {
		if (modChannel[i].playing) {
			if (! alureStopSource(modChannel[i].playingOnSource, AL_TRUE)) {
				debugOut( "Failed to stop source: %s\n",
							alureGetErrorString());
			}
		}

		if (modChannel[i].stream != NULL) {
			if (! alureDestroyStream(modChannel[i].stream, 0, NULL)) {
				debugOut("Failed to destroy stream: %s\n",
							alureGetErrorString());
			}
		}
	}

	SilenceIKillYou = false;

	alureShutdownDevice();
}



/*
 * Some setters:
 */

void setMusicVolume (int a, int v) {
	if (! soundOK) return;

	if (modChannel[a].playing) {
		alSourcef (modChannel[a].playingOnSource, AL_GAIN, (float) modLoudness * v / 256);
	}
}

void setDefaultMusicVolume (int v) {
	defMusicVol = v;
}

void setSoundVolume (int a, int v) {
	if (! soundOK) return;
	int ch = findSoundChannel (a);
	if (ch != -1) {
		if (soundChannel[ch].playing) {
			soundChannel[ch].vol = v;
			alSourcef (soundChannel[ch].playingOnSource,
					AL_GAIN, (float) v / 256);
		}
	}
}

void setDefaultSoundVolume (int v) {
	defSoundVol = v;
}

/*
 * End of stream callbacks:
 */

static void sound_eos_callback(void *ch, ALuint source)
{
	int *a = (int*)ch;
	alDeleteSources(1, &source);
	if(alGetError() != AL_NO_ERROR)
	{
		debugOut("Failed to delete OpenAL source!\n");
	}
	soundChannel[*a].playingOnSource = 0;
	soundChannel[*a].playing = false;
	soundChannel[*a].looping = false;
}

static void mod_eos_callback(void *ch, ALuint source)
{
	int *a = (int*)ch;
	alDeleteSources(1, &source);
	if(alGetError() != AL_NO_ERROR)
	{
		debugOut("Failed to delete OpenAL source!\n");
	}
	modChannel[*a].playingOnSource = 0;
	if (! alureDestroyStream(modChannel[*a].stream, 0, NULL)) {
		debugOut("Failed to destroy stream: %s\n",
				alureGetErrorString());
	}
	modChannel[*a].stream = NULL;
	modChannel[*a].playing = false;
}

/*
 * Stopping things:
 */

int findSoundChannel (int a) {
	int i;
	for (i = 3; i < MAX_SAMPLES; i ++) {
		if (soundChannel[i].fileLoaded == a) {
			return i;
		}
	}
	return -1;
}

void stopMOD (int i) {
	if (! soundOK) return;
	alGetError();
	if (modChannel[i].playing) {
		if (! alureStopSource(modChannel[i].playingOnSource, AL_TRUE)) {
			debugOut("Failed to stop source: %s\n",
						alureGetErrorString());
		}
	}
}

void freeSound (int a) {
	if (! soundOK) return;
	// Clear OpenAL errors to make sure they don't block anything:
	alGetError();
    
	SilenceIKillYou = true;
    
	if (soundChannel[a].playing) {
		if (! alureStopSource(soundChannel[a].playingOnSource, AL_TRUE)) {
			debugOut( "Failed to stop source: %s\n",
                     alureGetErrorString());
		}
	}
	if (! alureDestroyStream(soundChannel[a].stream, 0, NULL)) {
		debugOut("Failed to destroy stream: %s\n",
                 alureGetErrorString());
	}
	soundChannel[a].stream = NULL;
	soundChannel[a].fileLoaded = -1;
    
	SilenceIKillYou = false;
}

void huntKillSound (int filenum) {
	if (! soundOK) return;

	int gotSlot = findSoundChannel (filenum);
	if (gotSlot == -1) return;

	freeSound (gotSlot);
}


/*
 * Loading and playing:
 */

void playStream (int a, bool isMOD, bool loopy) {
	if (! soundOK) return;
	ALboolean ok;
	ALuint src;
	soundThing *st;
	void (*eos_callback)(void *userdata, ALuint source);
	
	if (isMOD) {
		st = &modChannel[a];
		eos_callback = mod_eos_callback;
	}
	else {
		st = &soundChannel[a];
		eos_callback = sound_eos_callback;
	}

	alGenSources(1, &src);
	if(alGetError() != AL_NO_ERROR)
	{
		debugOut( "Failed to create OpenAL source!\n");
		return;
	}

	if (isMOD) {
		alSourcef (src, AL_GAIN, (float) modLoudness * defMusicVol / 256);
	} else {
		alSourcef (src, AL_GAIN, (float) soundChannel[a].vol / 256);
	}

	if (loopy) {
		ok = alurePlaySourceStream(src, (*st).stream,
				NUM_BUFS, -1, eos_callback, &intpointers[a]);
	} else {
		ok = alurePlaySourceStream(src, (*st).stream,
				NUM_BUFS, 0, eos_callback, &intpointers[a]);
	}

	if(!ok) {
		debugOut("Failed to play stream: %s\n", alureGetErrorString());
		alDeleteSources(1, &src);
		if(alGetError() != AL_NO_ERROR)
		{
			debugOut("Failed to delete OpenAL source!\n");
		}
		(*st).playingOnSource = 0;
	} else {
		(*st).playingOnSource = src;
		(*st).playing = true;
	}
}

char * loadEntireFileToMemory (FILE * inputFile, uint32_t size) {
	char * allData = new char[size];
	if (! allData) return NULL;

	size_t bytes_read = fread (allData, size, 1, inputFile);
	if (bytes_read != size && ferror (inputFile)) {
		debugOut("Reading error in loadEntireFileToMemory.\n");
	}

	finishAccess ();

	return allData;
}

bool playMOD (int f, int a, int fromTrack) {
	if (! soundOK) return true;
	stopMOD (a);
    
    char * file = new char [7];
	if (! checkNew (file)) return false;
    snprintf(file, 7, "%d", f);
    fprintf (stderr, "%s", file);

	modChannel[a].stream = alureCreateStreamFromFile(file, 19200, 0, NULL);
    
    delete file;
    
	if (modChannel[a].stream != NULL) {
		setMusicVolume (a, defMusicVol);
		if (! alureSetStreamOrder (modChannel[a].stream, fromTrack)) {
			debugOut( "Failed to set stream order: %s\n",
						alureGetErrorString());
		}

		playStream (a, true, true);

	} else {
		debugOut("Failed to create stream from MOD: %s\n",
						alureGetErrorString());
		warning (ERROR_MUSIC_ODDNESS);
//        warning (resourceNameFromNum (f));
		soundChannel[a].stream = NULL;
		soundChannel[a].playing = false;
		soundChannel[a].playingOnSource = 0;
	}
	setResourceForFatal (-1);

	return true;
}

bool stillPlayingSound (int filenum) {
	if (soundOK) {
        int ch = findSoundChannel (filenum);
		if (ch != -1)
			if (soundChannel[ch].fileLoaded != -1)
				if (soundChannel[ch].playing)
					return true;
    }
	return false;
}

bool forceRemoveSound () {
	for (int a = 3; a < MAX_SAMPLES; a ++) {
		if (soundChannel[a].fileLoaded != -1 && ! stillPlayingSound (a)) {
//			soundWarning ("Deleting silent sound", a);
			freeSound (a);
			return 1;
		}
	}

	for (int a = 3; a < MAX_SAMPLES; a ++) {
		if (soundChannel[a].fileLoaded != -1) {
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
	for (t = 3; t < MAX_SAMPLES; t ++) {
		emptySoundSlot ++;
		emptySoundSlot %= MAX_SAMPLES;
		if (soundChannel[emptySoundSlot].stream == NULL)
			return emptySoundSlot;
	}

	for (t = 3; t < MAX_SAMPLES; t ++) {
		emptySoundSlot ++;
		emptySoundSlot %= MAX_SAMPLES;
		if (!soundChannel[emptySoundSlot].playing)
			return emptySoundSlot;
	}

	// Argh! They're all playing! Let's trash the oldest that's not looping...
	for (t = 3; t < MAX_SAMPLES; t ++) {
		emptySoundSlot ++;
		emptySoundSlot %= MAX_SAMPLES;
		if (! soundChannel[emptySoundSlot].looping) return emptySoundSlot;
	}

	// Holy crap, they're all looping! What's this twat playing at?
	emptySoundSlot ++;
	if (emptySoundSlot >= MAX_SAMPLES) emptySoundSlot = 3;
	
	return emptySoundSlot;
}



int openSoundFile (int filenum, bool loopy) {
    int retVal = 0;
 	unsigned int chunkLength = 19200;
   
	if (! soundOK) return -1;
 	setResourceForFatal (filenum);

	// Is the sound already playing?
	int a = findSoundChannel (filenum);
	if (a != -1) {
		if (soundChannel[a].playing) {
			if (! alureStopSource(soundChannel[a].playingOnSource, AL_TRUE)) {
				debugOut( "Failed to stop source: %s\n",
                         alureGetErrorString());
			}
		}
		if (! alureRewindStream (soundChannel[a].stream)) {
			debugOut( "Failed to rewind stream: %s\n",
                     alureGetErrorString());
		}
        
		return a;
	}
	a = findEmptySoundSlot ();
	freeSound (a);
    
	// Small looping sounds need small chunklengths.
    unsigned int length = openFileFromNum (filenum);
    finishAccess();
	if (loopy) {
		if (length < NUM_BUFS * chunkLength) {
			chunkLength = length / NUM_BUFS;
		}
	} else if (length < chunkLength) {
		chunkLength = length;
	}
    
    char * file = new char [7];
	if (! checkNew (file)) return false;
    snprintf(file, 7, "%d", filenum);
    fprintf (stderr, "%s", file);
    
	soundChannel[a].stream = alureCreateStreamFromFile(file, chunkLength, 0, NULL);
    
    delete file;
    
	if (soundChannel[a].stream != NULL) {
		soundChannel[a].fileLoaded = filenum;
		setResourceForFatal (-1);
		retVal = a;
	} else {
		debugOut("Failed to create stream from sound: %s\n",
                 alureGetErrorString());
		warning (ERROR_SOUND_ODDNESS);
        //warning (resourceNameFromNum (filenum));

		soundChannel[a].stream = NULL;
		soundChannel[a].playing = false;
		soundChannel[a].playingOnSource = 0;
		soundChannel[a].fileLoaded = -1;
		soundChannel[a].looping = false;
		retVal = -1;
	}
    
    return retVal;
}


bool startSound (int f, bool loopy) {
	if (soundOK) {
		int a = openSoundFile (f, loopy);
		if (a == -1) {
			debugOut( "Failed to open sound file!\n");
			return false;
		}
		soundChannel[a].looping = loopy;
		soundChannel[a].vol = defSoundVol;

		playStream (a, false, loopy);
	}
	return true;
}

void saveSounds (FILE * fp) {
	
	if (soundOK) {
		for (int i = 3; i < MAX_SAMPLES; i ++) {
			if (soundChannel[i].looping) {
				fputc (1, fp);
				put2bytes (soundChannel[i].fileLoaded, fp);
				put2bytes (soundChannel[i].vol, fp);
			}
		}
	}
	fputc (0, fp);
	put2bytes (defSoundVol, fp);
	put2bytes (defMusicVol, fp);
}

void loadSounds (FILE * fp) {
	for (int i = 3; i < MAX_SAMPLES; i ++) freeSound (i);

	while (fgetc (fp)) {
		int fileLoaded = get2bytes (fp);
		defSoundVol = get2bytes (fp);
		startSound (fileLoaded, 1);
	}

	defSoundVol = get2bytes (fp);
	defMusicVol = get2bytes (fp);
}

bool getActiveSounds (stackHandler * sH) {
	variable newFileHandle;
	newFileHandle.varType = SVT_NULL;

	for (int a = 0; a < MAX_SAMPLES; a ++) {
		if (soundChannel[a].fileLoaded != -1) {
			setVariable (newFileHandle, SVT_FILE, soundChannel[a].fileLoaded);
			if (! addVarToStackQuick (newFileHandle, sH -> first)) return false;
			if (sH -> last == NULL) sH -> last = sH -> first;
		}
	}
	return true;
}

soundList *deleteSoundFromList (soundList *s) {
	// Don't delete a playing sound.
	if (s->ch) return NULL;

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

static void list_eos_callback(void *list, ALuint source)
{
	soundList *s = (soundList *) list;

	int a = s->ch;
	alDeleteSources(1, &source);
	if(alGetError() != AL_NO_ERROR)
	{
		debugOut( "Failed to delete OpenAL source!\n");
	}
	soundChannel[a].playingOnSource = 0;
	soundChannel[a].playing = false;
	soundChannel[a].looping = false;
	s-> ch = false;
	if (SilenceIKillYou) {
		while ((s = deleteSoundFromList(s)));
	} else {
		if (s->next) {
			if (s->next == s) {
				int v = defSoundVol;
				defSoundVol = soundChannel[a].vol;
				startSound (s->sound, true);
				defSoundVol = v;
				while ((s = deleteSoundFromList(s)));
				return;
			}
			s->next->vol = soundChannel[a].vol;
			playSoundList(s->next);
		} else {
			while ((s = deleteSoundFromList(s)));
		}
	}
}


void playSoundList(soundList *s) {
	if (soundOK) {

		int a = openSoundFile (s->sound, true);
		if (a == -1) {
			debugOut("Failed to open sound file!\n");
			return;
		}
		soundChannel[a].looping = false;
		if (s->vol < 0)
			soundChannel[a].vol = defSoundVol;
		else
			soundChannel[a].vol = s->vol;
		s-> ch = a;

		ALboolean ok;
		ALuint src;
		soundThing *st;

		st = &soundChannel[a];

		alGenSources(1, &src);
		if(alGetError() != AL_NO_ERROR)
		{
			debugOut("Failed to create OpenAL source!\n");
			return;
		}

		alSourcef (src, AL_GAIN, (float) soundChannel[a].vol / 256);

		ok = alurePlaySourceStream(src, (*st).stream,
									   NUM_BUFS, 0, list_eos_callback, s);

		if(!ok) {
			debugOut("Failed to play stream: %s\n", alureGetErrorString());
			alDeleteSources(1, &src);
			if(alGetError() != AL_NO_ERROR)
			{
				debugOut("Failed to delete OpenAL source!\n");
			}
			(*st).playingOnSource = 0;
		} else {
			(*st).playingOnSource = src;
			(*st).playing = true;
		}
	}
}

void playMovieStream (int a) {
	if (! soundOK) return;
	ALboolean ok;
	ALuint src;
	
	alGenSources(1, &src);
	if(alGetError() != AL_NO_ERROR)
	{
		debugOut( "Failed to create OpenAL source!\n");
		return;
	}
	
	alSourcef (src, AL_GAIN, (float) soundChannel[a].vol / 256);
	
	ok = alurePlaySourceStream(src, soundChannel[a].stream,
								   10, 0, sound_eos_callback, &intpointers[a]);
	
	if(!ok) {
		debugOut("Failed to play stream: %s\n", alureGetErrorString());
		alDeleteSources(1, &src);
		if(alGetError() != AL_NO_ERROR)
		{
			debugOut("Failed to delete OpenAL source!\n");
		}
		soundChannel[a].playingOnSource = 0;
	} else {
		soundChannel[a].playingOnSource = src;
		soundChannel[a].playing = true;
	}
}


int initMovieSound(int f, ALenum format, int audioChannels, ALuint samplerate, 
				   ALuint (*callback)(void *userdata, ALubyte *data, ALuint bytes)) {
	if (! soundOK) return 0;

	int retval;
	int a = findEmptySoundSlot ();
	freeSound (a);
	
	soundChannel[a].looping = false;
	// audioChannel * sampleRate gives us a buffer of half a second. Not much, but it should be enough.
	soundChannel[a].stream = alureCreateStreamFromCallback(
					 callback,
					 &intpointers[a], format, samplerate,
					 audioChannels*samplerate, 0, NULL);
	
	if (soundChannel[a].stream != NULL) {
		soundChannel[a].fileLoaded = f;
		soundChannel[a].vol = defSoundVol;
		retval = a;
	} else {
		debugOut("Failed to create stream from sound: %s\n",
				 alureGetErrorString());
		warning (ERROR_SOUND_ODDNESS);
		soundChannel[a].stream = NULL;
		soundChannel[a].playing = false;
		soundChannel[a].playingOnSource = 0;
		soundChannel[a].fileLoaded = -1;
		retval = -1;
	}
	//fprintf (stderr, "Stream %d created. Sample rate: %d Channels: %d\n", retval, samplerate, audioChannels);
	
	return retval;
}

unsigned int getSoundSource(int index) {
	return soundChannel[index].playingOnSource;
}


void addSoundQ(int filenum, int ch) {}
void replaceSoundQ(int filenum, int ch) {}
void stopSoundQ(int ch) {}
void pauseSoundQ(int ch) {}
void resumeSoundQ(int ch) {}
void setSoundQLoop(int loopHow, int ch) {}
void setSoundQVolume (int volume, int ch) {}
void setDefaultSoundQVolume (int volume) {}
bool getSoundQInfo (stackHandler * sH) {
    return true;
}
bool skipSoundQ (int ch){
    return false;
}
