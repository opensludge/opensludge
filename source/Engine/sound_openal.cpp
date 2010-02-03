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

struct soundThing {
	alureStream *stream;
	ALuint playingOnSource;
	bool playing;
	int fileLoaded, vol;	//Used for sounds only.
	bool looping;		//Used for sounds only.
};

soundThing soundCache[MAX_SAMPLES];
soundThing modCache[MAX_MODS];
int intpointers[MAX_SAMPLES];

int defVol = 128;
int defSoundVol = 255;
const float modLoudness = 0.95f;

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

	return soundOK = true;
}

void killSoundStuff () {
	for (int i = 0; i < MAX_SAMPLES; i ++) {
		if (soundCache[i].playing) {
			alureStopSource(soundCache[i].playingOnSource, AL_TRUE);
		}

		if (soundCache[i].stream != NULL) {
			alureDestroyStream(soundCache[i].stream, 0, NULL);
		}
	}

	for (int i = 0; i < MAX_MODS; i ++) {
		if (modCache[i].playing) {
			alureStopSource(modCache[i].playingOnSource, AL_TRUE);
		}

		if (modCache[i].stream != NULL) {
			alureDestroyStream(modCache[i].stream, 0, NULL);
		}
	}

	alureShutdownDevice();
}

/*
 * Some setters:
 */

void setMusicVolume (int a, int v) {
	if (modCache[a].playing) {
		alSourcef (modCache[a].playingOnSource, AL_GAIN, (float) modLoudness * v / 256);
	}
}

void setDefaultMusicVolume (int v) {
	defVol = v;
}

void setSoundVolume (int a, int v) {
	if (soundOK) {
		int ch = findInSoundCache (a);
		if (ch != -1) {
			if (soundCache[ch].playing) {
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
	soundCache[*a].playingOnSource = 0;
	soundCache[*a].playing = false;
	soundCache[*a].looping = false;
}

static void mod_eos_callback(void *cacheIndex, ALuint source)
{
	int *a = (int*)cacheIndex;
	alDeleteSources(1, &source);
	modCache[*a].playingOnSource = 0;
	alureDestroyStream(modCache[*a].stream, 0, NULL);
	modCache[*a].stream = NULL;
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

	if (soundCache[gotSlot].playing) {
		alureStopSource (soundCache[gotSlot].playingOnSource, AL_TRUE);
	}
}

void freeSound (int a) {
	if (soundCache[a].playing) {
		alureStopSource(soundCache[a].playingOnSource, AL_TRUE);
	}
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
	ALboolean ok;
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
		alSourcef (src, AL_GAIN, (float) modLoudness * defVol / 256);
	}

	if (loopy) {
		ok = alurePlaySourceStream(src, (*st).stream, 
				NUM_BUFS, -1, eos_callback, &intpointers[a]);
	}
	else {
		ok = alurePlaySourceStream(src, (*st).stream, 
				NUM_BUFS, 0, eos_callback, &intpointers[a]);
	}

	if(!ok) {
		fprintf(stderr, "Failed to play stream: %s\n", alureGetErrorString());
		alDeleteSources(1, &src);
		(*st).playingOnSource = 0;
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

	modCache[a].stream = alureCreateStreamFromMemory(memImage, length, 19200, 0, NULL);
	delete memImage; 

	setMusicVolume (a, defVol);
	alureSetStreamOrder (modCache[a].stream, fromTrack);

	playStream (a, true, true);
	
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
		if (soundCache[emptySoundSlot].stream == NULL)
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
	unsigned int chunkLength;
	int retval;
	bool loopy;

	loopy = cacheLoopySound;
	cacheLoopySound = false;

	setResourceForFatal (f);

	if (! soundOK) return 0;

	int a = findInSoundCache (f);
	if (a != -1) {
		if (soundCache[a].playing) {
			alureStopSource(soundCache[a].playingOnSource, AL_TRUE);
		}
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

	chunkLength = 19200;

	// Small looping sounds need small chunklengths.
	if (loopy) {
		if (length < NUM_BUFS * chunkLength) {
			chunkLength = length / NUM_BUFS;
		}
	} else if (length < chunkLength) {
		chunkLength = length;
	}

	soundCache[a].stream = alureCreateStreamFromMemory(memImage, length, chunkLength, 0, NULL);

	delete memImage; 

	if (soundCache[a].stream != NULL) {
		soundCache[a].fileLoaded = f;
		setResourceForFatal (-1);
		retval = a;
	} else {
		warning (ERROR_SOUND_ODDNESS);
		soundCache[a].stream = NULL;
		soundCache[a].playing = false;
		soundCache[a].playingOnSource = 0;
		soundCache[a].fileLoaded = -1;
		soundCache[a].looping = false;
		retval = -1;
	}

	return retval;
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

