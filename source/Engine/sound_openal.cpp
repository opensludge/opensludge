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


#define MAX_MODS 3
#define MAX_SAMPLES 10
#define MAX_SOUNDQS 3
#define SAMPLE_CHAN0 MAX_MODS
#define SQ_CHAN0 SAMPLE_CHAN0+MAX_SAMPLES
#define TOTAL_CHANNELS MAX_MODS+MAX_SAMPLES+MAX_SOUNDQS
#define NUM_BUFS 3

// Variables from fileset.cpp, for accessing the datafile.
extern char * sludgeFile;
extern uint32_t startOfDataIndex;

bool soundOK = false;
bool SilenceIKillYou = false;

struct soundQThing {
    int file;
	struct soundQThing * next;
	struct soundQThing * prev;
};

struct soundThing {
	alureStream *stream;
	ALuint playingOnSource;
	bool playing;
	bool looping;
    int vol;			// Not used for mods.
	int fileLoaded;             // Used for sounds only.

    soundQThing *soundQ;        // for soundQ only
};


soundThing soundChannel[TOTAL_CHANNELS];
int intpointers[TOTAL_CHANNELS];

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

void playStream (int ch, bool isMOD, bool loopy);

void * open_forAlure(const char *filename, ALuint mode) {
    
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
    fprintf (stderr, "Alure is trying to write to the datafile! I'm not letting it.\n");
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
        fprintf (stderr, "Alure is trying an unexpected seeking attempt!\n");
        return -1;
    }
    
   // fprintf (stderr, "Seeking to %ld of %ld.\n", pos, h->l);
    if (pos > h->l) {
        fprintf (stderr, "Alure is seeking past end.\n");
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
	for (a = 0; a < TOTAL_CHANNELS; a ++) {
		soundChannel[a].stream = NULL;
		soundChannel[a].playing = false;
		soundChannel[a].fileLoaded = -1;
		soundChannel[a].looping = false;
        soundChannel[a].soundQ = NULL;
        soundChannel[a].vol = defSoundVol;
		intpointers[a] = a;
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
	
	for (int i = 0; i < TOTAL_CHANNELS; i ++) {
		if (soundChannel[i].playing) {
			if (! alureStopSource(soundChannel[i].playingOnSource, AL_TRUE)) {
				debugOut( "Failed to stop source: %s\n",
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

void setMusicVolume (int ch, int v) {
	if (! soundOK) return;
    if (ch >= MAX_MODS || ch < 0) return;

	if (soundChannel[ch].playing) {
		alSourcef (soundChannel[ch].playingOnSource, AL_GAIN, (float) modLoudness * v / 256);
	}
}

void setDefaultMusicVolume (int v) {
	defMusicVol = v;
}

void setSoundVolume (int fileNum, int v) {
	if (! soundOK) return;
	int ch = findSoundChannel (fileNum);
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
    soundThing *channel = &soundChannel[*a];
    
    // Is it a non-queued sound? Just clean up.
    if (*a < SQ_CHAN0) {
        alDeleteSources(1, &source);
        if(alGetError() != AL_NO_ERROR)
        {
            debugOut("Failed to delete OpenAL source!\n");
        }
        if (! alureDestroyStream(channel->stream, 0, NULL)) {
            debugOut("Failed to destroy stream: %s\n", alureGetErrorString());
        }
        channel->playingOnSource = 0;
        channel->playing = false;
        channel->looping = false;
        return;
    }
    
    // Ok, so we have a queue!
    soundQThing * qt = channel->soundQ;
    if (!qt) fatal("Something is very messed up with the sound queue internals. This error should never happen: You have found a bug in the SLUDGE engine.");
    
    if (SilenceIKillYou) {
        // Kill the queue
        alDeleteSources(1, &source);
        if(alGetError() != AL_NO_ERROR) {
            debugOut("Failed to delete OpenAL source!\n");
        }
        if (! alureDestroyStream(channel->stream, 0, NULL)) {
            debugOut("Failed to destroy stream: %s\n", alureGetErrorString());
        }
        soundQThing * next;
        while ((next = qt->next)) {
            delete qt;
            qt = next;
        }
        delete qt;
        channel->soundQ = NULL;
        channel->stream = NULL;
        channel->playing = false;
        channel->playingOnSource = 0;
        channel->fileLoaded = -1;
        
    } else {
        if (qt->next) {
            channel->soundQ = qt->next;
            alDeleteSources(1, &source);
            if(alGetError() != AL_NO_ERROR) {
                debugOut("Failed to delete OpenAL source!\n");
            }
            if (! alureDestroyStream(channel->stream, 0, NULL)) {
                debugOut("Failed to destroy stream: %s\n", alureGetErrorString());
            }
        } else if (channel->looping == 2) {
            alureRewindStream(channel->stream);
            playStream (*a, false, false);
            return;
        } else if (channel->looping) {
            soundQThing *first = qt;
            while (first->prev)
                first = first->prev;
            channel->soundQ = first;
            if (first == qt) {
                alureRewindStream(channel->stream);
                playStream (*a, false, false);
                return;               
            }
            alDeleteSources(1, &source);
            if(alGetError() != AL_NO_ERROR) {
                debugOut("Failed to delete OpenAL source!\n");
            }
            if (! alureDestroyStream(channel->stream, 0, NULL)) {
                debugOut("Failed to destroy stream: %s\n",
                         alureGetErrorString());
            }
           
        } else {
            // End of queue - let's clean up
            alDeleteSources(1, &source);
            if(alGetError() != AL_NO_ERROR) {
                debugOut("Failed to delete OpenAL source!\n");
            }
            if (! alureDestroyStream(channel->stream, 0, NULL)) {
                debugOut("Failed to destroy stream: %s\n", alureGetErrorString());
            }

            channel->soundQ = NULL;
            channel->stream = NULL;
            channel->playing = false;
            channel->playingOnSource = 0;
            channel->fileLoaded = -1;            
            delete qt;
            return;
        }
        
        int filenum = channel->soundQ->file;
        setResourceForFatal(filenum);
        unsigned int chunkLength = 19200;

        char * file = new char [7];
        if (! checkNew (file)) return;
        snprintf(file, 7, "%d", filenum);
        channel->stream = alureCreateStreamFromFile(file, chunkLength, 0, NULL);
        
        delete file;
        
        if (channel->stream != NULL) {
            channel->fileLoaded = filenum;
            playStream (*a, false, false);
        } else {
            debugOut("Failed to create stream from sound: %s\n",
                     alureGetErrorString());
            warning (ERROR_SOUND_ODDNESS);
            
            channel->stream = NULL;
            channel->playing = false;
            channel->playingOnSource = 0;
            channel->fileLoaded = -1;
        }
        
        if (channel->looping != 1) {
            channel->soundQ->prev = NULL;
            delete qt;
        }
        setResourceForFatal (-1);
    }
}


static void mod_eos_callback(void *ch, ALuint source)
{
	int *a = (int*)ch;
	alDeleteSources(1, &source);
	if(alGetError() != AL_NO_ERROR)
	{
		debugOut("Failed to delete OpenAL source!\n");
	}
	soundChannel[*a].playingOnSource = 0;
	if (! alureDestroyStream(soundChannel[*a].stream, 0, NULL)) {
		debugOut("Failed to destroy stream: %s\n", alureGetErrorString());
	}
	soundChannel[*a].stream = NULL;
	soundChannel[*a].playing = false;
}

/*
 * Stopping things:
 */

int findSoundChannel (int filenum) {
	int i;
	for (i = SAMPLE_CHAN0; i < MAX_SAMPLES + SAMPLE_CHAN0; i ++) {
		if (soundChannel[i].fileLoaded == filenum) {
			return i;
		}
	}
	return -1;
}

void stopMOD (int ch) {
	if (! soundOK) return;
    if (ch >= MAX_MODS || ch < 0) return;

	alGetError();
	if (soundChannel[ch].playing) {
		if (! alureStopSource(soundChannel[ch].playingOnSource, AL_TRUE)) {
			debugOut("Failed to stop source: %s\n",
						alureGetErrorString());
		}
	}
}

void freeSound (int ch) {
	if (! soundOK) return;
	// Clear OpenAL errors to make sure they don't block anything:
	alGetError();
    
	SilenceIKillYou = true;
    
	if (soundChannel[ch].playing) {
		if (! alureStopSource(soundChannel[ch].playingOnSource, AL_TRUE)) {
			debugOut( "Failed to stop source: %s\n",
                     alureGetErrorString());
		}
	}
	soundChannel[ch].stream = NULL;
	soundChannel[ch].fileLoaded = -1;
    
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

void playStream (int ch, bool isMOD, bool loopy) {
	if (! soundOK) return;
	ALboolean ok;
	ALuint src;
	void (*eos_callback)(void *userdata, ALuint source);
	
	if (isMOD) {
		eos_callback = mod_eos_callback;
	} else {
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
		alSourcef (src, AL_GAIN, (float) soundChannel[ch].vol / 256);
        
	}

    ok = alurePlaySourceStream(src, soundChannel[ch].stream,
                                NUM_BUFS, (loopy)? -1: 0, eos_callback, &intpointers[ch]);

	if(!ok) {
		debugOut("Failed to play stream: %s\n", alureGetErrorString());
		alDeleteSources(1, &src);
		if(alGetError() != AL_NO_ERROR) {
			debugOut("Failed to delete OpenAL source!\n");
		}
		soundChannel[ch].playingOnSource = 0;
	} else {
		soundChannel[ch].playingOnSource = src;
		soundChannel[ch].playing = true;
 	}
}

bool playMOD (int f, int ch, int fromTrack) {
	if (! soundOK) return true;
	stopMOD (ch);
    
	setResourceForFatal (f);
    if (ch >= MAX_MODS || ch < 0) return false;
    
    char * file = new char [7];
	if (! checkNew (file)) return false;
    snprintf(file, 7, "%d", f);
	soundChannel[ch].stream = alureCreateStreamFromFile(file, 19200, 0, NULL);
    
    delete file;
    
	if (soundChannel[ch].stream != NULL) {
		setMusicVolume (ch, defMusicVol);
		if (! alureSetStreamOrder (soundChannel[ch].stream, fromTrack)) {
			debugOut( "Failed to set stream order: %s\n",
						alureGetErrorString());
		}

		playStream (ch, true, true);

	} else {
		debugOut("Failed to create stream from MOD: %s\n",
						alureGetErrorString());
		warning (ERROR_MUSIC_ODDNESS);
//        warning (resourceNameFromNum (f));
		soundChannel[ch].stream = NULL;
		soundChannel[ch].playing = false;
		soundChannel[ch].playingOnSource = 0;
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

int emptySoundSlot = 0;

int findEmptySoundSlot () {
	int t;
	for (t = 0; t < MAX_SAMPLES; t ++) {
		emptySoundSlot ++;
		emptySoundSlot %= MAX_SAMPLES;
		if (soundChannel[emptySoundSlot+SAMPLE_CHAN0].stream == NULL)
			return emptySoundSlot+SAMPLE_CHAN0;
	}

	for (t = 0; t < MAX_SAMPLES; t ++) {
		emptySoundSlot ++;
		emptySoundSlot %= MAX_SAMPLES;
		if (!soundChannel[emptySoundSlot+SAMPLE_CHAN0].playing)
			return emptySoundSlot+SAMPLE_CHAN0;
	}

	// Argh! They're all playing! Let's trash the oldest that's not looping...
	for (t = 0; t < MAX_SAMPLES; t ++) {
		emptySoundSlot ++;
		emptySoundSlot %= MAX_SAMPLES;
		if (! soundChannel[emptySoundSlot+SAMPLE_CHAN0].looping) return emptySoundSlot+SAMPLE_CHAN0;
	}

	// Holy crap, they're all looping! What's this twat playing at?
	emptySoundSlot ++;
	if (emptySoundSlot >= MAX_SAMPLES) emptySoundSlot = 0;
	
	return emptySoundSlot + SAMPLE_CHAN0;
}



int openSoundFile (int filenum, bool loopy) {
    int retVal = 0;
 	unsigned int chunkLength = 19200;
   
	if (! soundOK) return -1;
 	setResourceForFatal (filenum);

	// Is the sound already playing?
	int ch = findSoundChannel (filenum);
	if (ch != -1) {
		if (soundChannel[ch].playing) {
			if (! alureStopSource(soundChannel[ch].playingOnSource, AL_TRUE)) {
				debugOut( "Failed to stop source: %s\n",
                         alureGetErrorString());
			}
		}
		if (! alureRewindStream (soundChannel[ch].stream)) {
			debugOut( "Failed to rewind stream: %s\n",
                     alureGetErrorString());
		}
        
		return ch;
	}
	ch = findEmptySoundSlot ();
	freeSound (ch);
    
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
	soundChannel[ch].stream = alureCreateStreamFromFile(file, chunkLength, 0, NULL);
    
    delete file;
    
	if (soundChannel[ch].stream != NULL) {
		soundChannel[ch].fileLoaded = filenum;
		setResourceForFatal (-1);
		retVal = ch;
	} else {
		debugOut("Failed to create stream from sound: %s\n",
                 alureGetErrorString());
		warning (ERROR_SOUND_ODDNESS);
        //warning (resourceNameFromNum (filenum));

		soundChannel[ch].stream = NULL;
		soundChannel[ch].playing = false;
		soundChannel[ch].playingOnSource = 0;
		soundChannel[ch].fileLoaded = -1;
		soundChannel[ch].looping = false;
		retVal = -1;
	}
    
    return retVal;
}


bool startSound (int f, bool loopy) {
	if (soundOK) {
		int ch = openSoundFile (f, loopy);
		if (ch == -1) {
			debugOut( "Failed to open sound file!\n");
			return false;
		}
		soundChannel[ch].looping = loopy;
		soundChannel[ch].vol = defSoundVol;

		playStream (ch, false, loopy);
	}
	return true;
}

void saveSounds (FILE * fp) {
	
	if (soundOK) {
		for (int i = SAMPLE_CHAN0; i < MAX_SAMPLES+SAMPLE_CHAN0; i ++) {
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
	for (int i = SAMPLE_CHAN0; i < MAX_SAMPLES+SAMPLE_CHAN0; i ++) freeSound (i);

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

	for (int a = SAMPLE_CHAN0; a < MAX_SAMPLES+SAMPLE_CHAN0; a ++) {
		if (soundChannel[a].fileLoaded != -1) {
			setVariable (newFileHandle, SVT_FILE, soundChannel[a].fileLoaded);
			if (! addVarToStackQuick (newFileHandle, sH -> first)) return false;
			if (sH -> last == NULL) sH -> last = sH -> first;
		}
	}
	return true;
}

void playMovieStream (int ch) {
	if (! soundOK) return;
	ALboolean ok;
	ALuint src;
	
	alGenSources(1, &src);
	if(alGetError() != AL_NO_ERROR)
	{
		debugOut( "Failed to create OpenAL source!\n");
		return;
	}
	
	alSourcef (src, AL_GAIN, (float) soundChannel[ch].vol / 256);
	
	ok = alurePlaySourceStream(src, soundChannel[ch].stream,
								   10, 0, sound_eos_callback, &intpointers[ch]);
	
	if(!ok) {
		debugOut("Failed to play stream: %s\n", alureGetErrorString());
		alDeleteSources(1, &src);
		if(alGetError() != AL_NO_ERROR)
		{
			debugOut("Failed to delete OpenAL source!\n");
		}
		soundChannel[ch].playingOnSource = 0;
	} else {
		soundChannel[ch].playingOnSource = src;
		soundChannel[ch].playing = true;
	}
}


int initMovieSound(int f, ALenum format, int audioChannels, ALuint samplerate, 
				   ALuint (*callback)(void *userdata, ALubyte *data, ALuint bytes)) {
	if (! soundOK) return 0;

	int retval;
	int ch = findEmptySoundSlot ();
	freeSound (ch);
	
	soundChannel[ch].looping = false;
	// audioChannel * sampleRate gives us a buffer of half a second. Not much, but it should be enough.
	soundChannel[ch].stream = alureCreateStreamFromCallback(
					 callback,
					 &intpointers[ch], format, samplerate,
					 audioChannels*samplerate, 0, NULL);
	
	if (soundChannel[ch].stream != NULL) {
		soundChannel[ch].fileLoaded = f;
		soundChannel[ch].vol = defSoundVol;
		retval = ch;
	} else {
		debugOut("Failed to create stream from sound: %s\n",
				 alureGetErrorString());
		warning (ERROR_SOUND_ODDNESS);
		soundChannel[ch].stream = NULL;
		soundChannel[ch].playing = false;
		soundChannel[ch].playingOnSource = 0;
		soundChannel[ch].fileLoaded = -1;
		retval = -1;
	}
	//fprintf (stderr, "Stream %d created. Sample rate: %d Channels: %d\n", retval, samplerate, audioChannels);
	
	return retval;
}

unsigned int getSoundSource(int index) {
	return soundChannel[index].playingOnSource;
}


void addSoundQ(int filenum, int ch) {
	if (! soundOK) return;
    if (ch >= MAX_SOUNDQS || ch < 0) return;
    ch += SQ_CHAN0;
    
 	unsigned int chunkLength = 19200;
    
 	setResourceForFatal (filenum);
    
    soundQThing *qt = new soundQThing;
	if (! checkNew (qt)) return;
    qt->next = NULL;
    qt->file = filenum;
    
	// Is the channel busy? Then we should queue the song. 
    if (soundChannel[ch].playing) {
        soundQThing *last = soundChannel[ch].soundQ;
        while (last->next)
            last = last->next;
        last->next = qt;
        qt->prev = last;
        return;
	}
    
    qt->prev = NULL;
    
	/* Small looping sounds need small chunklengths.
    unsigned int length = openFileFromNum (filenum);
    finishAccess();
	if (soundChannel[ch].looping) {
		if (length < NUM_BUFS * chunkLength) {
			chunkLength = length / NUM_BUFS;
		}
	} else if (length < chunkLength) {
		chunkLength = length;
	}*/
    
    char * file = new char [7];
	if (! checkNew (file)) return;
    snprintf(file, 7, "%d", filenum);
	soundChannel[ch].stream = alureCreateStreamFromFile(file, chunkLength, 0, NULL);
    
    delete file;
    
	if (soundChannel[ch].stream != NULL) {
		soundChannel[ch].fileLoaded = filenum;
        soundChannel[ch].soundQ = qt;
        playStream (ch, false, false);
	} else {
		debugOut("Failed to create stream from sound: %s\n",
                 alureGetErrorString());
		warning (ERROR_SOUND_ODDNESS);
        
		soundChannel[ch].stream = NULL;
		soundChannel[ch].playing = false;
		soundChannel[ch].playingOnSource = 0;
		soundChannel[ch].fileLoaded = -1;
	}
    
    setResourceForFatal (-1);
}
void replaceSoundQ(int filenum, int ch) {
    if (ch >= MAX_SOUNDQS || ch < 0) return;
    ch += SQ_CHAN0;
    
    // First delete any existing queue
    soundQThing * next, *qt;
    if ((qt = soundChannel[ch].soundQ)) {
        if (qt->next) {
            qt = qt->next;
            soundChannel[ch].soundQ->next = NULL;
            while ((next = qt->next)) {
                delete qt;
                qt = next;
            }
            delete qt;
        }
    }

    // Then add the new sound
    addSoundQ(filenum, ch-SQ_CHAN0);
}
void stopSoundQ(int ch) {
    if (ch >= MAX_SOUNDQS || ch < 0) return;
    ch += SQ_CHAN0;
    
    freeSound(ch);
}

void pauseSoundQ(int ch) {
    if (ch >= MAX_SOUNDQS || ch < 0) return;
    ch += SQ_CHAN0;
    ALuint source = getSoundSource(ch);
    if (source) {
        alurePauseSource(source);
    }
}
void resumeSoundQ(int ch) {
    if (ch >= MAX_SOUNDQS || ch < 0) return;
    ch += SQ_CHAN0;
    ALuint source = getSoundSource(ch);
    if (source) {
        alureResumeSource(source);
    }
}

void setSoundQLoop(int loopHow, int ch) {
    if (ch >= MAX_SOUNDQS || ch < 0) return;
    ch += SQ_CHAN0;
    
    /* loopHow  -------------------------*\
     |  0 = Don't loop. Remove sounds from queue as they're finished.
     |  1 = Loop the whole thing         |
     |  2 = Loop the last segment only.  |
    \*-----------------------------------*/
    soundChannel[ch].looping = loopHow;
}

void setSoundQVolume (int volume, int ch) {
    if (! soundOK) return;
    if (ch >= MAX_SOUNDQS || ch < 0) return;
    ch += SQ_CHAN0;

    soundChannel[ch].vol = volume;
    if (soundChannel[ch].playing)
        alSourcef (soundChannel[ch].playingOnSource, AL_GAIN, (float) volume / 256);
}

bool getSoundQInfo (stackHandler * sH, int ch) {
    if (ch >= MAX_SOUNDQS || ch < 0) return false;
    ch += SQ_CHAN0;
    
    // TODO

    return true;
}
int skipSoundQ (int ch){
    int retVal = false;
    
    if (ch >= MAX_SOUNDQS || ch < 0) return false;
    ch += SQ_CHAN0;
    
	if (! soundOK) return false;
    
    if (soundChannel[ch].soundQ->next) retVal = soundChannel[ch].soundQ->next->file;
    
	// Clear OpenAL errors to make sure they don't block anything:
	alGetError();
        
	if (soundChannel[ch].playing) {
		if (! alureStopSource(soundChannel[ch].playingOnSource, AL_TRUE)) {
			debugOut( "Failed to stop source: %s\n",
                     alureGetErrorString());
		}
	}
    return retVal;
}

