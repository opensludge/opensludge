#include <stdint.h>

#include "debug.h"
#include "allfiles.h"

#include "sprites.h"
#include "colours.h"
#include "fonttext.h"
#include "newfatal.h"
#include "moreio.h"

spriteBank theFont;
bool fontLoaded = false;
int fontHeight = 0, numFontColours, loadedFontNum;
byte fontTable[256];
short fontSpace = -1;

extern uint32_t startOfDataIndex, startOfTextIndex,
			  startOfSubIndex, startOfObjectIndex;

extern float cameraZoom;

bool isInFont (char * theText) {
	return theText[0] && theText[1] == 0 && fontTable[(unsigned char) theText[0]] != 0;
}

int stringWidth (char * theText) {
	int a;
	int xOff = 0;

	if (! fontLoaded) return 0;

	for (a = 0; theText[a]; a ++) {
		xOff += theFont.sprites[fontTable[(unsigned char) theText[a]]].width + fontSpace;
	}

	return xOff;
}

void pasteString (char * theText, int xOff, int y, spritePalette & thePal) {
	sprite * mySprite;
	int a;

	if (! fontLoaded) return;

	xOff += (int)((float)(fontSpace >> 1) / cameraZoom);
	for (a = 0; theText[a]; a ++) {
		mySprite = & theFont.sprites[fontTable[(unsigned char) theText[a]]];
		fontSprite (xOff, y, * mySprite, thePal);
		xOff += (int)((double)(mySprite -> width + fontSpace) / cameraZoom);
	}
}

void pasteStringToBackdrop (char * theText, int xOff, int y, spritePalette & thePal) {
	sprite * mySprite;
	int a;

	if (! fontLoaded) return;

	xOff += fontSpace >> 1;
	for (a = 0; theText[a]; a ++) {
		mySprite = & theFont.sprites[fontTable[(unsigned char) theText[a]]];
		pasteSpriteToBackDrop (xOff, y, * mySprite, thePal);
		xOff += mySprite -> width + fontSpace;
	}
}

void burnStringToBackdrop (char * theText, int xOff, int y, spritePalette & thePal) {
	sprite * mySprite;
	int a;

	if (! fontLoaded) return;

	xOff += fontSpace >> 1;
	for (a = 0; theText[a]; a ++) {
		mySprite = & theFont.sprites[fontTable[ (unsigned char) theText[a]]];
		burnSpriteToBackDrop (xOff, y, * mySprite, thePal);
		xOff += mySprite -> width + fontSpace;
	}
}

void fixFont (spritePalette & spal) {

	delete [] spal.tex_names;
	delete [] spal.burnTex_names;
	delete [] spal.tex_h;
	delete [] spal.tex_w;
	
	spal.numTextures = theFont.myPalette.numTextures;

	spal.tex_names = new GLuint [spal.numTextures];
	spal.burnTex_names = new GLuint [spal.numTextures];
	spal.tex_w = new int [spal.numTextures];
	spal.tex_h = new int [spal.numTextures];
	
	for (int i = 0; i < theFont.myPalette.numTextures; i++) {
		spal.tex_names[i] = theFont.myPalette.tex_names[i];
		spal.burnTex_names[i] = theFont.myPalette.burnTex_names[i];
		spal.tex_w[i] = theFont.myPalette.tex_w[i];
		spal.tex_h[i] = theFont.myPalette.tex_h[i];
	}
}

void setFontColour (spritePalette & sP, byte r, byte g, byte b) {
	sP.originalRed = r;
	sP.originalGreen = g;
	sP.originalBlue = b;
}

bool loadFont (int filenum, const char * charOrder, int h) {
	int a;

	loadedFontNum = filenum;

	for (a = 0; a < 256; a ++) {
		fontTable[a] = 0;
	}
	for (a = 0; charOrder[a]; a ++) {
		fontTable[(unsigned char) charOrder[a]] = (byte) a;
	}

	if (! loadSpriteBank (filenum, theFont, true)) {
		fatal ("Can't load font");
		return false;
	}

	numFontColours = theFont.myPalette.total;
	fontHeight = h;
	fontLoaded = true;
	return true;
}
