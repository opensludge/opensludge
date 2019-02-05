#include "variable.h"
#include "graphics.h"

#if !defined(HAVE_GLES2)
#include <GL/glew.h>
#else
#include <GLES2/gl2.h>
#endif

enum {
	LIGHTMAPMODE_NONE		= -1,
	LIGHTMAPMODE_HOTSPOT,
	LIGHTMAPMODE_PIXEL,
	LIGHTMAPMODE_NUM
};

extern unsigned int winWidth, winHeight, sceneWidth, sceneHeight;
extern int lightMapMode;

struct parallaxLayer {
	GLubyte * texture;
	GLuint textureName;
	int width, height, speedX, speedY;
	bool wrapS, wrapT;
	unsigned short fileNum, fractionX, fractionY;
	int cameraX, cameraY;
	parallaxLayer * next;
	parallaxLayer * prev;
};

bool resizeBackdrop (int x, int y);
void killBackDrop ();
void loadBackDrop (int fileNum, int x, int y);
void mixBackDrop (int fileNum, int x, int y);
void drawBackDrop ();
void blankScreen (int x1, int y1, int x2, int y2);
void darkScreen ();
void saveHSI (FILE * writer);
void saveCoreHSI (FILE * writer, GLuint texture, int w, int h);
bool loadHSI (FILE * fp, int, int, bool);
bool mixHSI (FILE * fp, int x = 0, int y = 0);
void drawHorizontalLine (unsigned int, unsigned int, unsigned int);
void drawVerticalLine (unsigned int, unsigned int, unsigned int);
void hardScroll (int distance);
bool getRGBIntoStack (unsigned int x, unsigned int y, stackHandler * sH);

// Also the light map stuff

void killLightMap ();
bool loadLightMap (int v);

extern texture lightMap;


// And background parallax scrolling

void killParallax ();
bool loadParallax (unsigned short v, unsigned short fracX, unsigned short fracY);
void saveParallaxRecursive (parallaxLayer * me, FILE * fp);
void reloadParallaxTextures ();

void nosnapshot ();
bool snapshot ();
void saveSnapshot(FILE * fp);
bool restoreSnapshot (FILE * fp);
