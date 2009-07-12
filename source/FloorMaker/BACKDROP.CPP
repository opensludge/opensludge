//#include <windows.h>
#include <stdio.h>

#include "backdrop.h"
#include "tga.h"

/* TODO

extern unsigned short int * screen;
unsigned short int * * backDropImage = NULL;
int VERT_RES, HORZ_RES, xShift = 0, yShift = 0;
extern HWND hMainWindow;
extern HDC bufferDC;

void alert (char * txt);

void deleteBackdrop () {
	int y;

	if (backDropImage) {
		for (y = 0; y < VERT_RES; y ++) {
			delete backDropImage[y];
			y ++;
		}
		delete backDropImage;
		backDropImage = NULL;
	}
}

void blankScreen () {
	unsigned short int * toScreen;
	for (int t2 = 0; t2 < VERT_RES; t2 ++) {
		toScreen = backDropImage[t2];
		for (int t1 = 0; t1 < HORZ_RES; t1 ++) {
			* (toScreen) = 0;
			toScreen ++;
		}
	}
}

bool initBackDrop (int x, int y) {
	int a;

	deleteBackdrop ();

	if (x < WINWIDTH) x = WINWIDTH;
	if (y < WINHEIGHT) y = WINHEIGHT;

	HORZ_RES = x;
	VERT_RES = y;
	xShift = 0;
	yShift = 0;

	backDropImage = new unsigned short int * [y];
	if (backDropImage == NULL) return false;
	for (a = 0; a < y; a ++) {
		backDropImage[a] = new unsigned short int [x];
		if (backDropImage[a] == NULL) return false;
	}
	blankScreen ();
	return true;
}

bool loadBackDrop (char * fileName) {
	unsigned short int * toScreen;
	unsigned short int t1, t2;
	palCol thePalette[256];
	// Open the file
	
	FILE * fp = fopen (fileName, "rb");
	if (fp == NULL) {
		alert ("Can't open that image file!");
		return false;
	}
	
	// Grab the header

	TGAHeader imageHeader;
	char * errorBack;
	errorBack = readTGAHeader (imageHeader, fp, thePalette);
	if (errorBack) {
		alert (errorBack);
		return false;		
	}
	
	unsigned short (* readColFunction) (FILE *, int, palCol[], int, int) =
		imageHeader.compressed ? readCompressedColour : readAColour;
		
	initBackDrop (imageHeader.width, imageHeader.height);

	for (t2 = imageHeader.height; t2; t2 --) {
		toScreen = backDropImage[(imageHeader.imageDescriptor & 32) ? (imageHeader.height - t2) : (t2 - 1)];
		for (t1 = 0; t1 < imageHeader.width; t1 ++) {
			* (toScreen ++) = readColFunction (fp, imageHeader.pixelDepth, thePalette, 0, 0);
		}
	}
	fclose (fp);
	return true;
}

void screenBufferToWindow () {
	if (hMainWindow) {
		HDC hdc = GetDC (hMainWindow);
		BitBlt (hdc, 0, 0, WINWIDTH, WINHEIGHT, bufferDC, 0, 0, SRCCOPY);
		ReleaseDC (hMainWindow, hdc);
	}
}

void backDropToScreenBuffer () {
	unsigned short int a, b;
	unsigned short int * fromHere;
	unsigned short int * toScreen = screen;

	if (backDropImage) {
		for (a = 0; a < WINHEIGHT; a ++) {
			fromHere = backDropImage[a + yShift] + xShift;
			for (b = 0; b < WINWIDTH; b ++) {
				* (toScreen ++) = * (fromHere ++);
			}
		}
	}
	
	screenBufferToWindow ();
}

*/