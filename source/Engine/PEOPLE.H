#include "sprites_AA.h"

struct animFrame {
	int frameNum, howMany;
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
	BOOL walking, spinning;
	struct loadedFunction * continueAfterWalking;
	personaAnimation * myAnim;
	personaAnimation * lastUsedAnim;
	persona * myPersona;
	int frameNum, frameTick, angle, wantAngle;
	BOOL show;
	int direction, directionWhenDoneWalking;
	struct objectType * thisType;
	int drawMode, extra, spinSpeed;
	aaSettingsStruct aaSettings;
};

// Initialisation and creation

BOOL initPeople ();
BOOL addPerson (int x, int y, int objNum, persona * p);

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
void setShown (BOOL h, int ob);
void setDrawMode (int h, int ob);

// Moving 'em

void movePerson (int x, int y, int objNum);
BOOL makeWalkingPerson (int x, int y, int objNum, struct loadedFunction * func, int di);
BOOL forceWalkingPerson (int x, int y, int objNum, struct loadedFunction * func, int di);
void jumpPerson (int x, int y, int objNum);
void walkAllPeople ();
BOOL turnPersonToFace (int thisNum, int direc);
BOOL stopPerson (int o);
BOOL floatCharacter (int f, int objNum);
BOOL setCharacterWalkSpeed (int f, int objNum);

// Animating 'em

void animatePerson (int obj, personaAnimation *);
void animatePerson (int obj, persona * per);
personaAnimation * createPersonaAnim (int num, struct variableStack * & stacky);
inline void setBankFile (personaAnimation * newP, loadedSpriteBank * sB) { newP -> theSprites = sB; }
BOOL setPersonExtra (int f, int newSetting);
int timeForAnim (personaAnimation * fram);
personaAnimation * copyAnim (personaAnimation * orig);
personaAnimation * makeNullAnim ();
void deleteAnim (personaAnimation * orig);

// Loading and saving

BOOL saveAnim (personaAnimation * p, FILE * fp);
BOOL loadAnim (personaAnimation * p, FILE * fp);
BOOL savePeople (FILE * fp);
BOOL loadPeople (FILE * fp, int ssgVersion);
BOOL saveCostume (persona * cossy, FILE * fp);
BOOL loadCostume (persona * cossy, FILE * fp);
