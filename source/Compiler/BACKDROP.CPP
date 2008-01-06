#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <io.h>

#include "moreio.h"
#include "splitter.h"
#include "wintext.h"
#include "winterfa.h"
#include "messbox.h"
#include "tga.h"

//extern unsigned short int * screen;
unsigned short int * * backDropImage;
int VERT_RES, HORZ_RES;

BOOL initBackDrop (int x, int y) {
	int a;

	HORZ_RES = x;
	VERT_RES = y;

	backDropImage = new unsigned short int * [y];
	if (backDropImage == NULL) return FALSE;
	for (a = 0; a < y; a ++) {
		backDropImage[a] = new unsigned short int [x];
		if (backDropImage[a] == NULL) return FALSE;
	}
	return TRUE;
}

int loadBackDrop (char * fileName) {
	unsigned short int * toScreen;
	unsigned short int t1, t2;
	palCol thePalette[256];
	// Open the file
	
	FILE * fp = fopen (fileName, "rb");
	if (fp == NULL) {
		return FALSE;
	}
	
	// Grab the header

	TGAHeader imageHeader;
	char * errorBack;
	errorBack = readTGAHeader (imageHeader, fp, thePalette);
	if (errorBack) {
//		alert (errorBack);
		return FALSE;		
	}

	initBackDrop (imageHeader.width, imageHeader.height);
	
	unsigned short (* readColFunction) (FILE *, int, palCol[], int, int) =
		imageHeader.compressed ? readCompressedColour : readAColour;

	for (t2 = imageHeader.height; t2; t2 --) {
		toScreen = backDropImage[(imageHeader.imageDescriptor & 32) ? (imageHeader.height - t2) : (t2 - 1)];
		for (t1 = 0; t1 < imageHeader.width; t1 ++) {
			* (toScreen ++) = readColFunction (fp, imageHeader.pixelDepth, thePalette, t1, t2);
		}
	}
	
	int i = ftell (fp);
	fclose (fp);
	return i;
}
/*
void backDropToScreenBuffer () {
	unsigned short int a, b;
	unsigned short int * fromHere;
	unsigned short int * toScreen = screen;

	for (a = 0; a < VERT_RES; a ++) {
		fromHere = backDropImage[a];
		for (b = 0; b < HORZ_RES; b ++) {
			* (toScreen ++) = * (fromHere ++);
//			* (toScreen ++) = a;
		}
	}
}*/

int saveHSI (char * filename) {
	FILE * writer = fopen (filename, "wb");
	int x, y, lookAhead;
	unsigned short int * fromHere;
	unsigned short int * lookPointer;

	if (! writer) return 0;
	put2bytes (HORZ_RES, writer);
	put2bytes (VERT_RES, writer);
	for (y = 0; y < VERT_RES; y ++) {
		fromHere = backDropImage[y];
		x = 0;
		while (x < HORZ_RES) {
//			if (* fromHere & 32) errorBox ("OK, so I was wrong...");
			lookPointer = fromHere + 1;
			for (lookAhead = x + 1; lookAhead < HORZ_RES; lookAhead ++) {
				if (lookAhead - x == 256) break;
				if (* fromHere != * lookPointer) break;
				lookPointer ++;
			}
			if (lookAhead == x + 1) {
				put2bytes (* fromHere, writer);
			} else {
				put2bytes (* fromHere | 32, writer);
				fputc (lookAhead - x - 1, writer);
			}
			fromHere = lookPointer;
			x = lookAhead;
		}
	}
	int i = ftell (writer);
	fclose (writer);
	return i;
}
/*
BOOL newerFile (char * newFileN, char * oldFileN) {
	struct stat oldFileStat, newFileStat;
	if (stat (oldFileN, & oldFileStat)) return TRUE;
	if (stat (newFileN, & newFileStat)) return TRUE;
//	char buff[200];
//	sprintf (buff, "Difference between %s and %s is %i", oldFileN, newFileN, (int) difftime (oldFileStat.st_mtime, newFileStat.st_mtime));
//	if (difftime (oldFileStat.st_mtime, newFileStat.st_mtime) <= 0) errorBox ("Backdrop report", buff);
	return difftime (oldFileStat.st_mtime, newFileStat.st_mtime) <= 0;
}
*/

BOOL convertTGA (char * filename) {
	char * originalName = joinStrings (filename, "");
	filename[strlen (filename) - 3] = 's';
	filename[strlen (filename) - 2] = 'l';
	filename[strlen (filename) - 1] = getDither() ? '2' : 'x';
	int i, j;

	if (newerFile (originalName, filename)) {
		setWindowText (COM_ITEMTEXT, "Compressing image");
//		addComment ("Compressing ", originalName);
		i = loadBackDrop (originalName);
		if (! i) return FALSE;
		j = saveHSI (filename);
		if (! j) return FALSE;
//		{
//			char blah[255];
//			sprintf (blah, "Compressed %s...", originalName);
//			addComment (blah);
//			int p = ((float)j)/i * 100;
//			sprintf (blah, "     From %d to %d bytes (%d%%)", i, j, p);
//			addComment (blah);
//		}
		for (int a = 0; a < VERT_RES; a ++) delete backDropImage[a];
		delete backDropImage;
		setWindowText (COM_ITEMTEXT, "");
	}
	delete originalName;
	return TRUE;
}
