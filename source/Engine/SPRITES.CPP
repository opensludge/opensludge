#include <libpng/png.h>
#include "allfiles.h"

#include "fileset.h"
#include "sprites.h"
#include "sprites_AA.h"
#include "moreio.h"
#include "newfatal.h"
#include "colours.h"
#include "backdrop.h"
#include "sludger.h"
#include "zbuffer.h"
#include "debug.h"
#include "Graphics.h"

extern zBufferData zBuffer;
extern GLuint backdropTextureName;

extern inputType input;
extern int cameraX, cameraY;

unsigned char currentBurnR = 0, currentBurnG = 0, currentBurnB = 0;

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

bool loadSpriteBank (int fileNum, spriteBank & loadhere, bool isFont) {
	int i, tex_num, total, picwidth, picheight, loadSaveMode = 0, howmany,
		startIndex;
	int totalwidth[256], maxheight[256];
	int numTextures = 0;
	byte * data;
	
	//debugOut ("loadSpriteBank: Loading new sprite bank... \n");

	setResourceForFatal (fileNum);
	if (! openFileFromNum (fileNum)) return fatal ("Can't open sprite bank / font");
	
	loadhere.isFont = isFont;

	total = get2bytes (bigDataFile);
	if (! total) {
		loadSaveMode = fgetc (bigDataFile);
		if (loadSaveMode == 1) {
			total = 0;
		} else {
			total = get2bytes (bigDataFile);
		}
	}
	
	if (loadSaveMode > 3) return fatal ("Unsupported sprite bank file format");
	if (total <= 0) return fatal ("No sprites in bank or invalid sprite bank file");

	loadhere.total = total;
	loadhere.sprites = new sprite [total];
	byte ** spriteData = new byte * [total];
	if (! checkNew (loadhere.sprites)) return false;

	if (loadSaveMode && loadSaveMode < 3) {
		howmany = fgetc (bigDataFile);
		startIndex = 1;
	}
	
	totalwidth[0] = maxheight[0] = 0;

	for (i = 0; i < total; i ++) {
		switch (loadSaveMode) {
			case 3:
			{
				loadhere.sprites[i].xhot = readSigned (bigDataFile);
				loadhere.sprites[i].yhot = readSigned (bigDataFile);
				
				png_structp png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
				if (!png_ptr) {
					return fatal ("Can't open sprite bank / font.");
				}
					
				png_infop info_ptr = png_create_info_struct(png_ptr);
				if (!info_ptr) {
					png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
					return fatal ("Can't open sprite bank / font.");
				}
					
				png_infop end_info = png_create_info_struct(png_ptr);
				if (!end_info) {
					png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
					return fatal ("Can't open sprite bank / font.");
				}
				png_init_io(png_ptr, bigDataFile);		// Tell libpng which file to read
				png_set_sig_bytes(png_ptr, 8);			// No sig
				
				png_read_info(png_ptr, info_ptr);
				
				png_uint_32 width, height;
				int bit_depth, color_type, interlace_type, compression_type, filter_method;
				png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);
				
				int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
				
				unsigned char * row_pointers[height];
				spriteData[i] = new unsigned char [rowbytes*height];
				for (int row = 0; row<height; row++)
					row_pointers[row] = spriteData[i] + row*rowbytes;
					
				png_read_image(png_ptr, (png_byte **) row_pointers);
				png_read_end(png_ptr, NULL);
				png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
				
				picwidth = loadhere.sprites[i].width = width;
				picheight = loadhere.sprites[i].height = height;
				break;
			}
			case 2:
			picwidth = get2bytes (bigDataFile);
			picheight = get2bytes (bigDataFile);
			loadhere.sprites[i].xhot = readSigned (bigDataFile);
			loadhere.sprites[i].yhot = readSigned (bigDataFile);
			break;
			
			default:
			picwidth = (byte) fgetc (bigDataFile);
			picheight = (byte) fgetc (bigDataFile);
			loadhere.sprites[i].xhot = fgetc (bigDataFile);
			loadhere.sprites[i].yhot = fgetc (bigDataFile);
			break;
		}
		
		if (totalwidth[numTextures] + picwidth < 2048) {
			loadhere.sprites[i].tex_x = totalwidth[numTextures];
			totalwidth[numTextures] += (loadhere.sprites[i].width = picwidth);
			if ((loadhere.sprites[i].height = picheight) > maxheight[numTextures]) maxheight[numTextures] = picheight;
		} else {
			numTextures++;
			if (numTextures > 255) return fatal ("Can't open sprite bank / font - it's too big.");
			loadhere.sprites[i].tex_x = 0;
			totalwidth[numTextures] = (loadhere.sprites[i].width = picwidth);
			maxheight[numTextures] = loadhere.sprites[i].height = picheight;
		}
		loadhere.sprites[i].texNum = numTextures;

		if (loadSaveMode < 3) {
			data = (byte *) new byte [picwidth * (picheight + 1)];
			if (! checkNew (data)) return false;
			int ooo = picwidth * picheight;
			for (int tt = 0; tt < picwidth; tt ++) {
				data[ooo ++] = 0;
			}
			spriteData[i] = data;
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
	}
	numTextures++;

	if (! loadSaveMode) {
		howmany = fgetc (bigDataFile);
		startIndex = fgetc (bigDataFile);
	}

	if (loadSaveMode < 3) {
		if (! reserveSpritePal (loadhere.myPalette, howmany + startIndex)) return false;

		for (i = 0; i < howmany; i ++) {
			loadhere.myPalette.r[i + startIndex] = (byte) fgetc (bigDataFile);
			loadhere.myPalette.g[i + startIndex] = (byte) fgetc (bigDataFile);
			loadhere.myPalette.b[i + startIndex] = (byte) fgetc (bigDataFile);
			loadhere.myPalette.pal[i + startIndex] = makeColour (loadhere.myPalette.r[i + startIndex], loadhere.myPalette.g[i + startIndex], loadhere.myPalette.b[i + startIndex]);
		}
	}
	
	loadhere.myPalette.originalRed = loadhere.myPalette.originalGreen = loadhere.myPalette.originalBlue = 255;

	loadhere.myPalette.numTextures = numTextures;
	GLubyte * tmp[numTextures];
	GLubyte * tmp2[numTextures];
	for (tex_num = 0; tex_num < numTextures; tex_num++) {
		if (! NPOT_textures) {
			totalwidth[tex_num] = getNextPOT(totalwidth[tex_num]);
			maxheight[tex_num] = getNextPOT(maxheight[tex_num]);
		}		
		tmp[tex_num] = new GLubyte [maxheight[tex_num]*totalwidth[tex_num]*4];
		if (isFont) tmp2[tex_num] = new GLubyte [maxheight[tex_num]*totalwidth[tex_num]*4];
		loadhere.myPalette.tex_w[tex_num] = totalwidth[tex_num];
		loadhere.myPalette.tex_h[tex_num] = maxheight[tex_num];
	}


	int fromhere;
	unsigned char s;

	for (i = 0; i < total; i ++) {
		fromhere = 0;
		for (int y = 0; y < loadhere.sprites[i].height; y ++) {
			for (int x = loadhere.sprites[i].tex_x; x < loadhere.sprites[i].tex_x+loadhere.sprites[i].width; x ++) {
				GLubyte * target = tmp[loadhere.sprites[i].texNum] + 4*totalwidth[loadhere.sprites[i].texNum]*y + x*4;
				if (loadSaveMode < 3) {
					s = spriteData[i][fromhere++];
					if (s) {
						target[0] = (GLubyte) loadhere.myPalette.r[s];
						target[1] = (GLubyte) loadhere.myPalette.g[s];
						target[2] = (GLubyte) loadhere.myPalette.b[s];
						target[3] = (GLubyte) 255;
					} else {
						target[0] = (GLubyte) 0;
						target[1] = (GLubyte) 0;
						target[2] = (GLubyte) 0;
						target[3] = (GLubyte) 0;
					}
					if (isFont) {
						target = tmp2[loadhere.sprites[i].texNum] + 4*totalwidth[loadhere.sprites[i].texNum]*y + x*4;
						target[0] = (GLubyte) 255;
						target[1] = (GLubyte) 255;
						target[2] = (GLubyte) 255;
						if (s)
							target[3] = (GLubyte) loadhere.myPalette.r[s];
						else 
							target[3] = (GLubyte) 0;
					}
				} else {
					target[0] = (GLubyte) spriteData[i][fromhere++];
					target[1] = (GLubyte) spriteData[i][fromhere++];
					target[2] = (GLubyte) spriteData[i][fromhere++];
					target[3] = (GLubyte) spriteData[i][fromhere++];
				}
			}
		}
		delete spriteData[i];
	}
	delete spriteData;
	
	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
	
	glGenTextures (numTextures, loadhere.myPalette.tex_names);
	if (isFont)
		glGenTextures (numTextures, loadhere.myPalette.burnTex_names);
	
	for (tex_num = 0; tex_num < numTextures; tex_num++) {
	
		glBindTexture (GL_TEXTURE_2D, loadhere.myPalette.tex_names[tex_num]);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, totalwidth[tex_num], maxheight[tex_num], 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp[tex_num]);
	
		//fprintf (stderr, "%s w:%d h:%d %d\n", resourceNameFromNum(fileNum), totalwidth[tex_num], maxheight[tex_num], loadhere.myPalette.tex_names[tex_num]);

		delete tmp[tex_num];
		
		if (isFont) {
			glBindTexture (GL_TEXTURE_2D, loadhere.myPalette.burnTex_names[tex_num]);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, totalwidth[tex_num], maxheight[tex_num], 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp2[tex_num]);
						
			delete tmp2[tex_num];
		}
	}
		
	finishAccess ();
	setResourceForFatal (-1);
	return true;
}

void pasteSpriteToBackDrop (int x1, int y1, sprite & single, const spritePalette & fontPal) {
	
	float tx1 = (float)(single.tex_x + 0.5) / fontPal.tex_w[single.texNum];
	float ty1 = 0.0;
	float tx2 = (float)(single.tex_x + single.width - 0.5) / fontPal.tex_w[single.texNum];
	float ty2 = (float)(single.height)/fontPal.tex_h[single.texNum];

	int diffX = single.width;
	int diffY = single.height;	
	
	x1 -= single.xhot;
	y1 -= single.yhot;
	
	if (x1 < 0) diffX += x1;
	if (y1 < 0) diffY += y1;
	if (x1 + diffX > sceneWidth) diffX = sceneWidth - x1;
	if (y1 + diffY > sceneHeight) diffY = sceneHeight - y1;
	if (diffX < 0) return;
	if (diffY < 0) return;
	
	glEnable (GL_TEXTURE_2D);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // GL_MODULATE instead of decal mixes the colours!
	glColor3ub (fontPal.originalRed, fontPal.originalGreen, fontPal.originalBlue);
	glBindTexture (GL_TEXTURE_2D, fontPal.tex_names[single.texNum]);
	setPixelCoords (true);
	
	int xoffset = 0;
	while (xoffset < diffX) {
		int w = (diffX-xoffset < viewportWidth) ? diffX-xoffset : viewportWidth;
		
		int yoffset = 0;
		while (yoffset < diffY) {
			int h = (diffY-yoffset < viewportHeight) ? diffY-yoffset : viewportHeight;
			
			// Render the scene - first the old backdrop
			glBindTexture (GL_TEXTURE_2D, backdropTextureName);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex3f(0.325-x1-xoffset, 0.325-y1+yoffset, 0.0);
			glTexCoord2f(backdropTexW, 0.0); glVertex3f(sceneWidth-0.325-x1-xoffset, 0.325-y1+yoffset, 0.0);
			glTexCoord2f(backdropTexW, backdropTexH); glVertex3f(sceneWidth-0.325-x1-xoffset, sceneHeight-0.325-y1+yoffset, 0.0);
			glTexCoord2f(0.0, backdropTexH); glVertex3f(0.325-x1-xoffset, sceneHeight-0.325-y1+yoffset, 0.0);
			glEnd();	
			
			// Then the sprite
			glBindTexture (GL_TEXTURE_2D, fontPal.tex_names[single.texNum]);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			
			glBegin(GL_QUADS);			
			glTexCoord2f(tx1, ty1);	glVertex3f(0.325-xoffset, 0.325-yoffset, 0.0);
			glTexCoord2f(tx2, ty1);	glVertex3f(single.width-0.325-xoffset,0.325 -yoffset, 0.0);
			glTexCoord2f(tx2, ty2);	glVertex3f(single.width-0.325-xoffset, -yoffset+single.height-0.325, 0.0);
			glTexCoord2f(tx1, ty2);	glVertex3f(0.325-xoffset, -yoffset+single.height-0.325, 0.0);
			glEnd();
			glDisable(GL_BLEND);
	
			// Copy Our ViewPort To The Texture
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, (x1<0) ? xoffset : x1+xoffset, (y1<0) ? yoffset: y1+yoffset, viewportOffsetX, viewportOffsetY, w, h);
			
			yoffset += viewportHeight;
		}		
		xoffset += viewportWidth;
	}
	setPixelCoords (false);
	if (maxAntiAliasSettings.useMe) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}			
	
}

void burnSpriteToBackDrop (int x1, int y1, sprite & single, const spritePalette & fontPal) {
	
	/*pasteSpriteToBackDrop (x1, y1, single, fontPal);
	return;*/

	float tx1 = (float)(single.tex_x + 0.5) / fontPal.tex_w[single.texNum];
	float ty1 = 0.0;
	float tx2 = (float)(single.tex_x + single.width - 0.5) / fontPal.tex_w[single.texNum];
	float ty2 = (float)(single.height)/fontPal.tex_h[single.texNum];
	
	int diffX = single.width;
	int diffY = single.height;	
	
	x1 -= single.xhot;
	y1 -= single.yhot;
	
	if (x1 < 0) diffX += x1;
	if (y1 < 0) diffY += y1;
	if (x1 + diffX > sceneWidth) diffX = sceneWidth - x1;
	if (y1 + diffY > sceneHeight) diffY = sceneHeight - y1;
	if (diffX < 0) return;
	if (diffY < 0) return;
	
	setPixelCoords (true);
	glEnable (GL_TEXTURE_2D);
	glColor3ub (currentBurnR, currentBurnG, currentBurnB);
//	glColor4ub (255, 0, 0, 255);
	
	int xoffset = 0;
	while (xoffset < diffX) {
		int w = (diffX-xoffset < viewportWidth) ? diffX-xoffset : viewportWidth;
		
		int yoffset = 0;
		while (yoffset < diffY) {
			int h = (diffY-yoffset < viewportHeight) ? diffY-yoffset : viewportHeight;
			
			// Render the scene - first the old backdrop
			glBindTexture (GL_TEXTURE_2D, backdropTextureName);
			glBindTexture (GL_TEXTURE_2D, backdropTextureName);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex3f(0.325-x1-xoffset, 0.325-y1+yoffset, 0.0);
			glTexCoord2f(backdropTexW, 0.0); glVertex3f(sceneWidth-0.325-x1-xoffset, 0.325-y1+yoffset, 0.0);
			glTexCoord2f(backdropTexW, backdropTexH); glVertex3f(sceneWidth-0.325-x1-xoffset, sceneHeight-0.325-y1+yoffset, 0.0);
			glTexCoord2f(0.0, backdropTexH); glVertex3f(0.325-x1-xoffset, sceneHeight-0.325-y1+yoffset, 0.0);
			glEnd();	
			
			// Then the sprite
			glEnable(GL_BLEND);
			glBindTexture (GL_TEXTURE_2D, fontPal.burnTex_names[single.texNum]);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // GL_MODULATE instead of decal mixes the colours!
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			
			glBegin(GL_QUADS);			
			glTexCoord2f(tx1, ty1);	glVertex3f(0.325-xoffset, 0.325-yoffset, 0.0);
			glTexCoord2f(tx2, ty1);	glVertex3f(single.width-0.325-xoffset, 0.325-yoffset, 0.0);
			glTexCoord2f(tx2, ty2);	glVertex3f(single.width-0.325-xoffset, single.height-0.325-yoffset, 0.0);
			glTexCoord2f(tx1, ty2);	glVertex3f(0.325-xoffset, single.height-0.325-yoffset, 0.0);
			glEnd();
			glDisable(GL_BLEND);
			
			// Copy Our ViewPort To The Texture
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, (x1<0) ? xoffset : x1+xoffset, (y1<0) ? yoffset: y1+yoffset, viewportOffsetX, viewportOffsetY, w, h);
			
			yoffset += viewportHeight;
		}		
		xoffset += viewportWidth;
	}
	setPixelCoords (false);	
	if (maxAntiAliasSettings.useMe) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}			
	
}

extern GLuint backdropTextureName;

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

void flipFontSprite (int x, int y, sprite & single, const spritePalette & fontPal) {

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
	
	glTexCoord2f(tx1, ty1);	glVertex3f(x2, y1, 0.0);
	glTexCoord2f(tx2, ty1);	glVertex3f(x1, y1, 0.0);
	glTexCoord2f(tx2, ty2);	glVertex3f(x1, y2, 0.0);
	glTexCoord2f(tx1, ty2);	glVertex3f(x2, y2, 0.0);
	
	glEnd();
	glDisable(GL_BLEND);
	
}



enum drawModes {
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
	drawModeInvisible,
	numDrawModes
};

unsigned char curLight[3];

void setDrawMode (unsigned int drawMode) {
	switch (drawMode) {
		case drawModeTransparent3:
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4ub (curLight[0], curLight[1], curLight[2], 192);
			break;
		case drawModeTransparent2:
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4ub (curLight[0], curLight[1], curLight[2], 128);
			break;
		case drawModeTransparent1:
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4ub (curLight[0], curLight[1], curLight[2], 64);
			break;
		case drawModeInvisible:
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			// 1 rather than 0 so that we can do collision detection.
			glColor4ub (curLight[0], curLight[1], curLight[2], 1);
			break;
		case drawModeDark1:
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4ub (curLight[0]*3/4, curLight[1]*3/4, curLight[2]*3/4, 255);
			break;
		case drawModeDark2:
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4ub (curLight[0]/2, curLight[1]/2, curLight[2]/2, 255);
			break;
		case drawModeDark3:
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4ub (curLight[0]/4, curLight[1]/4, curLight[2]/4, 255);
			break;
		case drawModeBlack:
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4ub (0, 0, 0, 255);
			break;
		case 	drawModeShadow1:
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4ub (0, 0, 0, 192);
			break;
		case 	drawModeShadow2:
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4ub (0, 0, 0, 128);
			break;
		case 	drawModeShadow3:
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4ub (0, 0, 0, 64);
			break;
		case 	drawModeFoggy3:
			glEnable(GL_COLOR_SUM);
			glSecondaryColor3ub (curLight[0]*3/8, curLight[1]*3/8, curLight[2]*3/8);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4ub (curLight[0]/4, curLight[1]/4, curLight[2]/4, 255);
			break;
		case 	drawModeFoggy2:
			glEnable(GL_COLOR_SUM);
			glSecondaryColor3ub (curLight[0]/4, curLight[1]/4, curLight[2]/4);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4ub (curLight[0]/2, curLight[1]/2, curLight[2]/2, 255);
			break;
		case 	drawModeFoggy1:
			glEnable(GL_COLOR_SUM);
			glSecondaryColor3ub (curLight[0]/8, curLight[1]/8, curLight[2]/8);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4ub (curLight[0]*3/4, curLight[1]*3/4, curLight[2]*3/4, 255);
			break;
		case 	drawModeFoggy4:
			glEnable(GL_COLOR_SUM);
			glSecondaryColor3ub (128, 128, 128);
			glColor4ub (0, 0, 0, 255);
			break;
		case 	drawModeGlow3:
			glEnable(GL_COLOR_SUM);
			glSecondaryColor3ub (curLight[0]*3/4, curLight[1]*3/4, curLight[2]*3/4);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4ub (curLight[0]/4, curLight[1]/4, curLight[2]/4, 255);
			break;
		case 	drawModeGlow2:
			glEnable(GL_COLOR_SUM);
			glSecondaryColor3ub (curLight[0]/2, curLight[1]/2, curLight[2]/2);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4ub (curLight[0]/2, curLight[1]/2, curLight[2]/2, 255);
			break;
		case 	drawModeGlow1:
			glEnable(GL_COLOR_SUM);
			glSecondaryColor3ub (curLight[0]/4, curLight[1]/4, curLight[2]/4);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4ub (curLight[0]*3/4, curLight[1]*3/4, curLight[2]*3/4, 255);
			break;
		case 	drawModeGlow4:
			glEnable(GL_COLOR_SUM);
			glSecondaryColor3ub (255, 255, 255);
			glColor4ub (0, 0, 0, 255);
			break;
		default:
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4ub (curLight[0], curLight[1], curLight[2], 255);
			break;
	}
}

extern GLuint backdropTextureName;
bool checkColourChange (bool reset);

bool scaleSprite (int x, int y, sprite & single, const spritePalette & fontPal, float scale, unsigned int drawMode, int floaty, bool useZB, bool light, bool mirror, bool boundingBoxCollision, aaSettingsStruct * aa) {
	if (scale <= 0.05) return false;
		
	float tx1 = (float)(single.tex_x + 0.5) / fontPal.tex_w[single.texNum];
	float ty1 = 0.0;
	float tx2 = (float)(single.tex_x + single.width - 0.5) / fontPal.tex_w[single.texNum];
	float ty2 = (float)(single.height)/fontPal.tex_h[single.texNum];
	
	int diffX = single.width * scale + 0.5;
	int diffY = single.height * scale + 0.5;
	float x1 = x - (mirror ? (single.width - (single.xhot + 1)) : single.xhot) * scale - 0.325;
	float y1 = y - (single.yhot - floaty) * scale - 0.325;
	float x2 = x1 + diffX + 0.325;
	float y2 = y1 + diffY + 0.825;
	
	double z;

	if (useZB && zBuffer.numPanels) {
		int i;
		for (i = 1; i<zBuffer.numPanels; i++) {
			if (zBuffer.panel[i] >= y + cameraY) {
				i--;
				break;
			}
		}
		z = 0.999 - (double) i * (1.0 / 128.0);
	} else {
		z = -0.5;	
	}
	
	if (light && lightMapTexture) {
		if (lightMapMode == LIGHTMAPMODE_HOTSPOT) {
			int lx=x+cameraX;
			int ly=y+cameraY;
			if (lx<0 || ly<0 || lx>=sceneWidth || ly>=sceneHeight) {
				curLight[0] = curLight[1] = curLight[2] = 255;
			} else {
				GLubyte *target;
				if (! NPOT_textures) {
					target = lightMapTexture + (ly*getNextPOT(sceneWidth) + lx)*4;
				} else {
					target = lightMapTexture + (ly*sceneWidth + lx)*4;
				}
				curLight[0] = target[0];
				curLight[1] = target[1];
				curLight[2] = target[2];
			}
		} else if (lightMapMode == LIGHTMAPMODE_PIXEL) {
			curLight[0] = curLight[1] = curLight[2] = 255;
			glActiveTexture(GL_TEXTURE1);
			glEnable(GL_TEXTURE_2D);
			glBindTexture (GL_TEXTURE_2D, lightMapTextureName);
			glActiveTexture(GL_TEXTURE0);
		}
	} else {
		curLight[0] = curLight[1] = curLight[2] = 255;
	}
	
	
	glEnable (GL_TEXTURE_2D);
	
	setDrawMode (drawMode);
			
	glBindTexture (GL_TEXTURE_2D, fontPal.tex_names[single.texNum]);
	if (aa->useMe && maxAntiAliasSettings.useMe)
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	else
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glBegin(GL_QUADS);

	float ltx1;
	float ltx2;
	float lty1;
	float lty2;
	if (! NPOT_textures) {
		ltx1 = (float) (x1+cameraX) / getNextPOT(sceneWidth);
		ltx2 = (float) (x2+cameraX) / getNextPOT(sceneWidth);
		 lty1 = (float) (y1+cameraY) / getNextPOT(sceneHeight);
		 lty2 = (float) (y2+cameraY) / getNextPOT(sceneHeight);
	} else {
		 ltx1 = (float) (x1+cameraX) / sceneWidth;
		 ltx2 = (float) (x2+cameraX) / sceneWidth;
		 lty1 = (float) (y1+cameraY) / sceneHeight;
		 lty2 = (float) (y2+cameraY) / sceneHeight;
	}
	
	
	if (! mirror) {
		glTexCoord2f(tx1, ty1); glMultiTexCoord2f(GL_TEXTURE1, ltx1, lty1); glVertex3f(x1, y1, z);
		glTexCoord2f(tx2, ty1);	glMultiTexCoord2f(GL_TEXTURE1, ltx2, lty1); glVertex3f(x2, y1, z);
		glTexCoord2f(tx2, ty2);	glMultiTexCoord2f(GL_TEXTURE1, ltx2, lty2); glVertex3f(x2, y2, z);
		glTexCoord2f(tx1, ty2);	glMultiTexCoord2f(GL_TEXTURE1, ltx1, lty2); glVertex3f(x1, y2, z);
	} else {
		glTexCoord2f(tx2, ty1); glMultiTexCoord2f(GL_TEXTURE1, ltx1, lty1); glVertex3f(x1, y1, z);
		glTexCoord2f(tx1, ty1);	glMultiTexCoord2f(GL_TEXTURE1, ltx2, lty1); glVertex3f(x2, y1, z);
		glTexCoord2f(tx1, ty2);	glMultiTexCoord2f(GL_TEXTURE1, ltx2, lty2); glVertex3f(x2, y2, z);
		glTexCoord2f(tx2, ty2);	glMultiTexCoord2f(GL_TEXTURE1, ltx1, lty2); glVertex3f(x1, y2, z);
	}

	glEnd();
	glDisable(GL_BLEND);
	
	if (light && lightMapMode == LIGHTMAPMODE_PIXEL) {
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
	}
	glDisable(GL_COLOR_SUM);
	
	// Are we pointing at the sprite?	
	if (input.mouseX >= x1 && input.mouseX <= x2 && input.mouseY >= y1 && input.mouseY <= y2) {
		if (boundingBoxCollision) return true;
		return checkColourChange (false);
	}
	return false;
}

// Paste a scaled sprite onto the backdrop 
void fixScaleSprite (int x, int y, sprite & single, const spritePalette & fontPal, float scale, unsigned int drawMode, int floaty, bool useZB, bool light, int camX, int camY, bool mirror, aaSettingsStruct * aa) {
	if (scale <= 0.05) return;
		
	float tx1 = (float)(single.tex_x + 0.5) / fontPal.tex_w[single.texNum];
	float ty1 = 0.0;
	float tx2 = (float)(single.tex_x + single.width - 0.5) / fontPal.tex_w[single.texNum];
	float ty2 = (float)(single.height)/fontPal.tex_h[single.texNum];
	
	int diffX = single.width * scale + 0.5;
	int diffY = single.height * scale + 0.5;
	int spriteWidth = diffX;
	int spriteHeight = diffY;
	int x1 = x - (mirror ? (single.width - (single.xhot + 1)) : single.xhot) * scale;
	int y1 = y - (single.yhot - floaty) * scale;
	if (x1 < 0) diffX += x1;
	if (y1 < 0) diffY += y1;
	if (x1 + diffX > sceneWidth) diffX = sceneWidth - x1;
	if (y1 + diffY > sceneHeight) diffY = sceneHeight - y1;
	if (diffX < 0) return;
	if (diffY < 0) return;

	double z;
	
	if (useZB && zBuffer.numPanels) {
		int i;
		for (i = 1; i<zBuffer.numPanels; i++) {
			if (zBuffer.panel[i] >= y + cameraY) {
				i--;
				break;
			}
		}
		z = 0.999 - (double) i * (1.0 / 128.0);
	} else {
		z = -0.5;	
	}
	
	if (light && lightMapTexture) {
		if (lightMapMode == LIGHTMAPMODE_HOTSPOT) {
			int lx=x+cameraX;
			int ly=y+cameraY;
			if (lx<0 || ly<0 || lx>=sceneWidth || ly>=sceneHeight) {
				curLight[0] = curLight[1] = curLight[2] = 255;
			} else {
				GLubyte *target = lightMapTexture + (ly*sceneWidth + lx)*4;
				curLight[0] = target[0];
				curLight[1] = target[1];
				curLight[2] = target[2];
			}
		} else if (lightMapMode == LIGHTMAPMODE_PIXEL) {
			curLight[0] = curLight[1] = curLight[2] = 255;
			glActiveTexture(GL_TEXTURE1);
			glEnable(GL_TEXTURE_2D);
			glBindTexture (GL_TEXTURE_2D, lightMapTextureName);
			glActiveTexture(GL_TEXTURE0);
		}
	} else {
		curLight[0] = curLight[1] = curLight[2] = 255;
	}
	
	float ltx1 = (float) x1 / sceneWidth;
	float ltx2 = (float) (x1+spriteWidth) / sceneWidth;
	float lty1 = (float) y1 / sceneHeight;
	float lty2 = (float) (y1+spriteHeight) / sceneHeight;
	
	
	glEnable (GL_TEXTURE_2D);
	setPixelCoords (true);
	int xoffset = 0;
	while (xoffset < diffX) {
		int w = (diffX-xoffset < viewportWidth) ? diffX-xoffset : viewportWidth;
		
		int yoffset = 0;
		while (yoffset < diffY) {
			int h = (diffY-yoffset < viewportHeight) ? diffY-yoffset : viewportHeight;
			
			if (useZB) {
				glDepthMask (GL_TRUE);
				glClear(GL_DEPTH_BUFFER_BIT);
				drawZBuffer(x1+xoffset+camX, y1+yoffset+camY, false);

				glDepthMask (GL_FALSE);
				glEnable(GL_DEPTH_TEST);
			}
			
			// Render the scene - first the old backdrop
			glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glBindTexture (GL_TEXTURE_2D, backdropTextureName);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			if (light && lightMapMode == LIGHTMAPMODE_PIXEL) {
				glActiveTexture(GL_TEXTURE1);
				glDisable(GL_TEXTURE_2D);
				glActiveTexture(GL_TEXTURE0);
			}
			
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex3f(0.325-x1-xoffset, 0.325-y1+yoffset, 0.0);
			glTexCoord2f(backdropTexW, 0.0); glVertex3f(sceneWidth-0.325-x1-xoffset, 0.325-y1+yoffset, 0.0);
			glTexCoord2f(backdropTexW, backdropTexH); glVertex3f(sceneWidth-0.325-x1-xoffset, sceneHeight-0.325-y1+yoffset, 0.0);
			glTexCoord2f(0.0, backdropTexH); glVertex3f(0.325-x1-xoffset, sceneHeight-0.325-y1+yoffset, 0.0);
			glEnd();	

			// Then the sprite
			setDrawMode (drawMode);			
			glBindTexture (GL_TEXTURE_2D, fontPal.tex_names[single.texNum]);
			
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			if (light && lightMapMode == LIGHTMAPMODE_PIXEL) {
				glActiveTexture(GL_TEXTURE1);
				glEnable(GL_TEXTURE_2D);
				glActiveTexture(GL_TEXTURE0);
			}
			
			glBegin(GL_QUADS);			
			if (! mirror) {
				glTexCoord2f(tx2, ty1); glMultiTexCoord2f(GL_TEXTURE1, ltx2, lty1); 	glVertex3f(spriteWidth-0.325-xoffset, 0.325-yoffset, z);
				glTexCoord2f(tx1, ty1); glMultiTexCoord2f(GL_TEXTURE1, ltx1, lty1); 	glVertex3f(0.325-xoffset, 0.325-yoffset, z);
				glTexCoord2f(tx1, ty2); glMultiTexCoord2f(GL_TEXTURE1, ltx1, lty2); 	glVertex3f(0.325-xoffset, spriteHeight-0.325-yoffset, z);
				glTexCoord2f(tx2, ty2); glMultiTexCoord2f(GL_TEXTURE1, ltx2, lty2); 	glVertex3f(spriteWidth-0.325-xoffset, spriteHeight-0.325-yoffset, z);
			} else {
				glTexCoord2f(tx1, ty1); glMultiTexCoord2f(GL_TEXTURE1, ltx2, lty1); 	glVertex3f(spriteWidth-0.325-xoffset, 0.325-yoffset, z);
				glTexCoord2f(tx2, ty1); glMultiTexCoord2f(GL_TEXTURE1, ltx1, lty1); 	glVertex3f(0.325-xoffset, 0.325-yoffset, z);
				glTexCoord2f(tx2, ty2); glMultiTexCoord2f(GL_TEXTURE1, ltx1, lty2); 	glVertex3f(0.325-xoffset, spriteHeight-0.325-yoffset, z);
				glTexCoord2f(tx1, ty2); glMultiTexCoord2f(GL_TEXTURE1, ltx2, lty2); 	glVertex3f(spriteWidth-0.325-xoffset, spriteHeight-0.325-yoffset, z);
			}
			glEnd();
			glDisable(GL_BLEND);
			glDisable(GL_COLOR_SUM);
			
			// Copy Our ViewPort To The Texture
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, (x1<0) ? xoffset: x1+xoffset, (y1<0) ? yoffset: y1+yoffset, (x1<0) ?viewportOffsetX-x1:viewportOffsetX, (y1<0) ?viewportOffsetY-y1:viewportOffsetY, w, h);
			
			yoffset += viewportHeight;
		}		
		xoffset += viewportWidth;
	}
	
	setPixelCoords (false);
	if (maxAntiAliasSettings.useMe) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}			
	
	if (light && lightMapMode == LIGHTMAPMODE_PIXEL) {
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
	}
		
}
