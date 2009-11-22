#include <string.h>

#include "allfiles.h"
#include "Graphics.h"
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
extern GLuint backdropTextureName;
extern parallaxLayer * parallaxStuff;
extern int lightMapNumber, zBufferNumber;
extern eventHandlers * currentEvents;
extern personaAnimation * mouseCursorAnim;
extern int mouseCursorFrameNum;
extern int cameraX, cameraY, sceneWidth, sceneHeight;
extern zBufferData zBuffer;
extern bool backdropExists;

frozenStuffStruct * frozenStuff = NULL;

bool freeze () {
	frozenStuffStruct * newFreezer = new frozenStuffStruct;
	if (! checkNew (newFreezer)) return false;

	newFreezer -> backdropTextureName = backdropTextureName;

	int picWidth = sceneWidth;
	int picHeight = sceneHeight;
	if (! NPOT_textures) {
		picWidth = getNextPOT(picWidth);
		picHeight = getNextPOT(picHeight);
	}
	newFreezer -> backdropTexture = new GLubyte [picHeight*picWidth*4];
	glBindTexture (GL_TEXTURE_2D, backdropTextureName);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, newFreezer -> backdropTexture);
	backdropTextureName = 0;

	newFreezer -> sceneWidth = sceneWidth;
	newFreezer -> sceneHeight = sceneHeight;
	newFreezer -> cameraX = cameraX;
	newFreezer -> cameraY = cameraY;

	newFreezer -> lightMapTexture = lightMap.data;
	newFreezer -> lightMapTextureName = lightMap.name;
	newFreezer -> lightMapNumber = lightMapNumber;
	lightMap.data = NULL;
	lightMap.name = 0;

	newFreezer -> parallaxStuff = parallaxStuff;
	parallaxStuff = NULL;

	newFreezer -> zBufferImage = zBuffer.tex;
	newFreezer -> zBufferNumber = zBuffer.originalNum;
	newFreezer -> zPanels = zBuffer.numPanels;
	zBuffer.tex = NULL;

	// resizeBackdrop kills parallax stuff, light map, z-buffer...
	if (! resizeBackdrop (winWidth, winHeight)) return fatal ("Can't create new temporary backdrop buffer");

	if (! NPOT_textures) {
		picWidth = getNextPOT(sceneWidth);
		picHeight = getNextPOT(sceneHeight);
		backdropTexW = (double) sceneWidth / picWidth;
		backdropTexH = (double) sceneHeight / picHeight;
	}
	copyToBackDrop (newFreezer -> backdropTextureName,
					newFreezer -> sceneWidth,
					newFreezer -> sceneHeight,
					newFreezer -> cameraX,
					newFreezer -> cameraY,
					newFreezer -> parallaxStuff);

	// Put the lightmap and z-buffer back JUST for drawing the people...
	lightMap.data = newFreezer -> lightMapTexture;
	lightMap.name = newFreezer -> lightMapTextureName;
	zBuffer.tex = newFreezer -> zBufferImage;
	zBuffer.numPanels = newFreezer -> zPanels;
	fixPeople (newFreezer -> cameraX, newFreezer -> cameraY);
	lightMap.data = NULL;
	lightMap.name = 0;
	zBuffer.tex = NULL;
	zBuffer.numPanels = 0;

	// Free texture memory used by old stuff
	parallaxStuff = newFreezer -> parallaxStuff;
	while (parallaxStuff) {
		glDeleteTextures (1, &parallaxStuff -> textureName);
		parallaxStuff = parallaxStuff -> next;
	}
	if (newFreezer -> zBufferImage) {
		glDeleteTextures (1, &zBuffer.texName);
	}
	if (newFreezer -> lightMapTextureName) {
		glDeleteTextures (1, &newFreezer -> lightMapTextureName);
	}
	if (newFreezer -> backdropTextureName) {
		glDeleteTextures (1, &newFreezer -> backdropTextureName);
	}

	newFreezer -> allPeople = allPeople;
	allPeople = NULL;

	statusStuff * newStatusStuff = new statusStuff;
	if (! checkNew (newStatusStuff)) return false;
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
	if (! checkNew (currentEvents)) return false;
	memset (currentEvents, 0, sizeof (eventHandlers));

	newFreezer -> next = frozenStuff;
	frozenStuff = newFreezer;
	return true;
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

extern GLubyte * backdropTexture;

void unfreeze (bool killImage) {
	frozenStuffStruct * killMe = frozenStuff;

	if (! frozenStuff) return;

	sceneWidth = frozenStuff -> sceneWidth;
	sceneHeight = frozenStuff -> sceneHeight;

	cameraX = frozenStuff -> cameraX;
	cameraY = frozenStuff -> cameraY;

	killAllPeople ();
	allPeople = frozenStuff -> allPeople;

	killAllRegions ();
	allScreenRegions = frozenStuff -> allScreenRegions;

	killLightMap ();
	lightMap.data = frozenStuff -> lightMapTexture;
	lightMap.name = frozenStuff -> lightMapTextureName;
	lightMapNumber = frozenStuff -> lightMapNumber;
	if (lightMapNumber) {
		lightMap.name = 0;
		loadLightMap(lightMapNumber);
	}

	killZBuffer ();
	zBuffer.tex = frozenStuff -> zBufferImage;
	zBuffer.originalNum = frozenStuff -> zBufferNumber;
	zBuffer.numPanels = frozenStuff -> zPanels;
	if (zBuffer.numPanels) {
		zBuffer.texName = 0;
		setZBuffer (zBuffer.originalNum);
	}

	killParallax ();
	parallaxStuff = frozenStuff -> parallaxStuff;
	reloadParallaxTextures ();

	if (killImage) killBackDrop ();
	backdropTextureName = frozenStuff -> backdropTextureName;
	backdropExists = true;
	if (backdropTextureName) {
		backdropTextureName = 0;
		glGenTextures (1, &backdropTextureName);
		glBindTexture (GL_TEXTURE_2D, backdropTextureName);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		if (maxAntiAliasSettings.useMe) {
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //GL_NEAREST
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		} else {
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		int picWidth = sceneWidth;
		int picHeight = sceneHeight;
		if (! NPOT_textures) {
			picWidth = getNextPOT(picWidth);
			picHeight = getNextPOT(picHeight);
		}
		// Restore the backdrop
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, picWidth, picHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, frozenStuff -> backdropTexture);

	}

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
	killMe = NULL;
}
