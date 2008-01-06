#include "allfiles.h"
#include "zbuffer.h"
#include "fileset.h"
#include "moreio.h"
#include "newfatal.h"

zBufferData zBuffer;
extern int sceneWidth, sceneHeight;

void noZBuffer () {
	if (zBuffer.map) {
		for (int y = 0; y < sceneHeight; y ++) {
			delete zBuffer.map[y];
		}
//		zBuffer.height = 0;
//		zBuffer.loaded = FALSE;
		delete zBuffer.map;
		zBuffer.map = NULL;
	}
}

BOOL setZBuffer (int y) {
	int x, n, zBufferWidth, zBufferHeight;
	unsigned long stillToGo = 0;
	int yPalette[16];

	noZBuffer ();
//	fatal ("Loading Z-buffer");
	
	setResourceForFatal (y);

	zBuffer.originalNum = y;
	if (! openFileFromNum (y)) return FALSE;
	if (fgetc (bigDataFile) != 'S') return fatal ("Not a Z-buffer file");
	if (fgetc (bigDataFile) != 'z') return fatal ("Not a Z-buffer file");
	if (fgetc (bigDataFile) != 'b') return fatal ("Not a Z-buffer file");

	switch (fgetc (bigDataFile)) {
		case 0:
		zBufferWidth = 640;
		zBufferHeight = 480;
		break;
		
		case 1:
		zBufferWidth = get2bytes (bigDataFile);
		zBufferHeight = get2bytes (bigDataFile);
		break;
		
		default:
		return fatal ("Extended Z-buffer format not supported in this version of the SLUDGE engine");
	}
	if (zBufferWidth != sceneWidth || zBufferHeight != sceneHeight) {
		return fatal ("Z-buffer width and height don't match scene width and height");
	}
	
	x = fgetc (bigDataFile);
	for (y = 0; y < x; y ++) {
		yPalette[y] = get2bytes (bigDataFile);
	}
	zBuffer.map = new unsigned short * [sceneHeight];
	for (y = 0; y < sceneHeight; y ++) {
		zBuffer.map[y] = new unsigned short [sceneWidth];
	}

	for (y = 0; y < sceneHeight; y ++) {
		for (x = 0; x < sceneWidth; x ++) {
			if (stillToGo == 0) {
				n = fgetc (bigDataFile);
				stillToGo = n >> 4;
				if (stillToGo == 15) stillToGo = get2bytes (bigDataFile) + 16l;
				else stillToGo ++;
				n &= 15;
			}
			zBuffer.map[y][x] = yPalette[n];
			stillToGo --;
		}
	}
	finishAccess ();
	
	setResourceForFatal (-1);
	return TRUE;
}
