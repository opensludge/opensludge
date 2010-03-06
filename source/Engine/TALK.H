#include "sprites.h"

struct speechLine {
	char * textLine;
	speechLine * next;
	int x;
};

struct speechStruct {
	onScreenPerson * currentTalker;
	speechLine * allSpeech;
	int speechY, lastFile, lookWhosTalking;
	spritePalette talkCol;
};

int wrapSpeech (char * theText, int objT, int sampleFile, bool);
void viewSpeech ();
void killAllSpeech ();
int isThereAnySpeechGoingOn ();
void initSpeech ();
void saveSpeech (speechStruct * sS, FILE * fp);
bool loadSpeech (speechStruct * sS, FILE * fp);

