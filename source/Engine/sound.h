#include "variable.h"

// GENERAL...
BOOL initSoundStuff (HWND);
void killSoundStuff ();

// MUSIC...
BOOL playMOD (int, int, int);
void stopMOD (int);
void setMusicVolume (int a, int v);
void setDefaultMusicVolume (int v);

// SAMPLES...
int cacheSound (int f);
//int fakeCacheSoundForVideo (char * memImage, int length);
BOOL startSound (int, BOOL = FALSE);
void huntKillSound (int a);
void huntKillFreeSound (int filenum);
void setSoundVolume (int a, int v);
void setDefaultSoundVolume (int v);
void setSoundLoop (int a, int s, int e);
BOOL stillPlayingSound (int ch);
BOOL getSoundCacheStack (stackHandler * sH);
int findInSoundCache (int a);

void debugSounds ();
void loadSounds (FILE * fp);
void saveSounds (FILE * fp);
