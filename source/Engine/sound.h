#include "variable.h"
#ifdef _WIN32
#include <windows.h>
#else
#define HWND void *
#endif


// Sound list stuff
struct soundList {
	int sound;
	struct soundList * next;
	struct soundList * prev;
	int cacheIndex;
	int vol;
};
soundList *deleteSoundFromList (soundList *s);
void playSoundList(soundList *s);



// GENERAL...
bool initSoundStuff (HWND);
void killSoundStuff ();

// MUSIC...
bool playMOD (int, int, int);
void stopMOD (int);
void setMusicVolume (int a, int v);
void setDefaultMusicVolume (int v);

// SAMPLES...
int cacheSound (int f);
//int fakeCacheSoundForVideo (char * memImage, int length);
bool startSound (int, bool = false);
void huntKillSound (int a);
void huntKillFreeSound (int filenum);
void setSoundVolume (int a, int v);
void setDefaultSoundVolume (int v);
void setSoundLoop (int a, int s, int e);
bool stillPlayingSound (int ch);
bool getSoundCacheStack (stackHandler * sH);
int findInSoundCache (int a);

void debugSounds ();
void loadSounds (FILE * fp);
void saveSounds (FILE * fp);
