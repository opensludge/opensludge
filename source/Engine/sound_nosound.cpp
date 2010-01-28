#include <stdint.h>

#include "allfiles.h"
#include "newfatal.h"
#include "sound.h"
#include "MOREIO.H"
#include "fileset.h"

bool soundOK = false;

int defVol = 128;
int defSoundVol = 255;

#if 0
char * loadEntireFileToMemory (FILE * inputFile, uint32_t size) {
	char * allData = new char[size];
	if (! allData) return NULL;
	fread (allData, size, 1, inputFile);
	finishAccess ();

	return allData;
}
#endif

int findInSoundCache (int a) {
//#pragma unused(a)
	return -1;
}

void stopMOD (int i) {
//#pragma unused(i)
}

void huntKillSound (int filenum) {
//#pragma unused(filenum)
}

void huntKillFreeSound (int filenum) {
//#pragma unused(filenum)
}

bool initSoundStuff (HWND hwnd) {
//	#pragma unused(hwnd)
	return false;
}

void killSoundStuff () {
}

bool playMOD (int f, int a, int fromTrack) {
//#pragma unused (f,a,fromTrack)
    return true;
}

void setMusicVolume (int a, int v) {
//#pragma unused (a,v)
}

void setDefaultMusicVolume (int v) {
	defVol = v;
}

void setSoundVolume (int a, int v) {
//#pragma unused (a,v)
}

bool stillPlayingSound (int ch) {
//#pragma unused (ch)
	return false;
}

void setSoundLoop (int a, int s, int e) {
//#pragma unused (a,s,e)
}

void setDefaultSoundVolume (int v) {
	defSoundVol = v;
}

bool forceRemoveSound () {
	return 0;
}

int cacheSound (int f) {
//#pragma unused (f)
	return 0;
}

bool startSound (int f, bool loopy) {	
//#pragma unused (f,loopy)
	return true;
}

void saveSounds (FILE * fp) {
	fputc (0, fp);
	put2bytes (defSoundVol, fp);
	put2bytes (defVol, fp);
}

void loadSounds (FILE * fp) {
	while (fgetc (fp)) {
		get2bytes (fp);
		get2bytes (fp);
	}
	
	defSoundVol = get2bytes (fp);
	defVol = get2bytes (fp);
}

bool getSoundCacheStack (stackHandler * sH) {
//#pragma unused (sH)
	return true;
}
