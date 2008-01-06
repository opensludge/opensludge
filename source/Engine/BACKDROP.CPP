#include "allfiles.h"

#include "winstuff.h"
#include "newfatal.h"
#include "colours.h"
#include "fileset.h"
#include "cursors.h"
#include "backdrop.h"
#include "moreio.h"
#include "variable.h"
#include "zbuffer.h"

BOOL freeze ();
void unfreeze (BOOL killImage = TRUE);

unsigned short int * * backDropImage = NULL;
unsigned short int * * lightMapImage = NULL;
unsigned short int * * snapshotImage = NULL;

int lightMapMode = LIGHTMAPMODE_PIXEL;

parallaxLayer * parallaxStuff = NULL;

int cameraPX = 0, cameraPY = 0;

int sceneWidth, sceneHeight, lightMapNumber;
unsigned int currentBlankColour = makeColour (0, 0, 0);

extern unsigned short int * screen;
extern int cameraX, cameraY;

void nosnapshot () {
	if (snapshotImage) {
		for (int a = 0; a < winHeight; a ++) delete snapshotImage[a];
		delete snapshotImage;
		snapshotImage = NULL;
	}
}

BOOL snapshot () {
	nosnapshot ();
	if (! freeze ()) return FALSE;
	snapshotImage = backDropImage;
	unfreeze (FALSE);
	return TRUE;
}

BOOL restoreSnapshot (FILE * fp) {
	int picWidth = get2bytes (fp);
	int picHeight = get2bytes (fp);
	
	if (picWidth != winWidth || picHeight != winHeight) {
		return fatal ("Snapshot width and height don't match window width and height.");
	}
	
	snapshotImage = new unsigned short int * [sceneHeight];
	if (snapshotImage == NULL) return fatal ("Out of memory loading snapshot.");

	for (int a = 0; a < winHeight; a ++) {
		snapshotImage[a] = new unsigned short int [winWidth];
		if (snapshotImage[a] == NULL) return fatal ("Out of memory loading snapshot.");
	}

	int t1, t2, n;
	unsigned short c;
	unsigned short int * toScreen;

	for (t2 = 0; t2 < picHeight; t2 ++) {
		toScreen = snapshotImage[t2];
		t1 = 0;
		while (t1 < picWidth) {
			c = (unsigned short) get2bytes (fp);
			if (c & 32) {
				n = fgetc (fp) + 1;
				c -= 32;
			} else {
				n = 1;
			}
			while (n) {
				n --;
				* (toScreen ++) = c;
				t1 ++;
			}
		}
	}
	return TRUE;
}

BOOL reserveNewLightMap () {
	int a;
	lightMapImage = new unsigned short int * [sceneHeight];
	if (lightMapImage == NULL) return FALSE;
	for (a = 0; a < sceneHeight; a ++) {
		lightMapImage[a] = new unsigned short int [sceneWidth];
		if (lightMapImage[a] == NULL) return FALSE;
	}
	return TRUE;
}

void killBackDrop () {
	if (backDropImage) {
		for (int a = 0; a < sceneHeight; a ++) delete backDropImage[a];
		delete backDropImage;
		backDropImage = NULL;
	}
}

void killLightMap () {
	if (lightMapImage) {
		for (int a = 0; a < sceneHeight; a ++) delete lightMapImage[a];
		delete lightMapImage;
		lightMapImage = NULL;
	}
}

void killParallax () {
	while (parallaxStuff) {
		parallaxLayer * k = parallaxStuff;
		parallaxStuff = k -> next;
		
		// Now kill the image
		int a;
		for (a = 0; a < k -> height; a ++) delete k -> parallaxImage[a];
		delete k -> parallaxImage;
		delete k;
	}
}

BOOL reserveBackdrop () {
	int a;
	cameraX = 0;
	cameraY = 0;
	backDropImage = new unsigned short int * [sceneHeight];
	if (backDropImage == NULL) return FALSE;
	for (a = 0; a < sceneHeight; a ++) {
		backDropImage[a] = new unsigned short int [sceneWidth];
		if (backDropImage[a] == NULL) return FALSE;
	}
	return TRUE;
}

BOOL resizeBackdrop (int x, int y) {
	killLightMap ();
	killBackDrop ();
	killParallax ();
	noZBuffer ();
	sceneWidth = x;
	sceneHeight = y;
	return reserveBackdrop ();
}

void loadBackDrop (int fileNum, int x, int y) {
	setResourceForFatal (fileNum);

	if (! openFileFromNum (fileNum)) {
		fatal ("Can't load overlay image");
		return;
	}

	if (! loadHSI (bigDataFile, x, y, FALSE)) {
		char mess[200];
		sprintf (mess, "Can't paste overlay image outside scene dimensions\n\nX = %i\nY = %i\nWidth = %i\nHeight = %i", x, y, sceneWidth, sceneHeight);
		fatal (mess);
	}

	finishAccess ();
	setResourceForFatal (-1);
}

void mixBackDrop (int fileNum, int x, int y) {
	setResourceForFatal (fileNum);
	if (! openFileFromNum (fileNum)) {
		fatal ("Can't load overlay image");
		return;
	}

	if (! mixHSI (bigDataFile, x, y)) {
		fatal ("Can't paste overlay image outside screen dimensions");
	}

	finishAccess ();
	setResourceForFatal (-1);
}

void blankScreen (int x1, int y1, int x2, int y2) {
	unsigned short int * toScreen;
	int y, x;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (x2 >= sceneWidth) x2 = sceneWidth - 1;
	if (y2 >= sceneHeight) y2 = sceneHeight - 1;

	for (y = y1; y <= y2; y ++) {
		toScreen = backDropImage[y] + x1;
		for (x = x1; x <= x2; x ++) {
			* (toScreen ++) = currentBlankColour;
		}
	}
}

void hardScroll (int distance) {
	if (abs (distance) >= sceneHeight) {
		blankScreen (0, 0, sceneWidth, sceneHeight);
		return;
	}
	
	if (! distance) return;
	
	if (distance > 0) {
		for (int t = 0; t < distance; t ++) {
			unsigned short * remember = backDropImage[t];
			int lastY = t;
			for (int y = t + distance; y < sceneHeight; y += distance) {
				backDropImage[lastY] = backDropImage[y];
				lastY = y;
			}
			backDropImage[lastY] = remember;
			for (int x = 0; x < sceneWidth; x ++) * (remember ++) = currentBlankColour;
		}
	} else {
		for (int t = sceneHeight - 1; t >= sceneHeight + distance; t --) {
			unsigned short * remember = backDropImage[t];
			int lastY = t;
			for (int y = t + distance; y >= 0; y += distance) {
				backDropImage[lastY] = backDropImage[y];
				lastY = y;
			}
			backDropImage[lastY] = remember;
			for (int x = 0; x < sceneWidth; x ++) * (remember ++) = currentBlankColour;
		}
	}
}

void drawVerticalLine (unsigned int x, unsigned int y1, unsigned int y2) {
	if (x < sceneWidth) {
		for (unsigned int a = y1; a <= y2; a ++) {
			if (a < sceneHeight) {
				backDropImage[a][x] ^= 0xFFFF;
			}
		}
	}
}

void drawHorizontalLine (unsigned int x1, unsigned int y, unsigned int x2) {
	if (y < sceneHeight) {
		for (unsigned int a = x1; a <= x2; a ++) {
			if (a < sceneWidth) {
				backDropImage[y][a] ^= 0xFFFF;
			}
		}
	}
}

void darkScreen () {
	unsigned short int * toScreen;
	int y, x;

	for (y = 0; y < sceneHeight; y ++) {
		toScreen = backDropImage[y];
		for (x = 0; x < sceneWidth; x ++) {
			* toScreen = (unsigned short) (((* toScreen) >> 1) & 31727u);
			toScreen ++;
		}
	}
}

inline int sortOutPCamera (int cX, int fX, int sceneMax, int boxMax) {
	return (fX == 65535) ?
		(sceneMax ? ((cX * boxMax) / sceneMax) : 0)
	:
		((cX * fX) / 100);
}

void copyToBackDrop (unsigned short int * * fromHere, int orW, int orH, int orX, int orY, parallaxLayer * parallaxS) {
	unsigned short int * toScreen;
	unsigned short int * fromScreen;
	int y, x;

	if (parallaxS) {
		parallaxLayer * ps = parallaxS;

		while (ps) {
			ps -> cameraX = sortOutPCamera (orX, ps -> fractionX, orW - winWidth, ps -> width - winWidth);
			ps -> cameraY = sortOutPCamera (orY, ps -> fractionY, orH - winHeight, ps -> height - winHeight);
			ps = ps -> next;
		}

		for (y = 0; y < winHeight; y ++) {
			toScreen = backDropImage[y];
			fromScreen = fromHere[y + orY] + orX;

			for (x = 0; x < winWidth; x ++) {
				ps = parallaxS;
				unsigned int col = * (fromScreen ++);
				
				while (col == 2015 && ps) {
					col = ps -> parallaxImage[(y + ps -> cameraY) % ps -> wrapY][(x + ps -> cameraX) % ps -> wrapX];
					ps = ps -> next;
				}
				
				* (toScreen ++) = col;				
			}
		}
	} else {
		for (y = 0; y < winHeight; y ++) {
			toScreen = backDropImage[y];
			fromScreen = fromHere[y + orY] + orX;
			for (x = 0; x < winWidth; x ++) {
				* (toScreen ++) = * (fromScreen ++);
			}
		}
	}
}

void backDropToScreenBuffer () {
	unsigned short int a, b;
	unsigned short int * fromHere;
	unsigned short int * toScreen = screen;

//	bgDebug ("Starting backDropToScreenBuffer");
	
	if (parallaxStuff) {
//		bgDebug ("Got parallaxStuff");
		parallaxLayer * ps = parallaxStuff;

		while (ps) {
			ps -> cameraX = sortOutPCamera (cameraX, ps -> fractionX, sceneWidth - winWidth, ps -> width - winWidth);
			ps -> cameraY = sortOutPCamera (cameraY, ps -> fractionY, sceneHeight - winHeight, ps -> height - winHeight);
			ps = ps -> next;
		}
//		bgDebug ("Done cameras");

		for (a = 0; a < winHeight; a ++) {
			fromHere = backDropImage[a + cameraY] + cameraX;
						
			for (b = 0; b < winWidth; b ++) {
				ps = parallaxStuff;
				unsigned int col = * (fromHere ++);
				
				while (col == 2015 && ps) {
					col = ps -> parallaxImage[(a + ps -> cameraY) % ps -> wrapY][(b + ps -> cameraX) % ps -> wrapX];
					ps = ps -> next;
				}
				
				* (toScreen ++) = col;
			}
		}
	} else {
		for (a = 0; a < winHeight; a ++) {
			fromHere = backDropImage[a + cameraY] + cameraX;
			for (b = 0; b < winWidth; b ++) {
				* (toScreen ++) = * (fromHere ++);
			}
		}
	}	
//	bgDebug ("Drawn");
}

BOOL isAPowerOf2 (int j) {
	int d = 1;
	while (d >= 0) {
		if (d == j) return TRUE;
		d <<= 1;
	}
	return FALSE;
}

BOOL loadLightMap (int v) {
	setResourceForFatal (v);
	lightMapNumber = v;
	if (! openFileFromNum (v)) return fatal ("Can't open light map.");

	int picWidth = get2bytes (bigDataFile);
	int picHeight = get2bytes (bigDataFile);
	
	if (picWidth != sceneWidth || picHeight != sceneHeight) {
		return fatal ("Light map width and height don't match scene width and height.");
	}
	
	killLightMap ();
	if (! reserveNewLightMap ()) {
		return fatal ("Out of memory loading light map.");
	}

	int t1, t2, n;
	unsigned short c;
	unsigned short int * toScreen;

	for (t2 = 0; t2 < picHeight; t2 ++) {
		toScreen = lightMapImage[t2];
		t1 = 0;
		while (t1 < picWidth) {
			c = (unsigned short) get2bytes (bigDataFile);
			if (c & 32) {
				n = fgetc (bigDataFile) + 1;
				c -= 32;
			} else {
				n = 1;
			}
			while (n) {
				n --;
				* (toScreen ++) = c;
				t1 ++;
			}
		}
	}

	finishAccess ();
	setResourceForFatal (-1);
	return TRUE;
}

BOOL loadParallax (unsigned short v, unsigned short fracX, unsigned short fracY) {
	setResourceForFatal (v);
	if (! openFileFromNum (v)) return fatal ("Can't open parallax image");

	parallaxLayer * nP = new parallaxLayer;
	if (! checkNew (nP)) return FALSE;
	
	nP -> next = parallaxStuff;
	parallaxStuff = nP;

	nP -> width = get2bytes (bigDataFile);
	nP -> height = get2bytes (bigDataFile);

	nP -> fileNum = v;
	nP -> fractionX = fracX;
	nP -> fractionY = fracY;

	if (fracX == 65535) {
		nP -> wrapX = 0xFFFFFFFF;
		if (nP -> width < winWidth) {
			fatal ("For AUTOFIT parallax backgrounds, the image must be at least as wide as the game window/screen.");
			return FALSE;
		}		
	} else {
		if (! isAPowerOf2 (nP -> width)) {
			fatal ("Image width is not a power of 2!");
			return FALSE;
		}
		nP -> wrapX = nP -> width - 1;	
	}

	if (fracY == 65535) {
		nP -> wrapY = 0xFFFFFFFF;
		if (nP -> height < winHeight) {
			fatal ("For AUTOFIT parallax backgrounds, the image must be at least as tall as the game window/screen.");
			return FALSE;
		}		
	} else {
		if (! isAPowerOf2 (nP -> height)) {
			fatal ("Image height is not a power of 2!");
			return FALSE;
		}
		nP -> wrapY = nP -> height - 1;	
	}
		
	nP -> parallaxImage = new unsigned short int * [nP -> height];
	
	if (! checkNew (nP -> parallaxImage)) return FALSE;
	
	for (int a = 0; a < nP -> height; a ++) {
		nP -> parallaxImage[a] = new unsigned short int [nP -> width];
		if (nP -> parallaxImage[a] == NULL) return FALSE;
	}

	int t1, t2, n;
	unsigned short c;
	unsigned short int * toScreen;

	for (t2 = 0; t2 < nP -> height; t2 ++) {
		toScreen = nP -> parallaxImage[t2];
		t1 = 0;
		while (t1 < nP -> width) {
			c = (unsigned short) get2bytes (bigDataFile);
			if (c & 32) {
				n = fgetc (bigDataFile) + 1;
				c -= 32;
			} else {
				n = 1;
			}
			while (n) {
				n --;
				* (toScreen ++) = c;
				t1 ++;
			}
		}
	}

	finishAccess ();
	setResourceForFatal (-1);
	return TRUE;
}

BOOL loadHSI (FILE * fp, int x, int y, BOOL reserve) {
	int picWidth = get2bytes (fp);
	int picHeight = get2bytes (fp);
	int t1, t2, n;
	unsigned short c;
	unsigned short int * toScreen;
	signed long transCol = reserve ? -1 : 63519;
	
	if (reserve) {
		if (! resizeBackdrop (picWidth, picHeight)) return FALSE;
	}

	if (x == IN_THE_CENTRE) x = (sceneWidth - picWidth) >> 1;
	if (y == IN_THE_CENTRE) y = (sceneHeight - picHeight) >> 1;
	
	if (x < 0 || x + picWidth > sceneWidth || y < 0 || y + picHeight > sceneHeight) return FALSE;
	
	for (t2 = 0; t2 < picHeight; t2 ++) {
		toScreen = backDropImage[t2 + y] + x;
		t1 = 0;
		while (t1 < picWidth) {
			c = (unsigned short) get2bytes (fp);
			if (c & 32) {
				n = fgetc (fp) + 1;
				c -= 32;
			} else {
				n = 1;
			}
			t1 += n;
			while (n --) {
				if (c != transCol) * (toScreen) = c;
				toScreen ++;
			}
		}
	}

	return TRUE;
}

void drawModeTransparent2 (unsigned short int * scrCol, const unsigned short int & sprCol);

BOOL mixHSI (FILE * fp, int x, int y) {
	int picWidth = get2bytes (fp);
	int picHeight = get2bytes (fp);
	int t1, t2, n;
	unsigned short c;
	unsigned short int * toScreen;

	if (x == IN_THE_CENTRE) x = (sceneWidth - picWidth) >> 1;
	if (y == IN_THE_CENTRE) y = (sceneHeight - picHeight) >> 1;

	if (x < 0 || x + picWidth > sceneWidth || y < 0 || y + picHeight > sceneHeight) return FALSE;

	for (t2 = 0; t2 < picHeight; t2 ++) {
		toScreen = backDropImage[y + t2] + x;
		t1 = 0;
		while (t1 < picWidth) {
			c = (unsigned short) get2bytes (fp);
			if (c & 32) {
				n = fgetc (fp) + 1;
				c -= 32;
			} else {
				n = 1;
			}
			while (n) {
				n --;
				if (c != 63519) drawModeTransparent2 (toScreen, c);
				toScreen ++;
				t1 ++;
			}
		}
	}
	return TRUE;
}

void saveCoreHSI (FILE * writer, unsigned short int * * image, int w, int h) {
	int x, y, lookAhead;
	unsigned short int * fromHere, * lookPointer;

	put2bytes (w, writer);
	put2bytes (h, writer);
	
	for (y = 0; y < h; y ++) {
		fromHere = image[y];
		x = 0;
		while (x < w) {
			lookPointer = fromHere + 1;
			for (lookAhead = x + 1; lookAhead < w; lookAhead ++) {
				if (lookAhead - x == 256) break;
				if (* fromHere != * lookPointer) break;
				lookPointer ++;
			}
			if (lookAhead == x + 1) {
				put2bytes ((* fromHere) & 65503, writer);
			} else {
				put2bytes (* fromHere | 32, writer);
				fputc (lookAhead - x - 1, writer);
			}
			fromHere = lookPointer;
			x = lookAhead;
		}
	}
}

void saveHSI (FILE * writer) {
	saveCoreHSI (writer, backDropImage, sceneWidth, sceneHeight);
}

BOOL getRGBIntoStack (unsigned int x, unsigned int y, stackHandler * sH) {

	if (x >= sceneWidth || y >= sceneHeight) {
		return fatal ("Co-ordinates are outside current scene!");
	}

	variable newValue;
	
	newValue.varType = SVT_NULL;
	int gotColour = backDropImage[y][x];
	
	setVariable (newValue, SVT_INT, blueValue(gotColour));
	if (! addVarToStackQuick (newValue, sH -> first)) return FALSE;
	sH -> last = sH -> first;

	setVariable (newValue, SVT_INT, greenValue(gotColour));
	if (! addVarToStackQuick (newValue, sH -> first)) return FALSE;

	setVariable (newValue, SVT_INT, redValue(gotColour));
	if (! addVarToStackQuick (newValue, sH -> first)) return FALSE;

	return TRUE;
}

void saveParallaxRecursive (parallaxLayer * me, FILE * fp) {
	if (me) {
		saveParallaxRecursive (me -> next, fp);
		fputc (1, fp);
		put2bytes (me->fileNum, fp);
		put2bytes (me ->fractionX, fp);
		put2bytes (me->fractionY, fp);
	}
}

