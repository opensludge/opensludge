#include <OpenGL/gl.h>
#include <stdlib.h>
#include <string.h>

#include "TGA.h"
#include "Sprites.h"
#include "MoreIO.h"
#include "MessBox.h"

void forgetSpriteBank (spriteBank & forgetme) {
	unsigned int index;
	
	for (index = 0; index < forgetme.total; index ++) {
		delete forgetme.sprites[index].data;
		glDeleteTextures (1, &forgetme.myPalette.tex_names[index]);
	}
	delete forgetme.sprites;
}

bool reserveSpritePal (spritePalette * sP, int n) {
	if (! sP->pal) {
		sP->pal = new unsigned short int [256];
		sP->r = new unsigned char [256];
		sP->g = new unsigned char [256];
		sP->b = new unsigned char [256];
	}
	sP->total = n;
	return (bool) (sP->pal != NULL) && (sP->r != NULL) && (sP->g != NULL) && (sP->b != NULL);
}




bool saveSpriteBank (const char * filename, spriteBank *sprites) {
	int i;
	
	FILE * fp = fopen (filename, "wb");
	if (! fp) {
		return false;
	}
	
	if (sprites->type < 2) {
		// This is version 2 of the DUC format
		put2bytes(0, fp);
		fputc (2, fp);
	} else {
		// This is version 3 of the DUC format - in 32-bit glory!
		put2bytes(0, fp);
		fputc (3, fp);		
		errorBox ("Saving error", "32-bit format not yet fully supported. Sorry.");
		fclose(fp);
		return false;
	}
	
	put2bytes (sprites->total, fp);
	
	// Number of colours used, except 0 because that gets added (or ignored) by the engine.
	fputc (sprites->myPalette.total-1, fp); 
	
	for (i = 0; i < sprites->total; i ++) {
		put2bytes (sprites->sprites[i].width, fp);
		put2bytes (sprites->sprites[i].height, fp);
		putSigned (sprites->sprites[i].xhot, fp);
		putSigned (sprites->sprites[i].yhot, fp);
		
		// Run Length compressed data go here
		unsigned char * data = sprites->sprites[i].data;
		unsigned char * lookPointer;
		unsigned int size = sprites->sprites[i].width * sprites->sprites[i].height;

		int lookAhead;
		int x = 0;
		while (x < size) {
			lookPointer = data + 1;
			for (lookAhead = x+1; lookAhead < size; lookAhead++) {
				if (*data + sprites->myPalette.total > 255) break;
				if (lookAhead-x > 255) break;
				if (*data != * lookPointer) break;
				lookPointer++;
			}
			if (lookAhead == x+1) {
				fputc ((* data), fp);
			} else {
				fputc (*data + sprites->myPalette.total, fp);
				fputc (lookAhead - x - 1, fp);
			}
			data = lookPointer;
			x = lookAhead;
		}
	}
				
	for (i = 1; i < sprites->myPalette.total; i ++) {
		fputc (sprites->myPalette.r[i], fp);
		fputc (sprites->myPalette.g[i], fp);
		fputc (sprites->myPalette.b[i], fp);
	}
	
	fclose (fp);
	return true;
}


unsigned int get2bytes (unsigned char ** fp) {
	int r = (**fp) * 256 + *(*fp+1);
	*fp+=2;
	return r;
}
short readSigned (unsigned char **fp) {
	short f = **((short **) fp);
	*fp += sizeof(short);
	return f;
}

bool loadSpriteBank (unsigned char * file, spriteBank *loadhere) {
	int i, total, picwidth, picheight, loadSaveMode = 0, howmany,
		startIndex;
	int totalwidth[256], maxheight[256];
	int numTextures = 0;
		
	total = get2bytes(&file);
	if (! total) {
		loadSaveMode = *file++;
		if (loadSaveMode == 1) {
			total = 0;
		} else {
			total = get2bytes(&file);
		}
	}
		
	if (loadSaveMode > 2) return false;//fatal ("Unsupported sprite bank file format");
		if (total <= 0) return false;//fatal ("No sprites in bank or invalid sprite bank file");

	loadhere->type = 0;
			
	loadhere->total = total;
	loadhere->sprites = new sprite [total];
	//if (! checkNew (loadhere->sprites)) return false;

	if (loadSaveMode) {
		howmany = *file++;
		startIndex = 1;
	}
	
	totalwidth[0] = maxheight[0] = 0;

	for (i = 0; i < total; i ++) {
		switch (loadSaveMode) {
			case 2:
			picwidth = get2bytes((unsigned char**) &file);
			picheight = get2bytes((unsigned char**) &file);
			loadhere->sprites[i].xhot = readSigned (&file);
			loadhere->sprites[i].yhot = readSigned (&file);
			break;
			
			default:
			picwidth = (unsigned char) *file++;
			picheight = (unsigned char) *file++;
			loadhere->sprites[i].xhot = *file++;
			loadhere->sprites[i].yhot = *file++;
			break;
		}
		if (totalwidth[numTextures] + picwidth < 2048) {
			loadhere->sprites[i].tex_x = totalwidth[numTextures];
			totalwidth[numTextures] += (loadhere->sprites[i].width = picwidth);
			if ((loadhere->sprites[i].height = picheight) > maxheight[numTextures]) maxheight[numTextures] = picheight;
		} else {
			numTextures++;
			if (numTextures > 255) return false;//fatal ("Can't open sprite bank / font - it's too big.");
			loadhere->sprites[i].tex_x = 0;
			totalwidth[numTextures] = (loadhere->sprites[i].width = picwidth);
			maxheight[numTextures] = loadhere->sprites[i].height = picheight;
		}
		loadhere->sprites[i].texNum = numTextures;

		loadhere->sprites[i].data = (unsigned char *) new unsigned char [picwidth * (picheight + 1)];
		//if (! checkNew (data)) return false;
		int ooo = picwidth * picheight;
		for (int tt = 0; tt < picwidth; tt ++) {
			loadhere->sprites[i].data[ooo ++] = 0;
		}

		switch (loadSaveMode) {
			case 2:			// RUN LENGTH COMPRESSED DATA
			{
				unsigned size = picwidth * picheight;
				unsigned pip = 0;
				
				while (pip < size) {
					unsigned char col = *file++;
					int looper;
					
					if (col > howmany) {
						col -= howmany + 1;
						looper = (*file++) + 1;
					} else looper = 1;
					
					while (looper --) {
						loadhere->sprites[i].data[pip ++] = col;
					}
				}
			}
			break;
			
			default:		// RAW DATA
				memcpy (loadhere->sprites[i].data, file, picwidth * picheight);
				file += picwidth * picheight;
			break;
		}
	}
	numTextures++;

	if (! loadSaveMode) {
		howmany = *file++;
		startIndex = *file++;
	}

	if (! reserveSpritePal (&loadhere->myPalette, howmany + startIndex)) return false;

	for (i = 0; i < howmany; i ++) {
		loadhere->myPalette.r[i + startIndex] = (unsigned char) *file++;
		loadhere->myPalette.g[i + startIndex] = (unsigned char) *file++;
		loadhere->myPalette.b[i + startIndex] = (unsigned char) *file++;
		loadhere->myPalette.pal[i + startIndex] = makeColour (loadhere->myPalette.r[i + startIndex], loadhere->myPalette.g[i + startIndex], loadhere->myPalette.b[i + startIndex]);
	}
			
	return true;
}

bool loadSpriteTextures (spriteBank *loadhere) {
	int i;
	int fromhere;
	unsigned char s;

	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
	glGenTextures (loadhere->total, loadhere->myPalette.tex_names);	
	
	GLubyte * tmp[loadhere->total];		
	
	for (i = 0; i < loadhere->total; i ++) {
		tmp[i] = new GLubyte [loadhere->sprites[i].width*loadhere->sprites[i].height*4];
		fromhere = 0;
		for (int y = 0; y < loadhere->sprites[i].height; y ++) {
			for (int x = 0; x < loadhere->sprites[i].width; x ++) {
				GLubyte * target = tmp[i] + 4*loadhere->sprites[i].width*y + x*4;
				s = loadhere->sprites[i].data[fromhere++];
				if (s) {
					target[0] = (GLubyte) loadhere->myPalette.r[s];
					target[1] = (GLubyte) loadhere->myPalette.g[s];
					target[2] = (GLubyte) loadhere->myPalette.b[s];
					target[3] = (GLubyte) 255;
				} else {
					target[0] = (GLubyte) 0;
					target[1] = (GLubyte) 0;
					target[2] = (GLubyte) 0;
					target[3] = (GLubyte) 0;
				}
			}
		}
		
		loadhere->sprites[i].texNum = i;
	
		glBindTexture (GL_TEXTURE_2D, loadhere->myPalette.tex_names[i]);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, loadhere->sprites[i].width, loadhere->sprites[i].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp[i]);
		
		delete tmp[i];		
	}
	return true;
}

bool loadSpriteTexture (spriteBank *loadhere, int index) {
	int fromhere;
	unsigned char s;
	
	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);	
	GLubyte tmp[loadhere->sprites[index].width*loadhere->sprites[index].height*4];		
	
	if (loadhere->type < 2) {
		fromhere = 0;
		for (int y = 0; y < loadhere->sprites[index].height; y ++) {
			for (int x = 0; x < loadhere->sprites[index].width; x ++) {
				GLubyte * target = tmp + 4*loadhere->sprites[index].width*y + x*4;
				s = loadhere->sprites[index].data[fromhere++];
				if (s) {
					target[0] = (GLubyte) loadhere->myPalette.r[s];
					target[1] = (GLubyte) loadhere->myPalette.g[s];
					target[2] = (GLubyte) loadhere->myPalette.b[s];
					target[3] = (GLubyte) 255;
				} else {
					target[0] = (GLubyte) 0;
					target[1] = (GLubyte) 0;
					target[2] = (GLubyte) 0;
					target[3] = (GLubyte) 0;
				}
			}
		}
		
		glBindTexture (GL_TEXTURE_2D, loadhere->myPalette.tex_names[loadhere->sprites[index].texNum]);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, loadhere->sprites[index].width, loadhere->sprites[index].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
	} else {
		glBindTexture (GL_TEXTURE_2D, loadhere->myPalette.tex_names[loadhere->sprites[index].texNum]);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, loadhere->sprites[index].width, loadhere->sprites[index].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, loadhere->sprites[index].data);
		
	}
		
	return true;
}

void addSprite (int i, struct spriteBank *sprites) {
	sprite * newsprites = new sprite [sprites->total+1];
	
	int i1 = sprites->total-1, i2 = sprites->total;
	if (i == sprites->total) {
		newsprites [i2].width = 0;
		newsprites [i2].height = 0;
		newsprites [i2].xhot = 0;
		newsprites [i2].yhot = 0;
		newsprites [i2].tex_x = 0;
		newsprites [i2].texNum = i2;
		newsprites [i2].data = NULL;
		sprites->myPalette.tex_names[i2] = 0;
		glGenTextures (1, &sprites->myPalette.tex_names[i2]);
		i2--;
	}
	while (i1 >= 0 ) {
		newsprites [i2].width = sprites->sprites[i1].width;
		newsprites [i2].height = sprites->sprites[i1].height;
		newsprites [i2].xhot = sprites->sprites[i1].xhot;
		newsprites [i2].yhot = sprites->sprites[i1].yhot;
		newsprites [i2].tex_x = sprites->sprites[i1].tex_x;
		newsprites [i2].texNum = i2;
		newsprites [i2].data = sprites->sprites[i1].data;
		sprites->myPalette.tex_names[i2] = sprites->myPalette.tex_names[i1];
		i2--;
		if (i1 == i) {
			newsprites [i2].width = 0;
			newsprites [i2].height = 0;
			newsprites [i2].xhot = 0;
			newsprites [i2].yhot = 0;
			newsprites [i2].tex_x = 0;
			newsprites [i2].texNum = i2;
			newsprites [i2].data = NULL;
			sprites->myPalette.tex_names[i2] = 0;
			glGenTextures (1, &sprites->myPalette.tex_names[i2]);
			i2--;
		}
		i1--;
	}
	delete sprites->sprites;
	sprites->sprites = newsprites;
	sprites->total++;
}


void deleteSprite (int i, struct spriteBank *sprites) {
	if (! sprites->total) return;
	if (i >= sprites->total) return;
	
	sprite * newsprites = new sprite [sprites->total-1];
	
	int i1 = 0, i2 = 0;
	while (i1 < sprites->total) {
		if (i1 != i) {
			newsprites [i2].width = sprites->sprites[i1].width;
			newsprites [i2].height = sprites->sprites[i1].height;
			newsprites [i2].xhot = sprites->sprites[i1].xhot;
			newsprites [i2].yhot = sprites->sprites[i1].yhot;
			newsprites [i2].tex_x = sprites->sprites[i1].tex_x;
			newsprites [i2].texNum = i2;//sprites->sprites[i1].texNum;
			newsprites [i2].data = sprites->sprites[i1].data;
			sprites->myPalette.tex_names[i2] = sprites->myPalette.tex_names[i1];
			i2++;
		} else {
			if (sprites->sprites[i1].data) {
				delete sprites->sprites[i1].data;
				glDeleteTextures(1, (const GLuint*) &sprites->myPalette.tex_names[i1]);
			}
		}
		i1++;
	}
	delete sprites->sprites;
	sprites->sprites = newsprites;
	sprites->total--;
}

unsigned char findClosestPal (spritePalette *searchIn, unsigned char r, unsigned char g, unsigned char b) {
	if (r == 255 && g == 0 && b == 255) return 0;
	
	unsigned char ret = 0;
	int diff = 200000, diff1;
	for (int i=1; i < searchIn->total; i++) {
		diff1 = (searchIn->r[i]-r)*(searchIn->r[i]-r) + (searchIn->g[i]-g)*(searchIn->g[i]-g) + (searchIn->b[i]-b)*(searchIn->b[i]-b);
		if (! diff1) return i;
		if (diff1 < diff) {
			ret = i;
			diff = diff1;
		}
	}
	return ret;
}

int findOrAddPal (spritePalette *searchIn, unsigned char r, unsigned char g, unsigned char b) {
	if (r == 255 && g == 0 && b == 255) return 0;
	
	int i;
	for (i=1; i < searchIn->total; i++) {
		if (!(searchIn->r[i]-r) && !(searchIn->g[i]-g) && !(searchIn->b[i]-b)) return i;
	}
	if (i < 255) {
		searchIn->total++;
		searchIn->r[i]=r;
		searchIn->g[i]=g;
		searchIn->b[i]=b;
		return i;
	}
	return -1;
}


bool loadSpriteFromTGA (char * file, struct spriteBank *sprites, int index)
{
	palCol thePalette[256];

	// Open the file	
	FILE * fp = fopen (file, "rb");
	if (fp == NULL) {
		errorBox ("Can't open TGA file", "The file can't be opened. I don't know why.");
		return false;
	}
		
	// Grab the header
	TGAHeader imageHeader;
	char * errorBack;
	errorBack = readTGAHeader (imageHeader, fp, thePalette);
	if (errorBack) {
		fclose (fp);
		errorBox ("Can't open TGA file", errorBack);
		return false;		
	}
	fprintf (stderr, "Compressed: %d\n", imageHeader.compressed);
	fprintf (stderr, "Pixel depth: %d\n", imageHeader.pixelDepth);
	fprintf (stderr, "Palette: %d\n", imageHeader.numPalColours);
	
	unsigned char *data;
		
	if (sprites->type < 2) {
		
		data = new unsigned char[imageHeader.height * imageHeader.width];
		if (! data) {
			fclose (fp);
			errorBox ("Can't create sprite", "Out of memory.");
			return false;		
		}
		
		unsigned char palSize = sprites->myPalette.total;
		unsigned char r,g,b;
		int c;
			
		for (int t2 = imageHeader.height; t2; t2 --) {
			for (int t1 = 0; t1 < imageHeader.width; t1 ++) {
				if (! imageHeader.compressed) {
					grabRGB (fp, imageHeader.pixelDepth, r, g, b, thePalette);
				} else {
					grabRGBCompressed (fp, imageHeader.pixelDepth, r, g, b, thePalette);
				}
				if (sprites->type == 1)
					data[t2*imageHeader.width+t1] = findClosestPal (&sprites->myPalette, r, g, b);
				else {
					if ((c = findOrAddPal (&sprites->myPalette, r, g, b)) < 0) {
						fclose(fp);
						errorBox ("Can't create sprite", "The sprite bank palette doesn't have enough room left to add the colours. Change the palette mode to locked or 32-bit and try again.");
						sprites->myPalette.total = palSize;
						delete data;
						return false;
					}
					data[t2*imageHeader.width+t1] = c;
				}
			}
		}
	} else {
		data = new unsigned char[imageHeader.height * imageHeader.width * 4];
		if (! data) {
			fclose (fp);
			errorBox ("Can't create sprite", "Out of memory.");
			return false;		
		}
		
		unsigned char r,g,b,a;
		
		for (int t2 = imageHeader.height; t2; t2 --) {
			for (int t1 = 0; t1 < imageHeader.width; t1 ++) {
				if (! imageHeader.compressed) {
					grabRGBA (fp, imageHeader.pixelDepth, r, g, b, a, thePalette);
				} else {
					grabRGBACompressed (fp, imageHeader.pixelDepth, r, g, b, a, thePalette);
				}

				data[t2*imageHeader.width*4+t1*4] = r;
				data[t2*imageHeader.width*4+t1*4+1] = g;
				data[t2*imageHeader.width*4+t1*4+2] = b;
				data[t2*imageHeader.width*4+t1*4+3] = a;
			}
		}
	}
	fclose (fp);
	if (sprites->sprites[index].data) delete sprites->sprites[index].data;

	sprites->sprites[index].data = data;
	sprites->sprites[index].width = imageHeader.width;
	sprites->sprites[index].height = imageHeader.height;
	
	loadSpriteTexture (sprites, index);
	return true;
}

void pasteSprite (sprite * single, const spritePalette *fontPal, bool showBox) {
	
	if (! single) return;
	if (single->texNum < 0) return;
	
	float tx1 = 0.0;
	float ty1 = 0.0;
	float tx2 = 1.0;
	float ty2 = 1.0;

	int x1 = -single->xhot;
	int y1 = -single->yhot;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (showBox) {
		glColor4f(0.35f, 1.0f, 0.35f, 0.5f);
		//    glBegin(GL_LINE_LOOP);
		glBegin(GL_QUADS);			
		{
			glVertex3f(x1, -y1, 0.0);
			glVertex3f(x1+single->width, -y1, 0.0);
			glVertex3f(x1+single->width, -y1-single->height, 0.0);
			glVertex3f(x1, -y1-single->height, 0.0);
		}
		glEnd();
	}
	
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
	
	glEnable (GL_TEXTURE_2D);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); 
	
	// Draw the sprite
	glBindTexture (GL_TEXTURE_2D, fontPal->tex_names[single->texNum]);
			
	glBegin(GL_QUADS);			
	glTexCoord2f(tx1, ty1);	glVertex3f(x1, -y1, 0.0);
	glTexCoord2f(tx2, ty1);	glVertex3f(x1+single->width, -y1, 0.0);
	glTexCoord2f(tx2, ty2);	glVertex3f(x1+single->width, -y1-single->height, 0.0);
	glTexCoord2f(tx1, ty2);	glVertex3f(x1, -y1-single->height, 0.0);
	glEnd();
}

