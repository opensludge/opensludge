#include "allfiles.h"

#include "sprites.h"
#include "colours.h"
#include "fonttext.h"
#include "newfatal.h"
#include "moreio.h"

spriteBank theFont;
BOOL fontLoaded = FALSE;
int fontHeight = 0, numFontColours, loadedFontNum;
byte fontTable[256];
short fontSpace = -1;

/*
void fontDebug (char * tx) {
	FILE * db = fopen ("font_db.txt", "at");
	fprintf (db, "%s\n", tx);
	fclose (db);
}

void fontDebug (char * tx, int i) {
	FILE * db = fopen ("font_db.txt", "at");
	fprintf (db, "%s %i\n", tx, i);
	fclose (db);
}

void fontDebug (char * tx, const char * tx2) {
	FILE * db = fopen ("font_db.txt", "at");
	fprintf (db, "%s %s\n", tx, tx2);
	fclose (db);
}
*/
void createFontPalette (spritePalette & sP) {
	sP.total = 0;
	sP.pal = NULL;
	sP.originalRed = 0;
	sP.originalBlue = 0;
	sP.originalGreen = 0;
//	fontDebug ("Creating font palette");
}

int stringWidth (char * theText) {
	int a;
	int xOff = 0;
	
	if (! fontLoaded) return 0;

	for (a = 0; theText[a]; a ++) {
		xOff += theFont.sprites[fontTable[theText[a]]].width + fontSpace;
	}

	return xOff;
}

void pasteString (char * theText, int xOff, int y, spritePalette & thePal) {
	sprite * mySprite;
	int a;
	
	if (! fontLoaded) return;

	xOff += fontSpace >> 1;
	for (a = 0; theText[a]; a ++) {
		mySprite = & theFont.sprites[fontTable[theText[a]]];
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
		mySprite = & theFont.sprites[fontTable[theText[a]]];
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
		mySprite = & theFont.sprites[fontTable[theText[a]]];
		burnSpriteToBackDrop (xOff, y, * mySprite, thePal);
		xOff += mySprite -> width + fontSpace;
	}
}

void doFontColourFixLoop (spritePalette & sP) {
	int nfc = numFontColours - 2, a;
	if (nfc) {
		for (a = 0; a <= nfc; a ++) {
/*			sP.pal[a+1] = makeColour ((byte) ((a == 0 ? 255 : 0) / nfc),
									  (byte) ((a == 1 ? 255 : 0) / nfc),
									  (byte) ((a == 2 ? 255 : 0) / nfc));*/
			sP.pal[a+1] = makeColour ((byte) ((sP.originalRed * a) / nfc),
									  (byte) ((sP.originalGreen * a) / nfc),
									  (byte) ((sP.originalBlue * a) / nfc));
		}
	} else {
		sP.pal[1] = makeColour (sP.originalRed, sP.originalGreen, sP.originalBlue);
	}
}

void fixFont (spritePalette & spal) {
//	fontDebug ("Do I need to fix this font palette? Colours", spal.total);
//	fontDebug ("  Colours in sprite bank", numFontColours);
	if (spal.total != numFontColours) {
//		fontDebug ("  Yes! Deleting old palette");
		if (spal.pal) delete spal.pal;
//		fontDebug ("  Reserving new palette with entries totalling", numFontColours);
		reserveSpritePal (spal, numFontColours);
	}
	doFontColourFixLoop (spal);
}

void setFontColour (spritePalette & sP, byte r, byte g, byte b) {
//	fontDebug ("Changing font colour");
	if (sP.total != numFontColours) {
//		fontDebug ("  (Need to set up new palette first)");
		if (sP.pal) delete sP.pal;
		reserveSpritePal (sP, numFontColours);
	}
	sP.originalRed = r;
	sP.originalGreen = g;
	sP.originalBlue = b;
	doFontColourFixLoop (sP);
}

BOOL loadFont (int filenum, const char * charOrder, int h) {
	int a;

	loadedFontNum = filenum;

//	newDebug ("    Clearing font table...");
	for (a = 0; a < 256; a ++) {
		fontTable[a] = 0;
	}

//	fontDebug ("Loading font from file number", filenum);
//	fontDebug ("  Characters", charOrder);
	for (a = 0; charOrder[a]; a ++) {
		fontTable[charOrder[a]] = (byte) a;
	}

//	newDebug ("    Attempting to load file...");
	if (! loadSpriteBank (filenum, theFont)) {
//		newDebug ("    Didn't work...");
		fatal ("Can't load font");
		return FALSE;
	}

//	newDebug ("    Number of colours:", theFont.myPalette.total);
	numFontColours = theFont.myPalette.total;	
//	fontDebug ("  Colours", numFontColours);
	fontHeight = h;
	fontLoaded = TRUE;
//	newDebug ("    About to return TRUE...");
	return TRUE;
}
