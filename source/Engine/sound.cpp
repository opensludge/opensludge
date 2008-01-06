#include "allfiles.h"
#include "newfatal.h"
#include "fmod.h"
#include "sound.h"
#include "moreio.h"
#include "fileset.h"

#define MAX_SAMPLES 8
#define MAX_MODS 3
#define EFFECT_CHANNELS 8
#define TOTAL_CHANNELS 32

BOOL soundOK = FALSE;

struct soundThing {
	FSOUND_SAMPLE * sample;
	int fileLoaded, vol;
	int mostRecentChannel;
	bool looping;
};

FMUSIC_MODULE * mod[MAX_MODS];

soundThing soundCache[MAX_SAMPLES];

int defVol = 128;
int defSoundVol = 255;

char * loadEntireFileToMemory (FILE * inputFile, unsigned long size) {
	char * allData = new char[size];
	if (! allData) return NULL;
	fread (allData, size, 1, inputFile);
	finishAccess ();

	return allData;
}

void stopMOD (int i) {
	if (mod[i]) {
		FMUSIC_StopSong(mod[i]);
		FMUSIC_FreeSong(mod[i]);
		mod[i] = NULL;
	}
}

int findInSoundCache (int a) {
	int i;
	for (i = 0; i < MAX_SAMPLES; i ++) {
		if (soundCache[i].fileLoaded == a) return i;
	}
	return -1;
}

void huntKillSound (int filenum) {
	int gotSlot = findInSoundCache (filenum);
	if (gotSlot == -1) return;
	soundCache[gotSlot].looping = FALSE;
	
	for (int channel = 0; channel < EFFECT_CHANNELS; channel ++) {
		if (FSOUND_GetCurrentSample (channel) == soundCache[gotSlot].sample) {
			FSOUND_StopSound(channel);
		}
	}
}

void freeSound (int a) {
	FSOUND_Sample_Free (soundCache[a].sample);
	soundCache[a].sample = NULL;
	soundCache[a].fileLoaded = -1;
	soundCache[a].looping = false;
}

void huntKillFreeSound (int filenum) {
	int gotSlot = findInSoundCache (filenum);
	if (gotSlot != -1) freeSound (gotSlot);
}

BOOL initSoundStuff (HWND hwnd) {
	if (FSOUND_GetVersion() < FMOD_VERSION)
	{
		warning (WARNING_FMOD_WRONG_VERSION);
		return FALSE;
	}

	FSOUND_SetHWND(hwnd);
	if (!FSOUND_Init(44100, 32, 0)) {
		warning (WARNING_FMOD_FAIL); //, FMOD_ErrorString(FSOUND_GetError()));
		return FALSE;
	}
	
	int a;
	for (a = 0; a < MAX_SAMPLES; a ++) {
		soundCache[a].sample = NULL;
		soundCache[a].fileLoaded = -1;
	}
	for (a = 0; a < EFFECT_CHANNELS; a ++) {
		FSOUND_SetReserved (a, TRUE);
	}
	
	soundOK = TRUE;
}

void killSoundStuff () {
	if (soundOK) {
		int a;
		for (a = 0; a < MAX_MODS;		a ++) stopMOD	(a);
		for (a = 0; a < MAX_SAMPLES;	a ++) freeSound	(a);
		FSOUND_Close();
	}
}

BOOL playMOD (int f, int a, int fromTrack) {
	if (soundOK) {
		stopMOD (a);
	
		setResourceForFatal (f);
		unsigned long length = openFileFromNum (f);
		if (length == 0) return NULL;

		char * memImage;	
		memImage = loadEntireFileToMemory (bigDataFile, length);
		if (! memImage) return fatal (ERROR_MUSIC_MEMORY_LOW);
		
		mod[a] = FMUSIC_LoadSongMemory (memImage, length);
		delete memImage;
		
		if (! mod[a]) {
		
			// Something's gone wrong!
			switch (FSOUND_GetError()) {
				case FMOD_ERR_MEMORY:
				fatal (ERROR_MUSIC_MEMORY_LOW);
				break;
		
				case FMOD_ERR_VERSION:
				case FMOD_ERR_FILE_FORMAT:
				fatal (ERROR_MUSIC_UNKNOWN);
				break;
		
				default:		
				fatal (ERROR_MUSIC_ODDNESS);
				break;
			}

			return FALSE;
		}
		
		FMUSIC_SetMasterVolume (mod[a], defVol);
		FMUSIC_PlaySong (mod[a]);
		FMUSIC_SetOrder	(mod[a], fromTrack);
		setResourceForFatal (-1);
	}
    return TRUE;
}

void setMusicVolume (int a, int v) {
	if (soundOK && mod[a]) FMUSIC_SetMasterVolume (mod[a], v);
}

void setDefaultMusicVolume (int v) {
	defVol = v;
}

void setSoundVolume (int a, int v) {
	if (soundOK) {
		int ch = findInSoundCache (a);
		if (ch != -1) {
			if (FSOUND_GetCurrentSample (soundCache[ch].mostRecentChannel) == soundCache[ch].sample) {
				FSOUND_SetVolume (soundCache[ch].mostRecentChannel, v);
			}
		}
	}
}

BOOL stillPlayingSound (int ch) {
	if (soundOK) {
		if (ch != -1) {
			if (soundCache[ch].fileLoaded == -1) return FALSE;
			if (FSOUND_GetCurrentSample (soundCache[ch].mostRecentChannel) == soundCache[ch].sample) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

void setSoundLoop (int a, int s, int e) {
	if (soundOK) {
		int ch = findInSoundCache (a);
		if (ch != -1) {
			int en = FSOUND_Sample_GetLength (soundCache[ch].sample);
			if (e < 1 || e >= en) e = en - 1;
			if (s < 0 || s >= e) s = 0;
	
			FSOUND_Sample_SetLoopPoints (soundCache[ch].sample, s, e);
		}
	}
}

void setDefaultSoundVolume (int v) {
	defSoundVol = v;
}

int emptySoundSlot = 0;

int findEmptySoundSlot () {
	int t;
	for (t = 0; t < MAX_SAMPLES; t ++) {
		emptySoundSlot ++;
		emptySoundSlot %= MAX_SAMPLES;
		if (soundCache[emptySoundSlot].sample == NULL || FSOUND_GetCurrentSample (soundCache[emptySoundSlot].mostRecentChannel) != soundCache[emptySoundSlot].sample) return emptySoundSlot;
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

int guessSoundFree = 0;
/*
int fakeCacheSoundForVideo (char * memImage, int length) {
	if (! soundOK) return 0;
	int a = findEmptySoundSlot ();
	freeSound (a);

	soundCache[a].sample = FSOUND_Sample_Load(FSOUND_FREE, memImage, FSOUND_LOADMEMORY | FSOUND_2D, length);
	soundCache[a].fileLoaded = -2;

	if (soundCache[a].sample) return a;

	fatal ("Don't understand the audio for this movie");
	return -1;
}
*/

/*
void soundWarning (char * t, int i) {
	FILE * u = fopen ("soundlog.txt", "at");
	fprintf (u, "%s: %i\n", t, i);
	fclose (u);
}
*/

bool forceRemoveSound () {
	for (int a = 0; a < 8; a ++) {
		if (soundCache[a].fileLoaded != -1 && ! stillPlayingSound (a)) {
//			soundWarning ("Deleting silent sound", a);
			freeSound (a);
			return 1;
		}
	}
	
	for (int a = 0; a < 8; a ++) {
		if (soundCache[a].fileLoaded != -1) {
//			soundWarning ("Deleting playing sound", a);
			freeSound (a);
			return 1;
		}
	}
//	soundWarning ("Cache is empty!", 0);
	return 0;
}

int cacheSound (int f) {
	setResourceForFatal (f);

	if (! soundOK) return 0;

	int a = findInSoundCache (f);
	if (a != -1) return a;
	if (f == -2) return -1;
	a = findEmptySoundSlot ();
	freeSound (a);

	unsigned long length = openFileFromNum (f);
	if (! length) return -1;
	
	char * memImage;

	bool tryAgain = true;
	
	while (tryAgain) {
		memImage = loadEntireFileToMemory (bigDataFile, length);
		tryAgain = memImage == NULL;
		if (tryAgain) {
			if (! forceRemoveSound ()) {
				fatal (ERROR_SOUND_MEMORY_LOW);
				return -1;
			}
		}
	}
	
	for (;;) {
//		soundWarning ("  Trying to load sound into slot", a);
		soundCache[a].sample = FSOUND_Sample_Load(FSOUND_FREE, memImage, FSOUND_LOADMEMORY | FSOUND_2D, (int) length);

		if (soundCache[a].sample) {
			soundCache[a].fileLoaded = f;
			delete memImage;
			setResourceForFatal (-1);
			return a;
		}

		// Something's gone wrong!
		switch (FSOUND_GetError()) {
			case FMOD_ERR_MEMORY:
			if (forceRemoveSound ()) break; // Try again...
			fatal (ERROR_SOUND_MEMORY_LOW);
			return -1;
	
			case FMOD_ERR_VERSION:
			case FMOD_ERR_FILE_FORMAT:
			fatal (ERROR_SOUND_UNKNOWN);
			return -1;
	
			default:		
			fatal (ERROR_SOUND_ODDNESS);
			return -1;
		}	
	}
}

BOOL startSound (int f, BOOL loopy) {
	if (soundOK) {
		int a = cacheSound (f);
		if (a == -1) return FALSE;
		
		FSOUND_Sample_SetMode (soundCache[a].sample, loopy ? FSOUND_LOOP_NORMAL : FSOUND_LOOP_OFF);
	
		int aa = 0, bb = guessSoundFree;
		do {
			if (! FSOUND_IsPlaying (bb)) break;
			bb ++;
			bb &= (EFFECT_CHANNELS - 1);
		} while (aa ++ < EFFECT_CHANNELS);
	
		guessSoundFree = (bb + 1) & (EFFECT_CHANNELS - 1);
	
		soundCache[a].looping = loopy;
		soundCache[a].vol = defSoundVol;
		soundCache[a].mostRecentChannel = FSOUND_PlaySound(bb, soundCache[a].sample);
		FSOUND_SetVolume (soundCache[a].mostRecentChannel, defSoundVol);
	}
	return TRUE;
}

/*
void debugSounds () {
	FILE * fp = fopen ("newdebug.txt", "at");
	if (fp) {
		for (int aa = 0; aa < 32; aa ++) {
			if (aa == EFFECT_CHANNELS) fprintf (fp, "|");
			fprintf (fp, FSOUND_IsPlaying (aa) ? "#" : ".");
		}
		fprintf (fp, "\n");
		fclose (fp);
	}
}
// */

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

BOOL getSoundCacheStack (stackHandler * sH) {
	variable newFileHandle;
	newFileHandle.varType = SVT_NULL;
		
	for (int a = 0; a < MAX_SAMPLES; a ++) {
		if (soundCache[a].fileLoaded != -1) {
			setVariable (newFileHandle, SVT_FILE, soundCache[a].fileLoaded);
			if (! addVarToStackQuick (newFileHandle, sH -> first)) return FALSE;
			if (sH -> last == NULL) sH -> last = sH -> first;
		}
	}			
	return TRUE;
}

