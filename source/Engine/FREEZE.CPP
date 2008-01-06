#include "allfiles.h"
#include "newfatal.h"
#include "sprites.h"
#include "sprbanks.h"
#include "people.h"
#include "sludger.h"
#include "objtypes.h"
#include "region.h"
#include "backdrop.h"
#include "talk.h"
#include "fonttext.h"
#include "statusba.h"
#include "freeze.h"
#include "zbuffer.h"

extern onScreenPerson * allPeople;
extern screenRegion * allScreenRegions;
extern screenRegion * overRegion;
extern speechStruct * speech;
extern inputType input;
extern unsigned short int * * backDropImage;
extern unsigned short int * * lightMapImage;
extern unsigned short int * * zBufferImage;
extern parallaxLayer * parallaxStuff;
extern int lightMapNumber, zBufferNumber;
extern eventHandlers * currentEvents;
extern personaAnimation * mouseCursorAnim;
extern int mouseCursorFrameNum;
extern int cameraX, cameraY, sceneWidth, sceneHeight;
extern zBufferData zBuffer;

frozenStuffStruct * frozenStuff = NULL;

BOOL freeze () {
	frozenStuffStruct * newFreezer = new frozenStuffStruct;
	if (! checkNew (newFreezer)) return FALSE;

	newFreezer -> backDropImage = backDropImage;
	newFreezer -> sceneWidth = sceneWidth;
	newFreezer -> sceneHeight = sceneHeight;
	newFreezer -> cameraX = cameraX;
	newFreezer -> cameraY = cameraY;
	backDropImage = NULL;

	newFreezer -> lightMapImage = lightMapImage;
	newFreezer -> lightMapNumber = lightMapNumber;
	lightMapImage = NULL;

	newFreezer -> parallaxStuff = parallaxStuff;
	parallaxStuff = NULL;

	newFreezer -> zBufferImage = zBuffer.map;
	newFreezer -> zBufferNumber = zBuffer.originalNum;
	zBuffer.map = NULL;

	// resizeBackdrop kills parallax stuff, light map, z-buffer...
	if (! resizeBackdrop (winWidth, winHeight)) return fatal ("Can't create new temporary backdrop buffer");

	copyToBackDrop (newFreezer -> backDropImage,
					newFreezer -> sceneWidth,
					newFreezer -> sceneHeight,
					newFreezer -> cameraX,
					newFreezer -> cameraY,
					newFreezer -> parallaxStuff);

	// Put the lightmap and z-buffer back JUST for drawing the people...
	lightMapImage = newFreezer -> lightMapImage;
	zBuffer.map = newFreezer -> zBufferImage;
	fixPeople (newFreezer -> cameraX, newFreezer -> cameraY);
	lightMapImage = NULL;
	zBuffer.map = NULL;

	newFreezer -> allPeople = allPeople;
	allPeople = NULL;
	
	statusStuff * newStatusStuff = new statusStuff;
	if (! checkNew (newStatusStuff)) return FALSE;
	newFreezer -> frozenStatus = copyStatusBarStuff (newStatusStuff);
	
	newFreezer -> allScreenRegions = allScreenRegions;
	allScreenRegions = NULL;
	overRegion = NULL;

	newFreezer -> mouseCursorAnim = mouseCursorAnim;
	newFreezer -> mouseCursorFrameNum = mouseCursorFrameNum;
	mouseCursorAnim = makeNullAnim ();
	mouseCursorFrameNum = 0;

	newFreezer -> speech = speech;
	initSpeech ();
	
	newFreezer -> currentEvents = currentEvents;
	currentEvents = new eventHandlers;
	if (! checkNew (currentEvents)) return FALSE;
	memset (currentEvents, 0, sizeof (eventHandlers));

	newFreezer -> next = frozenStuff;
	frozenStuff = newFreezer;
	return TRUE;
}

int howFrozen () {
	int a = 0;
	frozenStuffStruct * f = frozenStuff;
	while (f) {
		a ++;
		f = f -> next;
	}
	return a;
}

void unfreeze (BOOL killImage) {
	frozenStuffStruct * killMe = frozenStuff;

	if (! frozenStuff) return;

	killAllPeople ();
	allPeople = frozenStuff -> allPeople;

	killAllRegions ();
	allScreenRegions = frozenStuff -> allScreenRegions;

	if (killImage) killBackDrop ();
	backDropImage = frozenStuff -> backDropImage;

	killLightMap ();
	lightMapImage = frozenStuff -> lightMapImage;
	lightMapNumber = frozenStuff -> lightMapNumber;
	
	noZBuffer ();
	zBuffer.map = frozenStuff -> zBufferImage;
	zBuffer.originalNum = frozenStuff -> zBufferNumber;

	killParallax ();
	parallaxStuff = frozenStuff -> parallaxStuff;

	sceneWidth = frozenStuff -> sceneWidth;
	sceneHeight = frozenStuff -> sceneHeight;
	cameraX = frozenStuff -> cameraX;
	cameraY = frozenStuff -> cameraY;

	deleteAnim (mouseCursorAnim);
	mouseCursorAnim = frozenStuff -> mouseCursorAnim;
	mouseCursorFrameNum = frozenStuff -> mouseCursorFrameNum;

	restoreBarStuff (frozenStuff -> frozenStatus);

	delete currentEvents;
	currentEvents = frozenStuff -> currentEvents;

	killAllSpeech ();
	delete speech;
	speech = frozenStuff -> speech;

	frozenStuff = frozenStuff -> next;

	overRegion = NULL;
	delete killMe;
}