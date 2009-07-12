//#include <windows.h>
#include <stdio.h>

#include "moreio.h"
#include "backdrop.h"
#include "zbuffer.h"
#include "tga.h"

extern unsigned short int * screen;
unsigned short int * * backDropImage = NULL;
int VERT_RES, HORZ_RES, xShift = 0, yShift = 0;

#ifndef HWND
#define HWND int
#endif
#ifndef HDC
#define HDC int
#endif

extern HWND hMainWindow;
extern HDC bufferDC;
extern int numPanels;
extern zPanel panel[16];

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

void blankBackDrop () {
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
	blankBackDrop ();
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
	
	unsigned short (* readColFunction) (FILE * fp, int bpc, palCol thePalette[], int x, int y) =
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
	/* TODO
	if (hMainWindow) {
		HDC hdc = GetDC (hMainWindow);
		BitBlt (hdc, 0, 0, WINWIDTH, WINHEIGHT, bufferDC, 0, 0, SRCCOPY);
		ReleaseDC (hMainWindow, hdc);
	}
	*/
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

char filebuffer[12] = {0, 1, 1, 0, 0, 0, 1, 24, 0, 0, 0, 0};

bool saveBackdropToTGA (char * filename) {
	FILE * create;
	int index, x, n;

	create = fopen (filename, "wb");
	if (create != NULL) {
		fwrite (filebuffer, 12, 1, create);
		put2bytesR (HORZ_RES, create);
		put2bytesR (VERT_RES, create);
		put2bytesR (8, create);

		// Put in all the colours...
		for (index = 0; index < numPanels; index ++) {
			fputc (blueValue  (panel[index].theColour), create);
			fputc (greenValue (panel[index].theColour), create);
			fputc (redValue   (panel[index].theColour), create);
		}
		
		for (index = numPanels; index < 256; index ++) {
			fputc (0, create);
			fputc (0, create);
			fputc (0, create);
		}
		
		for (index = VERT_RES - 1; index >= 0; index --) {
			unsigned short * fromHere = backDropImage[index];
			for (x = 0; x < HORZ_RES; x ++) {
				for (n = 0; n < numPanels; n ++) {
					if (panel[n].theColour == *fromHere) break;
				}
				fromHere ++;

				if (n > 15) n = 0;
				fputc (n, create);
			}
		}
		fclose (create);
		return true;
	}
	return false;
}
