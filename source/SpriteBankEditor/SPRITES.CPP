#include <OpenGL/gl.h>
#include <stdlib.h>
#include <string.h>

#include "TGA.h"
#include "Sprites.h"

void forgetSpriteBank (spriteBank & forgetme) {
	//unsigned int index;

	glDeleteTextures (forgetme.myPalette.numTextures, forgetme.myPalette.tex_names);
	if (forgetme.isFont) glDeleteTextures (forgetme.myPalette.numTextures, forgetme.myPalette.burnTex_names);

	/*
	for (index = 0; index < forgetme.total; index ++) {
		delete forgetme.sprites[index].data;
	}*/

	delete forgetme.sprites;
}

bool reserveSpritePal (spritePalette & sP, int n) {
	sP.pal = new unsigned short int [n];
	sP.r = new unsigned char [n];
	sP.g = new unsigned char [n];
	sP.b = new unsigned char [n];
	sP.total = n;
	return (bool) (sP.pal != NULL) && (sP.r != NULL) && (sP.g != NULL) && (sP.b != NULL);
}

unsigned int get2bytes (unsigned char ** fp) {
	int r = (**fp) * 256 + *(*fp+1);
	*fp+=2;
	return r;
}

short readSigned (unsigned char **fp) {
	short f = **((short **) fp);
	*fp += sizeof(short);
	//fread (& f, sizeof (short), 1, fp);
	return f;
}

bool loadSpriteBank (unsigned char * file, spriteBank *loadhere) {
	int i, total, picwidth, picheight, loadSaveMode = 0, howmany,
		startIndex;
	int totalwidth[256], maxheight[256];
	int numTextures = 0;
	
	//debugOut ("loadSpriteBank: Loading new sprite bank... \n");
	
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

	if (! reserveSpritePal (loadhere->myPalette, howmany + startIndex)) return false;

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
		delete loadhere->sprites[i].data;
		
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

void pasteSprite (sprite * single, const spritePalette *fontPal, bool showBox) {
	
	if (! single) return;
	
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


void fontSprite (int x, int y, sprite & single, const spritePalette & fontPal) {

	float tx1 = (float)(single.tex_x + 0.5) / fontPal.tex_w[single.texNum];
	float ty1 = 0.0;
	float tx2 = (float)(single.tex_x + single.width - 0.5) / fontPal.tex_w[single.texNum];
	float ty2 = (float)(single.height)/fontPal.tex_h[single.texNum];
	
	int x1 = x - single.xhot;
	int y1 = y - single.yhot;
	int x2 = x1 + single.width;
	int y2 = y1 + single.height;
	
	glEnable (GL_TEXTURE_2D);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // GL_MODULATE instead of decal mixes the colours!
	glColor3ub (fontPal.originalRed, fontPal.originalGreen, fontPal.originalBlue);
	glBindTexture (GL_TEXTURE_2D, fontPal.tex_names[single.texNum]);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glBegin(GL_QUADS);
	
	glTexCoord2f(tx1, ty1);	glVertex3f(x1, y1, 0.0);
	glTexCoord2f(tx2, ty1);	glVertex3f(x2, y1, 0.0);
	glTexCoord2f(tx2, ty2);	glVertex3f(x2, y2, 0.0);
	glTexCoord2f(tx1, ty2);	glVertex3f(x1, y2, 0.0);

	glEnd();
	glDisable(GL_BLEND);
}

