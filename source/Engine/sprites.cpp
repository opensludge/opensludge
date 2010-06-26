#if defined __unix__ && !(defined __APPLE__)
#include <png.h>
#else
#include <libpng/png.h>
#endif

#include "allfiles.h"

#include "fileset.h"
#include "people.h"
#include "sprites.h"
#include "moreio.h"
#include "newfatal.h"
#include "colours.h"
#include "backdrop.h"
#include "sludger.h"
#include "zbuffer.h"
#include "debug.h"
#include "graphics.h"

#include "shaders.h"

extern zBufferData zBuffer;
extern GLuint backdropTextureName;

extern inputType input;
extern int cameraX, cameraY;
extern float cameraZoom;

unsigned char currentBurnR = 0, currentBurnG = 0, currentBurnB = 0;


void forgetSpriteBank (spriteBank & forgetme) {
	glDeleteTextures (forgetme.myPalette.numTextures, forgetme.myPalette.tex_names);
	if (forgetme.isFont) glDeleteTextures (forgetme.myPalette.numTextures, forgetme.myPalette.burnTex_names);

	delete forgetme.sprites;
	forgetme.sprites = NULL;
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
	int i, tex_num, total, picwidth, picheight, loadSaveMode = 0, howmany=0, startIndex=0;
	int totalwidth[256], maxheight[256];
	int numTextures = 0;
	byte * data;

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

	if (total <= 0) return fatal ("No sprites in bank or invalid sprite bank file");
	if (loadSaveMode > 3) return fatal ("Unsupported sprite bank file format");

	loadhere.total = total;
	loadhere.sprites = new sprite [total];
	byte ** spriteData = new byte * [total];
	if (! checkNew (loadhere.sprites)) return false;

	if (loadSaveMode && loadSaveMode < 3) {
		howmany = fgetc (bigDataFile);
		startIndex = 1;
	}

	totalwidth[0] = maxheight[0] = 1;

	for (i = 0; i < total; i ++) {
		switch (loadSaveMode) {
			case 3:
			{
				loadhere.sprites[i].xhot = getSigned (bigDataFile);
				loadhere.sprites[i].yhot = getSigned (bigDataFile);

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
				for (unsigned int row = 0; row<height; row++)
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
			loadhere.sprites[i].xhot = getSigned (bigDataFile);
			loadhere.sprites[i].yhot = getSigned (bigDataFile);
			break;

			default:
			picwidth = (byte) fgetc (bigDataFile);
			picheight = (byte) fgetc (bigDataFile);
			loadhere.sprites[i].xhot = fgetc (bigDataFile);
			loadhere.sprites[i].yhot = fgetc (bigDataFile);
			break;
		}

		if (totalwidth[numTextures] + picwidth < 2047) {
			loadhere.sprites[i].tex_x = totalwidth[numTextures];
			totalwidth[numTextures] += (loadhere.sprites[i].width = picwidth) + 1;
			if ((loadhere.sprites[i].height = picheight)+2 > maxheight[numTextures]) maxheight[numTextures] = picheight+2;
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
		tmp[tex_num] = new GLubyte [(maxheight[tex_num]+1)*totalwidth[tex_num]*4];
		memset (tmp[tex_num], 0, maxheight[tex_num]*totalwidth[tex_num]*4);
		if (isFont) {
			tmp2[tex_num] = new GLubyte [(maxheight[tex_num]+1)*totalwidth[tex_num]*4];
			memset (tmp2[tex_num], 0, maxheight[tex_num]*totalwidth[tex_num]*4);
		}
		loadhere.myPalette.tex_w[tex_num] = totalwidth[tex_num];
		loadhere.myPalette.tex_h[tex_num] = maxheight[tex_num];
	}

	int fromhere;
	unsigned char s;

	for (i = 0; i < total; i ++) {
		fromhere = 0;

		int transColour = -1;
		if (loadSaveMode < 3) {
			int size = loadhere.sprites[i].height * loadhere.sprites[i].width;
			while (fromhere < size) {
				s = spriteData[i][fromhere++];
				if (s) {
					transColour = s;
					break;
				}
			}
			fromhere = 0;
		}

		for (int y = 1; y < 1 + loadhere.sprites[i].height; y ++) {
			for (int x = loadhere.sprites[i].tex_x; x < loadhere.sprites[i].tex_x+loadhere.sprites[i].width; x ++) {
				GLubyte * target = tmp[loadhere.sprites[i].texNum] + 4*totalwidth[loadhere.sprites[i].texNum]*y + x*4;
				if (loadSaveMode < 3) {
					s = spriteData[i][fromhere++];
					if (s) {
						target[0] = (GLubyte) loadhere.myPalette.r[s];
						target[1] = (GLubyte) loadhere.myPalette.g[s];
						target[2] = (GLubyte) loadhere.myPalette.b[s];
						target[3] = (GLubyte) 255;
						transColour = s;
					} else if (transColour >=0) {
						target[0] = (GLubyte) loadhere.myPalette.r[transColour];
						target[1] = (GLubyte) loadhere.myPalette.g[transColour];
						target[2] = (GLubyte) loadhere.myPalette.b[transColour];
						target[3] = (GLubyte) 0;
					}
					if (isFont) {
						target = tmp2[loadhere.sprites[i].texNum] + 4*totalwidth[loadhere.sprites[i].texNum]*y + x*4;
						target[0] = (GLubyte) 255;
						target[1] = (GLubyte) 255;
						target[2] = (GLubyte) 255;
						if (s)
							target[3] = (GLubyte) loadhere.myPalette.r[s];
						/*else
							target[3] = (GLubyte) 0;*/
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
	spriteData = NULL;


	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

	glGenTextures (numTextures, loadhere.myPalette.tex_names);
	if (isFont)
		glGenTextures (numTextures, loadhere.myPalette.burnTex_names);


	for (tex_num = 0; tex_num < numTextures; tex_num++) {

		glBindTexture (GL_TEXTURE_2D, loadhere.myPalette.tex_names[tex_num]);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, totalwidth[tex_num], maxheight[tex_num], 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp[tex_num]);

		delete tmp[tex_num];
		tmp[tex_num] = NULL;

		if (isFont) {
			glBindTexture (GL_TEXTURE_2D, loadhere.myPalette.burnTex_names[tex_num]);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, totalwidth[tex_num], maxheight[tex_num], 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp2[tex_num]);

			delete tmp2[tex_num];
			tmp2[tex_num] = NULL;
		}
	}

	finishAccess ();
	setResourceForFatal (-1);

	return true;
}

void pasteSpriteToBackDrop (int x1, int y1, sprite & single, const spritePalette & fontPal) {

	float tx1 = (float)(single.tex_x) / fontPal.tex_w[single.texNum];
	float ty1 = 0.0;
	float tx2 = (float)(single.tex_x + single.width) / fontPal.tex_w[single.texNum];
	float ty2 = (float)(single.height)/fontPal.tex_h[single.texNum];

	float btx1;
	float btx2;
	float bty1;
	float bty2;
	
	int diffX = single.width;
	int diffY = single.height;

	x1 -= single.xhot;
	y1 -= single.yhot;

	if (! NPOT_textures) {
		btx1 = backdropTexW * x1 / sceneWidth;
		btx2 = backdropTexW * (x1+single.width) / sceneWidth;
		bty1 = backdropTexH * y1 / sceneHeight;
		bty2 = backdropTexH * (y1+single.height) / sceneHeight;
	} else {
		btx1 = (float) x1 / sceneWidth;
		btx2 = (float) (x1+single.width) / sceneWidth;
		bty1 = (float) y1 / sceneHeight;
		bty2 = (float) (y1+single.height) / sceneHeight;
	}	
	
	if (x1 < 0) diffX += x1;
	if (y1 < 0) diffY += y1;
	if (x1 + diffX > sceneWidth) diffX = sceneWidth - x1;
	if (y1 + diffY > sceneHeight) diffY = sceneHeight - y1;
	if (diffX < 0) return;
	if (diffY < 0) return;

	setPixelCoords (true);

	int xoffset = 0;
	while (xoffset < diffX) {
		int w = (diffX-xoffset < viewportWidth) ? diffX-xoffset : viewportWidth;

		int yoffset = 0;
		while (yoffset < diffY) {
			int h = (diffY-yoffset < viewportHeight) ? diffY-yoffset : viewportHeight;

			// Render the sprite to the backdrop 
			// (using mulitexturing, so the backdrop is seen where alpha < 1.0)
			glActiveTexture(GL_TEXTURE2);
			glBindTexture (GL_TEXTURE_2D, backdropTextureName);
			glActiveTexture(GL_TEXTURE0);

			glUseProgram(shader.paste);
			GLint uniform = glGetUniformLocation(shader.paste, "useLightTexture");
			if (uniform >= 0) glUniform1i(uniform, 0); // No lighting
			
			glColor4ub (fontPal.originalRed, fontPal.originalGreen, fontPal.originalBlue, 255);
			glBindTexture (GL_TEXTURE_2D, fontPal.tex_names[single.texNum]);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

			glBegin(GL_QUADS);
			glTexCoord2f(tx1, ty1); glMultiTexCoord2f(GL_TEXTURE2, btx1, bty1); 	glVertex3f(-xoffset, -yoffset, 0.0);
			glTexCoord2f(tx2, ty1); glMultiTexCoord2f(GL_TEXTURE2, btx2, bty1); 	glVertex3f(single.width-xoffset, -yoffset, 0.0);
			glTexCoord2f(tx2, ty2); glMultiTexCoord2f(GL_TEXTURE2, btx2, bty2); 	glVertex3f(single.width-xoffset, single.height-yoffset, 0.0);
			glTexCoord2f(tx1, ty2); glMultiTexCoord2f(GL_TEXTURE2, btx1, bty2); 	glVertex3f(-xoffset, single.height-yoffset, 0.0);
			glEnd();

			// Copy Our ViewPort To The Texture
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glUseProgram(0);
			
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, (int) ((x1<0) ? xoffset: x1+xoffset), (int) ((y1<0) ? yoffset: y1+yoffset), (int) ((x1<0) ?viewportOffsetX-x1:viewportOffsetX), (int) ((y1<0) ?viewportOffsetY-y1:viewportOffsetY), w, h);
			
			yoffset += viewportHeight;
		}
		xoffset += viewportWidth;
	}
	setPixelCoords (false);
}

void burnSpriteToBackDrop (int x1, int y1, sprite & single, const spritePalette & fontPal) {

	float tx1 = (float)(single.tex_x - 0.5) / fontPal.tex_w[single.texNum];
	float ty1 = 0.0;
	float tx2 = (float)(single.tex_x + single.width + 0.5) / fontPal.tex_w[single.texNum];
	float ty2 = (float)(single.height+2)/fontPal.tex_h[single.texNum];

	int diffX = single.width+1;
	int diffY = single.height+2;

	x1 -= single.xhot;
	y1 -= single.yhot-1;

	if (x1 < 0) diffX += x1;
	if (y1 < 0) diffY += y1;
	if (x1 + diffX > sceneWidth) diffX = sceneWidth - x1;
	if (y1 + diffY > sceneHeight) diffY = sceneHeight - y1;
	if (diffX < 0) return;
	if (diffY < 0) return;

	setPixelCoords (true);
	glColor3ub (currentBurnR, currentBurnG, currentBurnB);

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
			glTexCoord2f(0.0, 0.0); glVertex3f(-x1-xoffset, -y1+yoffset, 0.0);
			glTexCoord2f(backdropTexW, 0.0); glVertex3f(sceneWidth-1-x1-xoffset, -y1+yoffset, 0.0);
			glTexCoord2f(backdropTexW, backdropTexH); glVertex3f(sceneWidth-1-x1-xoffset, sceneHeight-1-y1+yoffset, 0.0);
			glTexCoord2f(0.0, backdropTexH); glVertex3f(-x1-xoffset, sceneHeight-1-y1+yoffset, 0.0);
			glEnd();

			// Then the sprite
			glEnable(GL_BLEND);
			glBindTexture (GL_TEXTURE_2D, fontPal.burnTex_names[single.texNum]);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // GL_MODULATE instead of decal mixes the colours!

			glBegin(GL_QUADS);
			glTexCoord2f(tx1, ty1);	glVertex3f(-xoffset, -yoffset, 0.0);
			glTexCoord2f(tx2, ty1);	glVertex3f(single.width-1-xoffset, -yoffset, 0.0);
			glTexCoord2f(tx2, ty2);	glVertex3f(single.width-1-xoffset, single.height-1-yoffset, 0.0);
			glTexCoord2f(tx1, ty2);	glVertex3f(-xoffset, single.height-1-yoffset, 0.0);
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
}

extern GLuint backdropTextureName;

void fontSprite (int x, int y, sprite & single, const spritePalette & fontPal) {

	float tx1 = (float)(single.tex_x-0.5) / fontPal.tex_w[single.texNum];
	float ty1 = 0.0;
	float tx2 = (float)(single.tex_x + single.width+0.5) / fontPal.tex_w[single.texNum];
	float ty2 = (float)(single.height+2)/fontPal.tex_h[single.texNum];

	float x1 = (float)x - (float)single.xhot/cameraZoom;
	float y1 = (float)y - (float)single.yhot/cameraZoom;
	float x2 = x1 + (float)single.width/cameraZoom;
	float y2 = y1 + (float)single.height/cameraZoom;

	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // GL_MODULATE instead of decal mixes the colours!
	glColor3ub (fontPal.originalRed, fontPal.originalGreen, fontPal.originalBlue);
	glBindTexture (GL_TEXTURE_2D, fontPal.tex_names[single.texNum]);

	if (gameSettings.antiAlias) {
		glUseProgram(shader.smartScaler);
		GLuint uniform = glGetUniformLocation(shader.smartScaler, "useLightTexture");
		if (uniform >= 0) glUniform1i(uniform, 0);
	}
	
	glEnable(GL_BLEND);

	glBegin(GL_QUADS);

	glTexCoord2f(tx1, ty1);	glVertex3f(x1, y1, 0.0);
	glTexCoord2f(tx2, ty1);	glVertex3f(x2, y1, 0.0);
	glTexCoord2f(tx2, ty2);	glVertex3f(x2, y2, 0.0);
	glTexCoord2f(tx1, ty2);	glVertex3f(x1, y2, 0.0);

	glEnd();
	glDisable(GL_BLEND);
	glUseProgram(0);
}

void flipFontSprite (int x, int y, sprite & single, const spritePalette & fontPal) {

	float tx1 = (float)(single.tex_x-0.5) / fontPal.tex_w[single.texNum];
	float ty1 = 0.0;
	float tx2 = (float)(single.tex_x + single.width+1.0) / fontPal.tex_w[single.texNum];
	float ty2 = (float)(single.height+2)/fontPal.tex_h[single.texNum];

	float x1 = (float)x - (float)single.xhot/cameraZoom;
	float y1 = (float)y - (float)single.yhot/cameraZoom;
	float x2 = x1 + (float)single.width/cameraZoom;
	float y2 = y1 + (float)single.height/cameraZoom;
	
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // GL_MODULATE instead of decal mixes the colours!
	glColor3ub (fontPal.originalRed, fontPal.originalGreen, fontPal.originalBlue);
	glBindTexture (GL_TEXTURE_2D, fontPal.tex_names[single.texNum]);
	
	if (gameSettings.antiAlias) {
		glUseProgram(shader.smartScaler);
		GLuint uniform = glGetUniformLocation(shader.smartScaler, "useLightTexture");
		if (uniform >= 0) glUniform1i(uniform, 0);
	}
	
	glEnable(GL_BLEND);
	
	glBegin(GL_QUADS);
	
	glTexCoord2f(tx1, ty1);	glVertex3f(x2, y1, 0.0);
	glTexCoord2f(tx2, ty1);	glVertex3f(x1, y1, 0.0);
	glTexCoord2f(tx2, ty2);	glVertex3f(x1, y2, 0.0);
	glTexCoord2f(tx1, ty2);	glVertex3f(x2, y2, 0.0);

	glEnd();
	glDisable(GL_BLEND);
	glUseProgram(0);
	
}




unsigned char curLight[3];

void setDrawMode (onScreenPerson * thisPerson) {
	if (thisPerson->colourmix) {
		glEnable(GL_COLOR_SUM);
		glSecondaryColor3ub (curLight[0]*thisPerson->r*thisPerson->colourmix/65025, curLight[1]*thisPerson->g*thisPerson->colourmix/65025, curLight[2]*thisPerson->b*thisPerson->colourmix/65025);
	}

	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glColor4ub (curLight[0]*(255-thisPerson->colourmix)/255, curLight[1]*(255-thisPerson->colourmix)/255, curLight[2]*(255-thisPerson->colourmix)/255, 255 - thisPerson->transparency);
}

extern GLuint backdropTextureName;
bool checkColourChange (bool reset);

bool scaleSprite (sprite & single, const spritePalette & fontPal, onScreenPerson * thisPerson, bool mirror) {

	float x = thisPerson->x;
	float y = thisPerson->y;
	
	float scale = thisPerson-> scale;
	bool light = ! (thisPerson->extra & EXTRA_NOLITE);
	
	if (scale <= 0.05) return false;

	float tx1 = (float)(single.tex_x) / fontPal.tex_w[single.texNum];
	float ty1 = (float) 1.0/fontPal.tex_h[single.texNum];
	float tx2 = (float)(single.tex_x + single.width) / fontPal.tex_w[single.texNum];
	float ty2 = (float)(single.height+1)/fontPal.tex_h[single.texNum];	
	
	int diffX = ((float)single.width) * scale;
	int diffY = ((float)single.height) * scale;

	int x1, y1, x2, y2;
	
	if (thisPerson -> extra & EXTRA_FIXTOSCREEN) {
		x = x / cameraZoom;
		y = y / cameraZoom;
		if (single.xhot < 0)
			x1 = x - (mirror ? (float) (single.width - single.xhot) : (float)(single.xhot+1) ) * scale/cameraZoom;
		else
			x1 = x - (mirror ? (float) (single.width - (single.xhot+1)) : (float)single.xhot ) * scale / cameraZoom;
		y1 = y - (single.yhot - thisPerson->floaty) * scale / cameraZoom;
		x2 = x1 + diffX / cameraZoom;
		y2 = y1 + diffY / cameraZoom;
	} else {
		x -= cameraX;
		y -= cameraY;
		if (single.xhot < 0)
			x1 = x - (mirror ? (float) (single.width - single.xhot) : (float)(single.xhot+1) ) * scale;
		else
			x1 = x - (mirror ? (float) (single.width - (single.xhot+1)) : (float)single.xhot ) * scale;
		y1 = y - (single.yhot - thisPerson->floaty) * scale;
		x2 = x1 + diffX;
		y2 = y1 + diffY;
	}

	double z;

	if ((! (thisPerson->extra & EXTRA_NOZB)) && zBuffer.numPanels) {
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

	if (light && lightMap.data) {
		if (lightMapMode == LIGHTMAPMODE_HOTSPOT) {
			int lx=x+cameraX;
			int ly=y+cameraY;
			
			if (lx<0) lx = 0;
			else if (lx>=sceneWidth) lx = sceneWidth-1;
			if (ly<0) ly = 0;
			else if (ly>=sceneHeight) ly = sceneHeight-1;
			
			GLubyte *target;
			if (! NPOT_textures) {
				target = lightMap.data + (ly*getNextPOT(sceneWidth) + lx)*4;
			} else {
				target = lightMap.data + (ly*sceneWidth + lx)*4;
			}
			curLight[0] = target[0];
			curLight[1] = target[1];
			curLight[2] = target[2];
		} else if (lightMapMode == LIGHTMAPMODE_PIXEL) {
			curLight[0] = curLight[1] = curLight[2] = 255;
			glActiveTexture(GL_TEXTURE1);
			glEnable(GL_TEXTURE_2D);
			glBindTexture (GL_TEXTURE_2D, lightMap.name);
			glActiveTexture(GL_TEXTURE0);
		}
	} else {
		curLight[0] = curLight[1] = curLight[2] = 255;
	}

	if (! (thisPerson->extra & EXTRA_RECTANGULAR))
		checkColourChange (true);

	setDrawMode (thisPerson);

	glBindTexture (GL_TEXTURE_2D, fontPal.tex_names[single.texNum]);

	glEnable(GL_BLEND);

	if (gameSettings.antiAlias) {
		glUseProgram(shader.smartScaler);
		GLuint uniform = glGetUniformLocation(shader.smartScaler, "useLightTexture");
		if (uniform >= 0) glUniform1i(uniform, light && lightMapMode == LIGHTMAPMODE_PIXEL && lightMap.data);
	}
	
	glBegin(GL_QUADS);

	float ltx1, ltx2, lty1, lty2;
	if (! NPOT_textures) {
		ltx1 = lightMap.texW * (x1+cameraX) / sceneWidth;
		ltx2 = lightMap.texW * (x2+cameraX) / sceneWidth;
		lty1 = lightMap.texH * (y1+cameraY) / sceneHeight;
		lty2 = lightMap.texH * (y2+cameraY) / sceneHeight;
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
	glUseProgram(0);

	if (light && lightMapMode == LIGHTMAPMODE_PIXEL) {
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
	}
	glSecondaryColor3ub (0, 0, 0);
	glDisable(GL_COLOR_SUM);

	// Are we pointing at the sprite?
	if (input.mouseX >= x1 && input.mouseX <= x2 && input.mouseY >= y1 && input.mouseY <= y2) {
		if (thisPerson->extra & EXTRA_RECTANGULAR) return true;
		return checkColourChange (false);
	}
	return false;
}

// Paste a scaled sprite onto the backdrop
void fixScaleSprite (int x, int y, sprite & single, const spritePalette & fontPal, onScreenPerson * thisPerson, int camX, int camY, bool mirror) {
	
	float scale = thisPerson-> scale;
	bool useZB = ! (thisPerson->extra & EXTRA_NOZB);
	bool light = ! (thisPerson->extra & EXTRA_NOLITE);
	
	if (scale <= 0.05) return;

	float tx1 = (float)(single.tex_x) / fontPal.tex_w[single.texNum];
	float ty1 = (float) 1.0/fontPal.tex_h[single.texNum];//0.0;
	float tx2 = (float)(single.tex_x + single.width) / fontPal.tex_w[single.texNum];
	float ty2 = (float)(single.height+1)/fontPal.tex_h[single.texNum];

	int diffX = ((float)single.width) * scale;
	int diffY = ((float)single.height) * scale;
	int x1;
	if (single.xhot < 0)
		x1 = x - (mirror ? (float) (single.width - single.xhot) : (float)(single.xhot+1) ) * scale;
	else
		x1 = x - (mirror ? (float) (single.width - (single.xhot+1)) : (float)single.xhot ) * scale;
	int y1 = y - (single.yhot - thisPerson->floaty) * scale;

	float spriteWidth = diffX;
	float spriteHeight = diffY;
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

	if (light && lightMap.data) {
		if (lightMapMode == LIGHTMAPMODE_HOTSPOT) {
			int lx=x+cameraX;
			int ly=y+cameraY;
			if (lx<0 || ly<0 || lx>=sceneWidth || ly>=sceneHeight) {
				curLight[0] = curLight[1] = curLight[2] = 255;
			} else {
				GLubyte *target = lightMap.data + (ly*sceneWidth + lx)*4;
				curLight[0] = target[0];
				curLight[1] = target[1];
				curLight[2] = target[2];
			}
		} else if (lightMapMode == LIGHTMAPMODE_PIXEL) {
			curLight[0] = curLight[1] = curLight[2] = 255;
			glActiveTexture(GL_TEXTURE1);
			glBindTexture (GL_TEXTURE_2D, lightMap.name);
		}
	} else {
		curLight[0] = curLight[1] = curLight[2] = 255;
	}
	glActiveTexture(GL_TEXTURE2);
	glBindTexture (GL_TEXTURE_2D, backdropTextureName);
	glActiveTexture(GL_TEXTURE0);

	float ltx1, btx1;
	float ltx2, btx2;
	float lty1, bty1;
	float lty2, bty2;
	if (! NPOT_textures) {
		ltx1 = lightMap.texW * x1 / sceneWidth;
		ltx2 = lightMap.texW * (x1+spriteWidth) / sceneWidth;
		lty1 = lightMap.texH * y1 / sceneHeight;
		lty2 = lightMap.texH * (y1+spriteHeight) / sceneHeight;
		btx1 = backdropTexW * x1 / sceneWidth;
		btx2 = backdropTexW * (x1+spriteWidth) / sceneWidth;
		bty1 = backdropTexH * y1 / sceneHeight;
		bty2 = backdropTexH * (y1+spriteHeight) / sceneHeight;
	} else {
		btx1 = ltx1 = (float) x1 / sceneWidth;
		btx2 = ltx2 = (float) (x1+spriteWidth) / sceneWidth;
		bty1 = lty1 = (float) y1 / sceneHeight;
		bty2 = lty2 = (float) (y1+spriteHeight) / sceneHeight;
	}
	
	setPixelCoords (true);
	int xoffset = 0;
	while (xoffset < diffX) {
		int w = (diffX-xoffset < viewportWidth) ? (int) (diffX-xoffset) : viewportWidth;

		int yoffset = 0;
		while (yoffset < diffY) {
			int h = (diffY-yoffset< viewportHeight) ? (int) (diffY-yoffset) : viewportHeight;

			// Render the scene - first the old backdrop (so that it'll show through when the z-buffer is active
			glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glBindTexture (GL_TEXTURE_2D, backdropTextureName);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex3f(-x1-xoffset, -y1-yoffset, 0.0);
			glTexCoord2f(backdropTexW, 0.0); glVertex3f(sceneWidth-x1-xoffset, -y1-yoffset, 0.0);
			glTexCoord2f(backdropTexW, backdropTexH); glVertex3f(sceneWidth-x1-xoffset, sceneHeight-y1-yoffset, 0.0);
			glTexCoord2f(0.0, backdropTexH); glVertex3f(-x1-xoffset, sceneHeight-y1-yoffset, 0.0);
			glEnd();

			// The z-buffer
			if (useZB) {
				glDepthMask (GL_TRUE);
				glClear(GL_DEPTH_BUFFER_BIT);
				drawZBuffer((int) (x1+xoffset+camX), (int) (y1+yoffset+camY), false);

				glDepthMask (GL_FALSE);
				glEnable(GL_DEPTH_TEST);
			}

			// Then the sprite
			glUseProgram(shader.paste);
			GLint uniform = glGetUniformLocation(shader.paste, "useLightTexture");
			if (uniform >= 0) glUniform1i(uniform, light && lightMapMode == LIGHTMAPMODE_PIXEL && lightMap.data);
			setDrawMode (thisPerson);

			glBindTexture (GL_TEXTURE_2D, fontPal.tex_names[single.texNum]);

			glBegin(GL_QUADS);
			if (! mirror) {
				glTexCoord2f(tx1, ty1); glMultiTexCoord2f(GL_TEXTURE1, ltx1, lty1); glMultiTexCoord2f(GL_TEXTURE2, btx1, bty1); 	glVertex3f(-xoffset, -yoffset, z);
				glTexCoord2f(tx2, ty1); glMultiTexCoord2f(GL_TEXTURE1, ltx2, lty1); glMultiTexCoord2f(GL_TEXTURE2, btx2, bty1); 	glVertex3f(spriteWidth-xoffset, -yoffset, z);
				glTexCoord2f(tx2, ty2); glMultiTexCoord2f(GL_TEXTURE1, ltx2, lty2); glMultiTexCoord2f(GL_TEXTURE2, btx2, bty2); 	glVertex3f(spriteWidth-xoffset, spriteHeight-yoffset, z);
				glTexCoord2f(tx1, ty2); glMultiTexCoord2f(GL_TEXTURE1, ltx1, lty2); glMultiTexCoord2f(GL_TEXTURE2, btx1, bty2); 	glVertex3f(-xoffset, spriteHeight-yoffset, z);
			} else {
				glTexCoord2f(tx2, ty1); glMultiTexCoord2f(GL_TEXTURE1, ltx1, lty1); glMultiTexCoord2f(GL_TEXTURE2, btx1, bty1); 	glVertex3f(-xoffset, -yoffset, z);
				glTexCoord2f(tx1, ty1); glMultiTexCoord2f(GL_TEXTURE1, ltx2, lty1); glMultiTexCoord2f(GL_TEXTURE2, btx2, bty1); 	glVertex3f(spriteWidth-xoffset, -yoffset, z);
				glTexCoord2f(tx1, ty2); glMultiTexCoord2f(GL_TEXTURE1, ltx2, lty2); glMultiTexCoord2f(GL_TEXTURE2, btx2, bty2); 	glVertex3f(spriteWidth-xoffset, spriteHeight-yoffset, z);
				glTexCoord2f(tx2, ty2); glMultiTexCoord2f(GL_TEXTURE1, ltx1, lty2); glMultiTexCoord2f(GL_TEXTURE2, btx1, bty2); 	glVertex3f(-xoffset, spriteHeight-yoffset, z);
			}
			glEnd();

			glSecondaryColor3ub (0, 0, 0);
			glDisable(GL_COLOR_SUM);

			// Copy Our ViewPort To The Texture
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glUseProgram(0);

			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, (int) ((x1<0) ? xoffset: x1+xoffset), (int) ((y1<0) ? yoffset: y1+yoffset), (int) ((x1<0) ?viewportOffsetX-x1:viewportOffsetX), (int) ((y1<0) ?viewportOffsetY-y1:viewportOffsetY), w, h);

			yoffset += viewportHeight;
		}
		xoffset += viewportWidth;
	}

	setPixelCoords (false);
	glUseProgram(0);
}

