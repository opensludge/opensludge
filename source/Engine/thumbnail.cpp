#include "allfiles.h"
#include "errors.h"
#include "moreio.h"
#include "version.h"
#include "sludger.h"
#include "colours.h"
#include "backdrop.h"

BOOL freeze ();
void unfreeze (BOOL);	// Because freeze.h needs a load of other includes

int thumbWidth = 0, thumbHeight = 0;

unsigned short int * * backDropImage = NULL;

BOOL saveThumbnail (FILE * fp) {
	put4bytes (thumbWidth, fp);
	put4bytes (thumbHeight, fp);
	if (thumbWidth && thumbHeight) {
		if (! freeze ()) return FALSE;
		for (int y = 0; y < thumbHeight; y ++) {
			for (int x = 0; x < thumbWidth; x ++) {
				unsigned long redTotal = 0;
				unsigned long greenTotal = 0;
				unsigned long blueTotal = 0;
				unsigned long div = 0;
				unsigned int yStart = (y * winHeight) / thumbHeight;
				unsigned int xStart = (x * winWidth) / thumbWidth;
				unsigned int yStop = ((y+1) * winHeight) / thumbHeight;
				unsigned int xStop = ((x+1) * winWidth) / thumbWidth;
				
				for (unsigned int yy = yStart; yy < yStop; yy ++) {
					for (unsigned int xx = xStart; xx < xStop; xx ++) {
						
						redTotal += redValue (backDropImage[yy][xx]);
						greenTotal += greenValue (backDropImage[yy][xx]);
						blueTotal += blueValue (backDropImage[yy][xx]);
						div ++;
					}
				}
				put2bytes (makeColour (redTotal/div, greenTotal/div, blueTotal/div), fp);
			}
		}
		unfreeze (TRUE);
	}		
	fputc ('!', fp);
	return TRUE;
}

void showThumbnail (char * filename, int atX, int atY) {
	int ssgVersion;
	FILE * fp = openAndVerify (filename, 'S', 'A', ERROR_GAME_LOAD_NO, ssgVersion);
	if (ssgVersion >= VERSION(1,4)) {
		if (fp == NULL) return;
		int fileWidth = get4bytes (fp);
		int fileHeight = get4bytes (fp);
		for (int y = 0; y < fileHeight; y ++) {
			for (int x = 0; x < fileWidth; x ++) {
				backDropImage[atY + y][atX + x] = get2bytes (fp);
			}
		}
		fclose (fp);
	}
}

BOOL skipThumbnail (FILE * fp) {
	thumbWidth = get4bytes (fp);
	thumbHeight = get4bytes (fp);
	unsigned long skippy = thumbWidth;
	skippy *= thumbHeight << 1;
	fseek (fp, skippy, 1);
	return (fgetc (fp) == '!');
}

/*BOOL freeze () {
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

void unfreeze () {
	frozenStuffStruct * killMe = frozenStuff;

	if (! frozenStuff) return;

	killAllPeople ();
	allPeople = frozenStuff -> allPeople;

	killAllRegions ();
	allScreenRegions = frozenStuff -> allScreenRegions;

	killBackDrop ();
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

*/