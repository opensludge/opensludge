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

extern unsigned long startOfDataIndex, startOfTextIndex,
			  startOfSubIndex, startOfObjectIndex;


void createFontPalette (spritePalette & sP) {
	sP.total = 0;
	sP.pal = NULL;
	sP.originalRed = 0;
	sP.originalBlue = 0;
	sP.originalGreen = 0;
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

	xOff += fontSpace >> 1;
	for (a = 0; theText[a]; a ++) {
		mySprite = & theFont.sprites[fontTable[(unsigned char) theText[a]]];
		fontSprite (xOff, y, * mySprite, thePal);
		xOff += mySprite -> width + fontSpace;
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

	for (int i = 0; i < theFont.myPalette.numTextures; i++) {
		spal.tex_names[i] = theFont.myPalette.tex_names[i];
		spal.burnTex_names[i] = theFont.myPalette.burnTex_names[i];
		spal.tex_w[i] = theFont.myPalette.tex_w[i];
		spal.tex_h[i] = theFont.myPalette.tex_h[i];
	}
	spal.numTextures = theFont.myPalette.numTextures;
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
