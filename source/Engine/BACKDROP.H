#include "variable.h"

enum {
	LIGHTMAPMODE_NONE		= -1,
	LIGHTMAPMODE_HOTSPOT,
	LIGHTMAPMODE_PIXEL,
	LIGHTMAPMODE_NUM
};

extern int winWidth, winHeight, sceneWidth, sceneHeight, lightMapMode;

struct parallaxLayer {
	unsigned short int * * parallaxImage;
	int width, height, speedX, speedY;
	unsigned short fileNum, wrapX, wrapY, fractionX, fractionY;
	unsigned int cameraX, cameraY;
	parallaxLayer * next;
};

BOOL resizeBackdrop (int x, int y);
void killBackDrop ();
void loadBackDrop (int fileNum, int x, int y);
void mixBackDrop (int fileNum, int x, int y);
void backDropToScreenBuffer ();
void copyToBackDrop (unsigned short int * * fromHere, int orW, int orH, int orX, int orY, parallaxLayer * parallaxS);
void blankScreen (int x1, int y1, int x2, int y2);
void darkScreen ();
void saveHSI (FILE * writer);
void saveCoreHSI (FILE * writer, unsigned short int * * image, int w, int h);
BOOL loadHSI (FILE * fp, int, int, BOOL);
BOOL mixHSI (FILE * fp, int x = 0, int y = 0);
void drawHorizontalLine (unsigned int, unsigned int, unsigned int);
void drawVerticalLine (unsigned int, unsigned int, unsigned int);
void hardScroll (int distance);
BOOL getRGBIntoStack (unsigned int x, unsigned int y, stackHandler * sH);

// Also the light map stuff

void killLightMap ();
BOOL loadLightMap (int v);

// And background parallax scrolling

void killParallax ();
BOOL loadParallax (unsigned short v, unsigned short fracX, unsigned short fracY);
void saveParallaxRecursive (parallaxLayer * me, FILE * fp);

void nosnapshot ();
BOOL snapshot ();
BOOL restoreSnapshot (FILE * fp);
