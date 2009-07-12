#if 0
/*TODO! */

#include "../engine/allfiles.h"

#include "../engine/fileset.h"
#include "sprites.h"
#include "moreio.h"
#include "../engine/newfatal.h"
#include "../engine/colours.h"
#include "../engine/backdrop.h"
#include "../engine/sludger.h"
#include "../engine/zbuffer.h"

extern zBufferData zBuffer;
extern unsigned short int * screen;
extern unsigned short int * * backDropImage;
extern inputType input;
extern int cameraX, cameraY;

unsigned char currentBurnR = 0, currentBurnG = 0, currentBurnB = 0;

void forgetSpriteBank (spriteBank & forgetme) {
	unsigned int index;

	for (index = 0; index < forgetme.total; index ++) {
		delete forgetme.sprites[index].data;
	}

	delete forgetme.sprites;
}

bool reserveSpritePal (spritePalette & sP, int n) {
	sP.pal = new unsigned short int [n];
	sP.total = n;
	return (bool) (sP.pal != NULL);
}

/*bool loadSpriteBank (char * filename, spriteBank & loadhere) {
	unsigned int index, total;
	byte picwidth, picheight, * data;

	FILE * fp;

	fp = fopen (filename, "rb");
//	debug ("Attempting to load sprites (old method) from", filename);
	if (fp == NULL) {
		fatal ("Argh! Can't load sprites", filename);
		return false;
	}

	total = get2bytes (fp);

	loadhere.total = total;
	if (total == 0) {
		fatal ("Argh 2! No sprites in bank!");
		return false;
	}
	loadhere.sprites = new sprite [total];
	checkNew (loadhere.sprites);

	for (index = 0; index < total; index ++) {
		picwidth = (byte) fgetc (fp);
		picheight = (byte) fgetc (fp);

		data = (byte *) new byte [picwidth * picheight];
		checkNew (data);

		loadhere.sprites[index].width = picwidth;
		loadhere.sprites[index].height = picheight;
		loadhere.sprites[index].xhot = fgetc (fp);
		loadhere.sprites[index].yhot = fgetc (fp);
		loadhere.sprites[index].data = data;

		fread (data, picwidth, picheight, fp);
	}

	int howmany = fgetc (fp);
	int startIndex = fgetc (fp);
	byte r, g, b;

	if (! reserveSpritePal (loadhere.myPalette, howmany + startIndex)) return false;

	for (index = 0; index < howmany; index ++) {
		r = (byte) fgetc (fp);
		g = (byte) fgetc (fp);
		b = (byte) fgetc (fp);
		loadhere.myPalette.pal[index + startIndex] = makeColour (r, g, b);
	}
	
	fclose (fp);
	return true;
}
*/
bool loadSpriteBank (int fileNum, spriteBank & loadhere) {
	int index, total, picwidth, picheight, loadSaveMode = 0, howmany,
		startIndex;
	unsigned char r, g, b, * data;

	setResourceForFatal (fileNum);
	if (! openFileFromNum (fileNum)) return fatal ("Can't open sprite bank / font");

	total = get2bytes (bigDataFile);
	if (! total) {
		loadSaveMode = fgetc (bigDataFile);
		if (loadSaveMode == 1) {
			total = 0;
		} else {
			total = get2bytes (bigDataFile);
		}
	}
	
	if (loadSaveMode > 2) return fatal ("Unsupported sprite bank file format");
	if (total <= 0) return fatal ("No sprites in bank or invalid sprite bank file");

	loadhere.total = total;
	loadhere.sprites = new sprite [total];
	if (! checkNew (loadhere.sprites)) return false;

	if (loadSaveMode) {
		howmany = fgetc (bigDataFile);
		startIndex = 1;
	}

	for (index = 0; index < total; index ++) {
		switch (loadSaveMode) {
			case 2:
			picwidth = get2bytes (bigDataFile);
			picheight = get2bytes (bigDataFile);
			loadhere.sprites[index].xhot = getSigned (bigDataFile);
			loadhere.sprites[index].yhot = getSigned (bigDataFile);
			break;
			
			default:
			picwidth = (byte) fgetc (bigDataFile);
			picheight = (byte) fgetc (bigDataFile);
			loadhere.sprites[index].xhot = fgetc (bigDataFile);
			loadhere.sprites[index].yhot = fgetc (bigDataFile);
			break;
		}
		loadhere.sprites[index].width = picwidth;
		loadhere.sprites[index].height = picheight;

		data = (byte *) new byte [picwidth * (picheight + 1)];
		if (! checkNew (data)) return false;
		int ooo = picwidth * picheight;
		for (int tt = 0; tt < picwidth; tt ++) {
			data[ooo ++] = 0;
		}
		loadhere.sprites[index].data = data;
		switch (loadSaveMode) {
			case 2:			// RUN LENGTH COMPRESSED DATA
			{
				unsigned size = picwidth * picheight;
				unsigned pip = 0;
				
				while (pip < size) {
					byte col = fgetc (bigDataFile);
					int looper;
					
					if (col > howmany) {
						col -= howmany + 1;
						looper = fgetc (bigDataFile) + 1;
					} else looper = 1;
					
					while (looper --) {
						data[pip ++] = col;
					}
				}
			}
			break;
			
			default:		// RAW DATA
			fread (data, picwidth, picheight, bigDataFile);
			break;
		}
	}

	if (! loadSaveMode) {
		howmany = fgetc (bigDataFile);
		startIndex = fgetc (bigDataFile);
	}

	if (! reserveSpritePal (loadhere.myPalette, howmany + startIndex)) return false;

	for (index = 0; index < howmany; index ++) {
		r = (byte) fgetc (bigDataFile);
		g = (byte) fgetc (bigDataFile);
		b = (byte) fgetc (bigDataFile);
		loadhere.myPalette.pal[index + startIndex] = makeColour (r, g, b);
	}

	finishAccess ();
	setResourceForFatal (-1);
	return true;
}

void pasteSpriteToBackDrop (int x1, int y1, sprite & single, const spritePalette & fontPal) {
	int xpos, ypos, fromhere = 0, skip = 0, x2, y2;
	unsigned short int s;

	x1 -= single.xhot;
	y1 -= single.yhot;
	x2 = x1 + single.width;
	y2 = y1 + single.height;

	if (x1 < 0) {
		skip -= x1;
		fromhere -= x1;
		x1 = 0;
	} else if (x2 > winWidth) {
		skip += (x2 - winWidth);
		x2 = winWidth;
	}

	if (y1 < 0) {
		fromhere -= (y1 * single.width);
		y1 = 0;
	} else if (y2 > winHeight)
		y2 = winHeight;

	unsigned short int * toHere;

	for (ypos = y1; ypos < y2; ypos ++) {
		toHere = backDropImage[ypos] + x1;
		for (xpos = x1; xpos < x2; xpos ++) {
			s = single.data[fromhere++];
			if (s) (* toHere) = fontPal.pal[s];
			toHere ++;
		}
		fromhere += skip;
	}
}

void burnSpriteToBackDrop (int x1, int y1, sprite & single, const spritePalette & fontPal) {
	int xpos, ypos, fromhere = 0, skip = 0, x2, y2;
	unsigned short int s;

	x1 -= single.xhot;
	y1 -= single.yhot;
	x2 = x1 + single.width;
	y2 = y1 + single.height;

	if (x1 < 0) {
		skip -= x1;
		fromhere -= x1;
		x1 = 0;
	} else if (x2 > winWidth) {
		skip += (x2 - winWidth);
		x2 = winWidth;
	}

	if (y1 < 0) {
		fromhere -= (y1 * single.width);
		y1 = 0;
	} else if (y2 > winHeight)
		y2 = winHeight;

	unsigned short int * toHere;

	for (ypos = y1; ypos < y2; ypos ++) {
		toHere = backDropImage[ypos] + x1;
		for (xpos = x1; xpos < x2; xpos ++) {
			s = single.data[fromhere ++];
			if (s) {
				double mlev = (s - 1) / ((double) fontPal.total - 1);
				double lev = 1 - mlev;
				
				* toHere = makeColour (redValue   (* toHere) * lev + currentBurnR * mlev,
									   greenValue (* toHere) * lev + currentBurnG * mlev,
									   blueValue  (* toHere) * lev + currentBurnB * mlev);
				// fontPal.pal[s];
			}
			toHere ++;
		}
		fromhere += skip;
	}
}

void fontSprite (int x1, int y1, sprite & single, const spritePalette & fontPal) {
	int xpos, ypos, fromhere = 0, skip = 0, x2, y2;
	unsigned short int s;

	x1 -= single.xhot;
	y1 -= single.yhot;
	x2 = x1 + single.width;
	y2 = y1 + single.height;

	if (x1 < 0) {
		skip -= x1;
		fromhere -= x1;
		x1 = 0;
	} else if (x2 > winWidth) {
		skip += (x2 - winWidth);
		x2 = winWidth;
	}

	if (y1 < 0) {
		fromhere -= (y1 * single.width);
		y1 = 0;
	} else if (y2 > winHeight)
		y2 = winHeight;

	unsigned short int * toHere;

	for (ypos = y1; ypos < y2; ypos ++) {
		toHere = screen + winWidth * ypos + x1;
		for (xpos = x1; xpos < x2; xpos ++) {
			s = single.data[fromhere++];
			if (s) (* toHere) = fontPal.pal[s];
			toHere ++;
		}
		fromhere += skip;
	}
}

void flipFontSprite (int x1, int y1, sprite & single, const spritePalette & fontPal) {
	int xpos, ypos, fromhere = 0, skip = 0, x2, y2;
	unsigned short int s;

	x1 -= single.width - (1 + single.xhot);
	y1 -= single.yhot;
	x2 = x1 + single.width;
	y2 = y1 + single.height;

	if (x1 < 0) {
		skip -= x1;
		x1 = 0;
	} else if (x2 > winWidth) {
		skip += (x2 - winWidth);
		fromhere += (x2 - winWidth);
		x2 = winWidth;
	}

	if (y1 < 0) {
		fromhere -= (y1 * single.width);
		y1 = 0;
	} else if (y2 > winHeight)
		y2 = winHeight;

	unsigned short int * toHere;

	for (ypos = y1; ypos < y2; ypos ++) {
		toHere = screen + winWidth * ypos + x2;
		for (xpos = x1; xpos < x2; xpos ++) {
			toHere --;
			s = single.data[fromhere++];
			if (s) (* toHere) = fontPal.pal[s];
		}
		fromhere += skip;
	}
}

void drawModeTransparent1 (unsigned short int * scrCol, const unsigned short int & sprCol) {
	(* scrCol) = (unsigned short int) ((((* scrCol) >> 1) & 31727) + ((sprCol >> 2) & 31207) + (((* scrCol) >> 2) & 31207));
}

void drawModeTransparent2 (unsigned short int * scrCol, const unsigned short int & sprCol) {
	(* scrCol) = (unsigned short int) ((((* scrCol) >> 1) & 31727) + ((sprCol >> 1) & 31727));
}

void drawModeTransparent3 (unsigned short int * scrCol, const unsigned short int & sprCol) {
	(* scrCol) = (unsigned short int) (((sprCol >> 1) & 31727) + ((sprCol >> 2) & 31207) + (((* scrCol) >> 2) & 31207));
}

void drawModeDark3 (unsigned short int * scrCol, const unsigned short int & sprCol) {
	(* scrCol) = (unsigned short int) (((sprCol) >> 2) & 31207u);
}

void drawModeDark2 (unsigned short int * scrCol, const unsigned short int & sprCol) {
	(* scrCol) = (unsigned short int) (((sprCol) >> 1) & 31727u);
}

void drawModeDark1 (unsigned short int * scrCol, const unsigned short int & sprCol) {
	(* scrCol) = (unsigned short int) ((((sprCol) >> 1) & 31727u) + (((sprCol) >> 2) & 31207u));
}

#define g255 65535u
#define g191 48631u
#define g127 31727u
#define g95 23275u
#define g63 14823u
#define g31 6371u

void drawModeFoggy4 (unsigned short int * scrCol, const unsigned short int & sprCol) {
	#pragma unused (sprCol)
	(* scrCol) = g127;
}

void drawModeFoggy3 (unsigned short int * scrCol, const unsigned short int & sprCol) {
	(* scrCol) = g95 + (unsigned short int) (((sprCol) >> 2) & 31207u);
}

void drawModeFoggy2 (unsigned short int * scrCol, const unsigned short int & sprCol) {
	(* scrCol) = g63 + (unsigned short int) (((sprCol) >> 1) & 31727u);
}

void drawModeFoggy1 (unsigned short int * scrCol, const unsigned short int & sprCol) {
	(* scrCol) = g31 + (unsigned short int) ((((sprCol) >> 1) & 31727u) + (((sprCol) >> 2) & 31207u));
}

void drawModeGlow4 (unsigned short int * scrCol, const unsigned short int & sprCol) {
	#pragma unused (sprCol)
	(* scrCol) = g255;
}

void drawModeGlow3 (unsigned short int * scrCol, const unsigned short int & sprCol) {
	(* scrCol) = g191 + (unsigned short int) (((sprCol) >> 2) & 31207u);
}

void drawModeGlow2 (unsigned short int * scrCol, const unsigned short int & sprCol) {
	(* scrCol) = g127 + (unsigned short int) (((sprCol) >> 1) & 31727u);
}

void drawModeGlow1 (unsigned short int * scrCol, const unsigned short int & sprCol) {
	(* scrCol) = g63 + (unsigned short int) ((((sprCol) >> 1) & 31727u) + (((sprCol) >> 2) & 31207u));
}

void drawModeBlack (unsigned short int * scrCol, const unsigned short int & sprCol) {
	#pragma unused (sprCol)
	(* scrCol) = 0;
}

void drawModeShadow3 (unsigned short int * scrCol, const unsigned short int & sprCol) {
	#pragma unused (sprCol)
	(* scrCol) = (unsigned short int) (((* scrCol) >> 2) & 31207u);
}

void drawModeShadow2 (unsigned short int * scrCol, const unsigned short int & sprCol) {
	#pragma unused (sprCol)
	(* scrCol) = (unsigned short int) (((* scrCol) >> 1) & 31727u);
}

void drawModeShadow1 (unsigned short int * scrCol, const unsigned short int & sprCol) {
	#pragma unused (sprCol)
	(* scrCol) = (unsigned short int) ((((* scrCol) >> 1) & 31727u) + (((* scrCol) >> 2) & 31207u));
}

void drawModeNormal (unsigned short int * scrCol, const unsigned short int & sprCol) {
	(* scrCol) = sprCol;
}

void drawModeInvisible (unsigned short int * scrCol, const unsigned short int & sprCol) {
	#pragma unused (sprCol, scrCol)
}

typedef void (* pixelDrawFunc)(unsigned short int *, const unsigned short int &);

#define numDrawModes 19

pixelDrawFunc dmf[] = {
	drawModeNormal,
	drawModeTransparent1,
	drawModeTransparent2,
	drawModeTransparent3,
	drawModeDark1,
	drawModeDark2,
	drawModeDark3,
	drawModeBlack,
	drawModeShadow1,
	drawModeShadow2,
	drawModeShadow3,
	drawModeFoggy1,
	drawModeFoggy2,
	drawModeFoggy3,
	drawModeFoggy4,
	drawModeGlow1,
	drawModeGlow2,
	drawModeGlow3,
	drawModeGlow4,
	drawModeInvisible
};

inline unsigned short multCols (unsigned short col1, unsigned short col2) {
	return makeColour (redValue   (col1) * redValue   (col2) >> 8,
					   greenValue (col1) * greenValue (col2) >> 8,
					   blueValue  (col1) * blueValue  (col2) >> 8);
}

bool getScaleData (int * & scaleDataX, int * & scaleDataY, int diffX, int diffY, float scale, bool mirror) {
	int xpos, biggest = ((diffX > diffY) ? diffX : diffY) + 1;
	scaleDataY = new int[biggest + 1];
	if (! checkNew (scaleDataY)) return false;

	for (xpos = 0; xpos <= biggest; xpos ++) {
		scaleDataY[xpos] = (xpos + 0.5) / (scale);
	}
	
	if (mirror) {
		scaleDataX = new int[diffX + 1];
		if (! checkNew (scaleDataX)) return false;
		for (xpos = 0; xpos <= diffX; xpos ++) {
			scaleDataX[diffX - xpos] = (xpos - 0.5) / (scale);
		}
	} else {
		scaleDataX = scaleDataY;
	}
	return true;
}

bool scaleSprite (int x1, int y1, sprite & single, const spritePalette & fontPal, float scale, unsigned int drawMode, int floaty, bool useZB, unsigned short int * * lightMapImage, bool mirror) {
	if (scale <= 0.05) return false;

	int xpos, ypos, x2, y2, imX1 = 0, imY1 = 0, imX, imY, diffX, diffY;
	unsigned short int s;

	if (drawMode >= numDrawModes) drawMode = 0;
	pixelDrawFunc drawModeFunction = dmf[drawMode];

	diffX = single.width * scale + 0.5;
	diffY = single.height * scale + 0.5;
	int originalY = y1 + cameraY;
	x1 -= (mirror ? (single.width - (single.xhot + 1)) : single.xhot) * scale;
	y1 -= (single.yhot - floaty) * scale;
	x2 = x1 + diffX;
	y2 = y1 + diffY;
//	debug ("Biggest difference", biggest);

	int * scaleDataX;
	int * scaleDataY;
	if (! getScaleData (scaleDataX, scaleDataY, diffX, diffY, scale, mirror)) return false;
	
	if (x1 < 0) {
		imX1 = - x1;
		x1 = 0;
	} else if (x2 > winWidth) {
		x2 = winWidth;
	}

	if (y1 < 0) {
		imY1 = - y1;
		y1 = 0;
	} else if (y2 > winHeight) {
		y2 = winHeight;
	}

	unsigned short int * toHere;
	unsigned short int * lmi;
	unsigned long startOfLine;
	bool exactHit = false;

	imY = imY1;
	if (zBuffer.map && useZB) {
		unsigned short int * mapHere;
		for (ypos = y1; ypos < y2; ypos ++) {
			imX = imX1;
			toHere = screen + winWidth * ypos + x1;
			mapHere = zBuffer.map[ypos + cameraY] + x1 + cameraX;
			if (lightMapImage) lmi = lightMapImage[ypos] + x1 + cameraX;
			
			startOfLine = scaleDataY[imY] * single.width;
			for (xpos = x1; xpos < x2; xpos ++) {
				s = single.data[scaleDataX[imX] + startOfLine];
				if (lightMapImage) {
					if (s && originalY > * mapHere) {
						drawModeFunction (toHere, multCols (fontPal.pal[s], * lmi));
						exactHit |= (input.mouseX == xpos && input.mouseY == ypos);
					}
					lmi ++;
				} else {
					if (s && originalY > * mapHere) {
						drawModeFunction (toHere, fontPal.pal[s]);
						exactHit |= (input.mouseX == xpos && input.mouseY == ypos);
					}
				}
				mapHere ++;
				toHere ++;
				imX ++;
			}
			imY ++;
		}
	} else {
		for (ypos = y1; ypos < y2; ypos ++) {
			imX = imX1;
			toHere = screen + winWidth * ypos + x1;
			startOfLine = scaleDataY[imY] * single.width;
			if (lightMapImage) lmi = lightMapImage[ypos] + x1 + cameraX;
			
			for (xpos = x1; xpos < x2; xpos ++) {
				s = single.data[scaleDataX[imX] + startOfLine];
				if (lightMapImage) {
					if (s) {
						drawModeFunction (toHere, multCols (fontPal.pal[s], * lmi));
						exactHit |= (input.mouseX == xpos && input.mouseY == ypos);
					}
					lmi ++;
				} else {
					if (s) {
						drawModeFunction (toHere, fontPal.pal[s]);
						exactHit |= (input.mouseX == xpos && input.mouseY == ypos);
					}
				}
				toHere ++;
				imX ++;
			}
			imY ++;
		}
	}
	
	// Free up the memory we used for our tables...
	
	delete scaleDataY;
	if (mirror) delete scaleDataX;
	
	// Are we pointing at the sprite?
	
	return exactHit;//(bool) (input.mouseX >= x1 && input.mouseX <= x2 && input.mouseY >= y1 && input.mouseY <= y2);
}

void fixScaleSprite (int x1, int y1, sprite & single, const spritePalette & fontPal, float scale, unsigned int drawMode, int floaty, bool useZB, unsigned short int * * lightMapImage, const int camX, const int camY, bool mirror) {
	if (scale <= 0.05) return;
	
	int xpos, ypos, x2, y2, imX1 = 0, imY1 = 0, imX, imY, diffX, diffY;
	unsigned short int s;

	if (drawMode >= numDrawModes) drawMode = 0;
	pixelDrawFunc drawModeFunction = dmf[drawMode];

	diffX = single.width * scale + 0.5;
	diffY = single.height * scale + 0.5;
	int originalY = y1;
	x1 -= (mirror ? (single.width - (single.xhot + 1)) : single.xhot) * scale;
	y1 -= (single.yhot - floaty) * scale;
	x2 = x1 + diffX;
	y2 = y1 + diffY;

	int * scaleDataX;
	int * scaleDataY;
	if (! getScaleData (scaleDataX, scaleDataY, diffX, diffY, scale, mirror)) return;
	
	if (x1 < 0) {
		imX1 = - x1;
		x1 = 0;
	} else if (x2 > winWidth) {
		x2 = winWidth;
	}

	if (y1 < 0) {
		imY1 = - y1;
		y1 = 0;
	} else if (y2 > winHeight) {
		y2 = winHeight;
	}

	unsigned short int * toHere, * lmi = NULL;
	unsigned long startOfLine;

	imY = imY1;
	if (zBuffer.map && useZB) {
		unsigned short int * mapHere;
		for (ypos = y1; ypos < y2; ypos ++) {
			imX = imX1;
			toHere = backDropImage[ypos] + x1;
			mapHere = zBuffer.map[ypos + camY] + x1 + camX;
			if (lightMapImage) lmi = lightMapImage[ypos + camY] + x1 + camX;
			
			startOfLine = scaleDataY[imY] * single.width;
			for (xpos = x1; xpos < x2; xpos ++) {
				s = single.data[scaleDataX[imX] + startOfLine];
				if (lightMapImage) {
					if (s && originalY > * mapHere)	drawModeFunction (toHere, multCols (fontPal.pal[s], * lmi));
					lmi ++;
				} else {
					if (s && originalY > * mapHere) drawModeFunction (toHere, fontPal.pal[s]);
				}
				toHere ++;
				mapHere ++;
				imX ++;
			}
			imY ++;
		}
	} else {
		for (ypos = y1; ypos < y2; ypos ++) {
			imX = imX1;
			toHere = backDropImage[ypos] + x1;
			if (lightMapImage) lmi = lightMapImage[ypos + camY] + x1 + camX;
			startOfLine = scaleDataY[imY] * single.width;
			
			for (xpos = x1; xpos < x2; xpos ++) {
				s = single.data[scaleDataX[imX] + startOfLine];
				if (lightMapImage) {
					if (s) drawModeFunction (toHere, multCols (fontPal.pal[s], * lmi));
					lmi ++;
				} else {
					if (s) drawModeFunction (toHere, fontPal.pal[s]);
				}
				toHere ++;
				imX ++;
			}
			imY ++;
		}
	}
	delete scaleDataY;
	if (mirror) delete scaleDataX;
}

struct myBitmapInfo {
	BITMAPINFOHEADER bmiHeader;
	DWORD bitMask[3];
};

myBitmapInfo screenBMPInfo;   // bitmap information/palette
HDC bufferDC;
unsigned short int * screen;
HBITMAP videoBuffer;          // handle to the buffer bitmap
extern HWND mainWin;

bool initSpriteArea () {
	int i;
	HDC hdc;

	// Fill in bitmap header information

	screenBMPInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	screenBMPInfo.bmiHeader.biWidth=SPRITE_AREA_W;
	screenBMPInfo.bmiHeader.biHeight=-SPRITE_AREA_H;
	screenBMPInfo.bmiHeader.biPlanes=1;
	screenBMPInfo.bmiHeader.biBitCount=16;
	screenBMPInfo.bmiHeader.biCompression=BI_BITFIELDS;
	screenBMPInfo.bmiHeader.biSizeImage=0;
	screenBMPInfo.bmiHeader.biXPelsPerMeter=0;
	screenBMPInfo.bmiHeader.biYPelsPerMeter=0;
	screenBMPInfo.bmiHeader.biClrUsed=256;
	screenBMPInfo.bmiHeader.biClrImportant=0;

	screenBMPInfo.bitMask[0]=0xF800;
	screenBMPInfo.bitMask[1]=0x07E0;
	screenBMPInfo.bitMask[2]=0x001F;

	hdc = GetDC(mainWin);

	// Attempt to create a memory DC for the buffer
	bufferDC=CreateCompatibleDC(hdc);

	if(bufferDC==NULL) return false;

	// Get pointer and handle to buffer DIB
	screen=NULL;
	videoBuffer=CreateDIBSection(hdc,(BITMAPINFO *)&screenBMPInfo,
										  DIB_RGB_COLORS,(void **)&screen,NULL,0);

	// Select the buffer DIB into the memory device context
	i=(int)SelectObject(bufferDC,videoBuffer);

	if((!i) || (i==GDI_ERROR)) {
//		MessageBox(main,"Could not select buffer DIB into device context",
//					  "initVideo(...) error",MB_ICONSTOP | MB_OK);
		return(false);
	};

	ReleaseDC(mainWin, hdc);

	return(true);
}

void paintSpriteArea () {
	HDC hdc = GetDC (mainWin);
	BitBlt (hdc, SPRITE_AREA_X, SPRITE_AREA_Y, SPRITE_AREA_W, SPRITE_AREA_H, bufferDC, 0, 0, SRCCOPY);
	ReleaseDC (mainWin, hdc);
}

#endif