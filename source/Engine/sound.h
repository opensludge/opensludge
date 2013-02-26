#include "variable.h"

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
bool initSoundStuff ();
void killSoundStuff ();

// MOD MUSIC...
bool playMOD (int filenum, int channel, int position);
void stopMOD (int channel);
void setMusicVolume (int channel, int volume);
void setDefaultMusicVolume (int volume);

// SAMPLES...
bool startSound (int filenum, bool loopy = false);
void huntKillSound (int filenum);
void huntKillFreeSound (int filenum);
void setSoundVolume (int filenum, int volume);
void setDefaultSoundVolume (int volume);
void setSoundLoop (int filenum, int start, int end);
bool stillPlayingSound (int filenum);
bool getSoundCacheStack (stackHandler * sH);
int findInSoundCache (int filenum);

void loadSounds (FILE * fp);
void saveSounds (FILE * fp);

unsigned int getSoundSource(int index);
