#include <stdio.h>
#include <string.h>

#include <png.h>

#include "moreio.h"
#include "helpers.h"
#include "splitter.hpp"
#include "interface.h"
#include "messbox.h"
#include "tga.h"
#include "compilerinfo.h"

unsigned short int * * backDropImage;
bool backdrop32;
unsigned char * backDrop32Image;
int VERT_RES, HORZ_RES;

bool initBackDropForCompiler (int x, int y) {
	int a;

	HORZ_RES = x;
	VERT_RES = y;

	backDropImage = new unsigned short int * [y];
	if (! backDropImage) return false;
	for (a = 0; a < y; a ++) {
		backDropImage[a] = new unsigned short int [x];
		if (! backDropImage[a]) return false;
	}
	return true;
}

int loadBackDropForCompiler (char * fileName) {
	unsigned short int * toScreen;
	unsigned short int t1, t2;
	palCol thePalette[256];

	// Open the file
	FILE * fp = fopen (fileName, "rb");
	if (fp == NULL) {
		return false;
	}

	// Grab the header
	TGAHeader imageHeader;
	const char * errorBack;
	errorBack = readTGAHeader (imageHeader, fp, thePalette);
	if (errorBack) {
//		alert (errorBack);
		return false;
	}

	if (imageHeader.pixelDepth < 24) {

		initBackDropForCompiler (imageHeader.width, imageHeader.height);

		unsigned short (* readColFunction) (FILE *, int, palCol[], int, int) =
			imageHeader.compressed ? readCompressedColour : readAColour;

		for (t2 = imageHeader.height; t2; t2 --) {
			toScreen = backDropImage[(imageHeader.imageDescriptor & 32) ? (imageHeader.height - t2) : (t2 - 1)];
			for (t1 = 0; t1 < imageHeader.width; t1 ++) {
				* (toScreen ++) = readColFunction (fp, imageHeader.pixelDepth, thePalette, t1, t2);
			}
		}
	} else {
		HORZ_RES = imageHeader.width;
		VERT_RES = imageHeader.height;

		backdrop32 = true;

		backDrop32Image = new unsigned char [imageHeader.height * imageHeader.width * 4];
		if (! backDrop32Image) return false;

		unsigned char r,g,b,a;

		for (int t2 = imageHeader.height-1; t2>=0; t2 --) {
			for (int t1 = 0; t1 < imageHeader.width; t1 ++) {
				if (! imageHeader.compressed) {
					grabRGBA (fp, imageHeader.pixelDepth, r, g, b, a, thePalette);
				} else {
					grabRGBACompressed (fp, imageHeader.pixelDepth, r, g, b, a, thePalette);
				}

				backDrop32Image[t2*imageHeader.width*4+t1*4] = r;
				backDrop32Image[t2*imageHeader.width*4+t1*4+1] = g;
				backDrop32Image[t2*imageHeader.width*4+t1*4+2] = b;
				backDrop32Image[t2*imageHeader.width*4+t1*4+3] = a;
			}
		}
	}

	int i = ftell (fp);
	fclose (fp);
	return i;
}


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

int savePNG (char * filename, int w, int h, unsigned char * data) {
	FILE * fp = fopen (filename, "wb");
	if (! fp) return 0;

	png_structp png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		fclose (fp);
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr){
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		fclose (fp);
		return false;
	}

	png_init_io(png_ptr, fp);

	png_set_IHDR(png_ptr, info_ptr, w, h,
				 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	unsigned char * row_pointers[h];

	for (int i = 0; i < h; i++) {
		row_pointers[i] = data + 4*i*w;
	}

	png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	int i = ftell (fp);
	fclose (fp);
	return i;
}


bool convertTGA (char * filename) {
	char * oldName = joinStrings (filename, "");
	filename[strlen (filename) - 3] = 's';
	filename[strlen (filename) - 2] = 'l';
	filename[strlen (filename) - 1] = getDither() ? '2' : 'x';
	int i, j;

	if (newerFile (oldName, filename)) {
		backdrop32 = false;
		setCompilerText (COMPILER_TXT_ITEM, "Compressing image");
		i = loadBackDropForCompiler (oldName);
		if (! i) return false;
		if (backdrop32) {
			j = savePNG (filename, HORZ_RES, VERT_RES, backDrop32Image);
			delete backDrop32Image;

			if (! j) return false;
		} else {
			j = saveHSI (filename);
			for (int a = 0; a < VERT_RES; a ++) delete backDropImage[a];
			delete backDropImage;

			if (! j) return false;
		}
		setCompilerText (COMPILER_TXT_ITEM, "");
	}
	delete oldName;
	return true;
}
