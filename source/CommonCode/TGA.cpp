#include <windows.h>
#include <stdio.h>

#include "TGA.h"

//FILE * debugFile = fopen ("TGAdebug.txt", "wt");

unsigned short int makeColour (byte r, byte g, byte b) {
	unsigned short int reply = (unsigned short int) (r >> 3);
	reply <<= 6;
	reply += (unsigned short int) (g >> 2);
	reply <<= 5;
	reply += (unsigned short int) (b >> 3);
	return reply & 65503;
}

int get2bytesReverse (FILE * fp) {
	int a = fgetc (fp);
	return a + fgetc (fp) * 256;
}

int countDown = 0;

bool dither24bitImages = 0;

char ditherArray[4][4] = {{4,12,6,14},{10,0,8,2},{7,15,5,13},{9,3,11,1}};

void grabRGB (FILE * fp, int bpc, byte & r, byte & g, byte & b, palCol thePalette[])
{
	int grabbed1, grabbed2;
	switch (bpc) {
		case 8:
		grabbed1 = fgetc (fp);
		r = thePalette[grabbed1].r;
		g = thePalette[grabbed1].g;
		b = thePalette[grabbed1].b;
		break;
	
		case 16:
		grabbed1 = fgetc (fp);
		grabbed2 = fgetc (fp);
		r = ((grabbed2 & 127) << 1),
		g = ((grabbed1 & 224) >> 2) + (grabbed2 << 6);
		b = ((grabbed1 & 31) << 3);
		break;
				
		case 24:
		b = fgetc (fp);
		g = fgetc (fp);
		r = fgetc (fp);
		break;
		
		case 32:
		b = fgetc (fp);
		g = fgetc (fp);
		r = fgetc (fp);
		fgetc (fp);
		break;		
	}
}

void addDither (byte & col, const byte add)
{
	int tot = col;
	tot += add;
	col = (tot > 255) ? 255 : tot;
}

unsigned short readAColour (FILE * fp, int bpc, palCol thePalette[], int x, int y) {
	byte r,g,b;
	grabRGB (fp, bpc, r, g, b, thePalette);

	if (dither24bitImages)
	{
		addDither (r, ditherArray[x&3][y&3]);
		addDither (g, ditherArray[x&3][y&3] / 2);
		addDither (b, ditherArray[x&3][y&3]);
	}

	return makeColour (r, g, b);
}

unsigned short readCompressedColour (FILE * fp, int bpc, palCol thePalette[], int x, int y) {
	static byte r, g, b;
	byte r2, g2, b2;
	static BOOL oneCol;
	unsigned short col;
	
	// Do we have to start a new packet?
	if (countDown == 0) {
			
		// Read the packet description thingy
		col = fgetc (fp);
				
		// Is it raw data?
		if (col >= 128) {
			oneCol = TRUE;
			countDown = col - 127;
			grabRGB (fp, bpc, r, g, b, thePalette);
//			fprintf (debugFile, "  %d raw colours...\n", countDown);
		} else {
			oneCol = FALSE;
			countDown = col + 1;
//			fprintf (debugFile, "  %d pixels the same colour...\n", countDown);
		}
	}

	countDown --;

	if (! oneCol) {
		grabRGB (fp, bpc, r2, g2, b2, thePalette);
	} else {
		r2 = r;
		g2 = g;
		b2 = b;
	}

	if (dither24bitImages)
	{
		addDither (r2, ditherArray[x&3][y&3]);
		addDither (g2, ditherArray[x&3][y&3] / 2);
		addDither (b2, ditherArray[x&3][y&3]);
	}

	return makeColour (r2, g2, b2);
}

char * readTGAHeader (TGAHeader & h, FILE * fp, palCol thePalette[]) {

	h.IDBlockSize = fgetc (fp);
	h.gotMap = fgetc (fp);
	byte imageType = fgetc (fp);
	h.firstPalColour = get2bytesReverse (fp);
	h.numPalColours = get2bytesReverse (fp);
	h.bitsPerPalColour = fgetc (fp);
	h.xOrigin = get2bytesReverse (fp);
	h.yOrigin = get2bytesReverse (fp);
	h.width = get2bytesReverse (fp);
	h.height = get2bytesReverse (fp);
	h.pixelDepth = fgetc (fp);
	h.imageDescriptor = fgetc (fp);
	countDown = 0;
	// Who cares about the ID block?
	fseek (fp, h.IDBlockSize, 1);

//	fprintf (debugFile, "\nNew file: %d by %d, %d bits per pixel, palette? %d\n\n", h.width, h.height, h.pixelDepth, h.gotMap);
	
	switch (imageType) {
		case 1:
		case 2:
		h.compressed = FALSE;
		break;
		
		case 9:
		case 10:
		h.compressed = TRUE;
		break;
		
		default:
		return "Unsupported internal image format... are you sure this is a valid TGA image file?";
	}

	if (h.pixelDepth != 8 && h.pixelDepth != 16 && h.pixelDepth != 24 && h.pixelDepth != 32) {
		return "Colour depth is not 8, 16, 24 or 32 bits... are you sure this is a valid TGA image file?";
	}

	if (h.gotMap) {
		int c;
		for (c = 0; c < h.numPalColours; c ++) {
			grabRGB (fp, h.bitsPerPalColour, thePalette[c].r, thePalette[c].g, thePalette[c].b, thePalette);
		}
	}

	return NULL;
}

void setDither (int dither)
{
	dither24bitImages = dither;
}

bool getDither ()
{
	return dither24bitImages;
}
