#include "sprites_aa.h"

struct animFrame {
	int frameNum, howMany;
	int noise;
};

#define EXTRA_FRONT			1
#define EXTRA_FIXEDSIZE		2
#define EXTRA_NOSCALE		2	// Alternative name
#define EXTRA_NOZB			4
#define EXTRA_FIXTOSCREEN	8
#define EXTRA_NOLITE		16
#define EXTRA_NOREMOVE		32
#define EXTRA_RECTANGULAR	64

struct personaAnimation {
	struct loadedSpriteBank * theSprites;
	animFrame * frames;
	int numFrames;
};

struct persona {
	personaAnimation * * animation;
	int numDirections;
};

struct aaSettingsStruct;

struct onScreenPerson {
	float x, y;
	int height, floaty, walkSpeed;
	float scale;
	onScreenPerson * next;
	int walkToX, walkToY, thisStepX, thisStepY, inPoly, walkToPoly;
	bool walking, spinning;
	struct loadedFunction * continueAfterWalking;
	personaAnimation * myAnim;
	personaAnimation * lastUsedAnim;
	persona * myPersona;
	int frameNum, frameTick, angle, wantAngle, angleOffset;
	bool show;
	int direction, directionWhenDoneWalking;
	struct objectType * thisType;
	int extra, spinSpeed;
	aaSettingsStruct aaSettings;
	unsigned char r,g,b,colourmix,transparency;
};

// Initialisation and creation

bool initPeople ();
bool addPerson (int x, int y, int objNum, persona * p);

// Draw to screen and to backdrop

void drawPeople ();
void fixPeople (int, int);

// Removalisationisms

void killAllPeople ();
void killMostPeople ();
void removeOneCharacter (int i);

// Things which affect or use all characters

onScreenPerson * findPerson (int v);
void setScale (short int h, short int d);

// Things which affect one character

void makeTalker (onScreenPerson & me);
void makeSilent (onScreenPerson & me);
void setShown (bool h, int ob);
void setDrawMode (int h, int ob);
void setPersonTransparency (int ob, unsigned char x);
void setPersonColourise (int ob, unsigned char r, unsigned char g, unsigned char b, unsigned char colourmix);

// Moving 'em

void movePerson (int x, int y, int objNum);
bool makeWalkingPerson (int x, int y, int objNum, struct loadedFunction * func, int di);
bool forceWalkingPerson (int x, int y, int objNum, struct loadedFunction * func, int di);
void jumpPerson (int x, int y, int objNum);
void walkAllPeople ();
bool turnPersonToFace (int thisNum, int direc);
bool stopPerson (int o);
bool floatCharacter (int f, int objNum);
bool setCharacterWalkSpeed (int f, int objNum);

// Animating 'em

void animatePerson (int obj, personaAnimation *);
void animatePerson (int obj, persona * per);
personaAnimation * createPersonaAnim (int num, struct variableStack * & stacky);
inline void setBankFile (personaAnimation * newP, loadedSpriteBank * sB) { newP -> theSprites = sB; }
bool setPersonExtra (int f, int newSetting);
int timeForAnim (personaAnimation * fram);
personaAnimation * copyAnim (personaAnimation * orig);
personaAnimation * makeNullAnim ();
void deleteAnim (personaAnimation * orig);

// Loading and saving

bool saveAnim (personaAnimation * p, FILE * fp);
bool loadAnim (personaAnimation * p, FILE * fp);
bool savePeople (FILE * fp);
bool loadPeople (FILE * fp);
bool saveCostume (persona * cossy, FILE * fp);
bool loadCostume (persona * cossy, FILE * fp);
