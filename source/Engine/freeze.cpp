#include <string.h>
#include <stdlib.h>

#include "allfiles.h"
#include "graphics.h"
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
extern float cameraZoom;
extern zBufferData zBuffer;
extern bool backdropExists;

frozenStuffStruct * frozenStuff = NULL;

extern int sceneWidth, sceneHeight;

void shufflePeople ();

void freezePeople (int oldX, int oldY) {
		
	shufflePeople ();
	
	onScreenPerson * thisPerson = allPeople;
	personaAnimation * myAnim;
	
	while (thisPerson) {
		if (thisPerson -> show) {
			myAnim = thisPerson -> myAnim;
			if (myAnim != thisPerson -> lastUsedAnim) {
				thisPerson -> lastUsedAnim = myAnim;
				thisPerson -> frameNum = 0;
				thisPerson -> frameTick = myAnim -> frames[0].howMany;
			}
			int fNumSign = myAnim -> frames[thisPerson -> frameNum].frameNum;
			int m = fNumSign < 0;
			int fNum = abs (fNumSign);
			
			if (fNum >= myAnim -> theSprites -> bank.total) {
				fNum = 0;
				m = 2 - m;
			}
			if (m != 2) {
				int meX, meY;
				if (thisPerson -> extra & EXTRA_FIXTOSCREEN) {
					meX = thisPerson -> x;
					meY = thisPerson -> y;
				} else {
					meX = thisPerson -> x - oldX;
					meY = thisPerson -> y - oldY;
				}
				fixScaleSprite (meX, meY, myAnim -> theSprites -> bank.sprites[fNum], myAnim -> theSprites -> bank.myPalette, thisPerson, oldX, oldY, m);
			}
		}
		thisPerson = thisPerson -> next;
	}
}

inline int sortOutPCamera (int cX, int fX, int sceneMax, int boxMax) {
	return (fX == 65535) ?
	(sceneMax ? ((cX * boxMax) / sceneMax) : 0)
	:
	((cX * fX) / 100);
}

/*
 / freezeBackdrop
 / This function is used to copy the old backdrop when doing a freeze
 */
void freezeBackDrop (frozenStuffStruct * newFreezer) {
		
	GLuint fromHere = newFreezer -> backdropTextureName;
	int orW = newFreezer -> sceneWidth;
	int orH = newFreezer -> sceneHeight;
	int orX = newFreezer -> cameraX;
	int orY = newFreezer -> cameraY;
	if (orX < 0) orX = 0;
	else if (orX > newFreezer ->sceneWidth - winWidth) orX = newFreezer ->sceneWidth - winWidth;
	if (orY < 0) orY = 0;
	else if (orY > newFreezer ->sceneHeight - winHeight) orY = newFreezer ->sceneHeight - winHeight;
	parallaxLayer * parallaxS = newFreezer -> parallaxStuff;
	
	setPixelCoords (true);
	
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glEnable (GL_TEXTURE_2D);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_BLEND);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen
	
	int xoffset = 0;
	
	while (xoffset < winWidth) {
		int w = (winWidth-xoffset < viewportWidth) ? winWidth-xoffset : viewportWidth;
		
		int yoffset = 0;
		while (yoffset < orH) {
			int h = (winHeight-yoffset < viewportHeight) ? winHeight-yoffset : viewportHeight;
			
			if (parallaxS) {
				parallaxLayer * ps = parallaxS;
				
				while (ps->next) ps = ps->next;
				
				while (ps) {
					ps -> cameraX = sortOutPCamera (orX, ps -> fractionX, orW - w, ps -> width - winWidth);
					ps -> cameraY = sortOutPCamera (orY, ps -> fractionY, orH - h, ps -> height - winHeight);
					
					glBindTexture (GL_TEXTURE_2D, ps->textureName);
					glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glBegin(GL_QUADS);
					
					float texw = (ps->wrapS) ? (float) orW / ps->width: 1.0;
					float wt = (ps->wrapS) ? orW : ps->width;
					float texh = (ps->wrapT) ? (float) orH / ps->height: 1.0;
					float ht = (ps->wrapT) ? orH : ps->height;
					
					glTexCoord2f(0.0, 0.0); glVertex3f(-ps -> cameraX-xoffset, -ps -> cameraY-yoffset, 0.1);
					glTexCoord2f(texw, 0.0); glVertex3f(wt -ps -> cameraX-xoffset, -ps -> cameraY-yoffset, 0.1);
					glTexCoord2f(texw, texh); glVertex3f(wt -ps -> cameraX-xoffset, ht -ps -> cameraY-yoffset, 0.1);
					glTexCoord2f(0.0, texh); glVertex3f(-ps -> cameraX-xoffset, ht -ps -> cameraY-yoffset, 0.1);
					
					glEnd();
					
					ps = ps -> prev;
				}
			}
			
			
			// Render the backdrop
			glBindTexture (GL_TEXTURE_2D, fromHere);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glColor4f(1.0, 1.0, 1.0, 1.0);
			
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex3f(-orX-xoffset, -orY-yoffset, 0);
			glTexCoord2f(backdropTexW, 0.0); glVertex3f(orW-orX-xoffset, -orY-yoffset, 0);
			glTexCoord2f(backdropTexW, backdropTexH); glVertex3f(orW-orX-xoffset, orH-orY-yoffset, 0);
			glTexCoord2f(0.0, backdropTexH); glVertex3f(-orX-xoffset, orH-orY-yoffset, 0);
			glEnd();
			
			// Copy Our ViewPort To The Texture
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0+xoffset, 0+yoffset, viewportOffsetX, viewportOffsetY, w, h);
			
			yoffset += viewportHeight;
		}
		xoffset += viewportWidth;
	}
	
	
	setPixelCoords(false);
	backdropExists = true;
}


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
	newFreezer -> cameraZoom = cameraZoom;

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
	freezeBackDrop (newFreezer);

	// Put the lightmap and z-buffer back JUST for drawing the people...
	lightMap.data = newFreezer -> lightMapTexture;
	lightMap.name = newFreezer -> lightMapTextureName;
	zBuffer.tex = newFreezer -> zBufferImage;
	zBuffer.numPanels = newFreezer -> zPanels;
	
	int oldX = newFreezer -> cameraX;
	int oldY = newFreezer -> cameraY;
	if (oldX < 0) oldX = 0;
	else if (oldX > newFreezer -> sceneWidth - winWidth) oldX = newFreezer -> sceneWidth - winWidth;
	if (oldY < 0) oldY = 0;
	else if (oldY > newFreezer -> sceneHeight - winHeight) oldY = newFreezer -> sceneHeight - winHeight;
	
	freezePeople (oldX, oldY);
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
	input.mouseX = input.mouseX * cameraZoom;
	input.mouseY = input.mouseY * cameraZoom;	
	cameraZoom = frozenStuff -> cameraZoom;
	input.mouseX = input.mouseX / cameraZoom;
	input.mouseY = input.mouseY / cameraZoom;
	setPixelCoords(false);

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
	if (backdropTexture) delete backdropTexture;
	backdropTexture = frozenStuff -> backdropTexture;
	backdropExists = true;
	if (backdropTextureName) {
		backdropTextureName = 0;
		glGenTextures (1, &backdropTextureName);
		glBindTexture (GL_TEXTURE_2D, backdropTextureName);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

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
