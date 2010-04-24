#if defined __unix__ && !(defined __APPLE__)
#include <png.h>
#else
#include <libpng/png.h>
#endif

#include <stdlib.h>
#include <stdint.h>

#include "GLee.h"

#include "allfiles.h"

#include "newfatal.h"
#include "colours.h"
#include "fileset.h"
#include "cursors.h"
#include "backdrop.h"
#include "moreio.h"
#include "variable.h"
#include "zbuffer.h"
#include "graphics.h"
#include "line.h"
#include "sprites_aa.h"

#include "version.h"

bool freeze ();
void unfreeze (bool);	// Because FREEZE.H needs a load of other includes

GLubyte * backdropTexture = NULL;
GLuint backdropTextureName = 0;
bool backdropExists = false;
double backdropTexW = 1.0;
double backdropTexH = 1.0;

texture lightMap;

GLuint snapshotTextureName = 0;

int lightMapMode = LIGHTMAPMODE_PIXEL;

parallaxLayer * parallaxStuff = NULL;

int cameraPX = 0, cameraPY = 0;

int sceneWidth, sceneHeight;
int lightMapNumber;
unsigned int currentBlankColour = makeColour (0, 0, 0);

extern int cameraX, cameraY;

void nosnapshot () {
	glDeleteTextures (1, &snapshotTextureName);
	snapshotTextureName = 0;
}


bool snapshot () {
	nosnapshot ();
	if (! freeze ()) return false;
	snapshotTextureName = backdropTextureName;
	unfreeze (false);
	return true;
}

bool restoreSnapshot (FILE * fp) {
	int realPicWidth, realPicHeight;
	int picWidth = realPicWidth = get2bytes (fp);
	int picHeight = realPicHeight = get2bytes (fp);


	int t1, t2, n;
	unsigned short c;

	GLubyte * target;
	if (! NPOT_textures) {
		picWidth = getNextPOT(picWidth);
		picHeight = getNextPOT(picHeight);
	}
	GLubyte * snapshotTexture = new GLubyte [picHeight*picWidth*4];

	for (t2 = 0; t2 < realPicHeight; t2 ++) {
		t1 = 0;
		while (t1 < realPicWidth) {
			c = (unsigned short) get2bytes (fp);
			if (c & 32) {
				n = fgetc (fp) + 1;
				c -= 32;
			} else {
				n = 1;
			}
			while (n --) {
				target = snapshotTexture + 4*picWidth*t2 + t1*4;
				target[0] = (GLubyte) redValue(c);
				target[1] = (GLubyte) greenValue(c);
				target[2] = (GLubyte) blueValue(c);
				target[3] = (GLubyte) 255;
				t1++;
			}
		}
	}

	if (! snapshotTextureName) glGenTextures (1, &snapshotTextureName);
	glBindTexture(GL_TEXTURE_2D, snapshotTextureName);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, picWidth, picHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, snapshotTexture);

	delete snapshotTexture;
	snapshotTexture = NULL;

	return true;
}


void killBackDrop () {
	glDeleteTextures (1, &backdropTextureName);
	backdropTextureName = 0;
	backdropExists = false;
}

void killLightMap () {
	glDeleteTextures (1, &lightMap.name);
	lightMap.name = 0;
	if (lightMap.data) {
		delete lightMap.data;
		lightMap.data = NULL;
	}
	lightMapNumber = 0;
}

void killParallax () {
	while (parallaxStuff) {

		parallaxLayer * k = parallaxStuff;
		parallaxStuff = k -> next;

		// Now kill the image
		glDeleteTextures (1, &k -> textureName);
		delete k -> texture;
		delete k;
		k = NULL;
	}
}

bool reserveBackdrop () {
	cameraX = 0;
	cameraY = 0;
	int picWidth = sceneWidth;
	int picHeight = sceneHeight;

	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

	if (backdropTexture) delete backdropTexture;
	if (! NPOT_textures) {
		picWidth = getNextPOT(sceneWidth);
		picHeight = getNextPOT(sceneHeight);
		backdropTexW = ((double)sceneWidth) / picWidth;
		backdropTexH = ((double)sceneHeight) / picHeight;
	}
	backdropTexture = new GLubyte [picWidth*picHeight*4];

	if (! backdropTextureName) glGenTextures (1, &backdropTextureName);
	glBindTexture (GL_TEXTURE_2D, backdropTextureName);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if (maxAntiAliasSettings.useMe) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //GL_NEAREST
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, picWidth, picHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, backdropTexture);

	return true;
}

bool resizeBackdrop (int x, int y) {
	killLightMap ();
	killBackDrop ();
	killParallax ();
	killZBuffer ();
	sceneWidth = x;
	sceneHeight = y;
	return reserveBackdrop ();
}

void loadBackDrop (int fileNum, int x, int y) {
	setResourceForFatal (fileNum);

	if (! openFileFromNum (fileNum)) {
		fatal ("Can't load overlay image");
		return;
	}

	if (! loadHSI (bigDataFile, x, y, false)) {
		char mess[200];
		sprintf (mess, "Can't paste overlay image outside scene dimensions\n\nX = %i\nY = %i\nWidth = %i\nHeight = %i", x, y, sceneWidth, sceneHeight);
		fatal (mess);
	}

	finishAccess ();
	setResourceForFatal (-1);
}

void mixBackDrop (int fileNum, int x, int y) {
	setResourceForFatal (fileNum);
	if (! openFileFromNum (fileNum)) {
		fatal ("Can't load overlay image");
		return;
	}

	if (! mixHSI (bigDataFile, x, y)) {
		fatal ("Can't paste overlay image outside screen dimensions");
	}

	finishAccess ();
	setResourceForFatal (-1);
}

void blankScreen (int x1, int y1, int x2, int y2) {

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (x2 > sceneWidth) x2 = sceneWidth;
	if (y2 > sceneHeight) y2 = sceneHeight;

	int picWidth = x2-x1;
	int picHeight = y2-y1;

	setPixelCoords (true);

	int xoffset = 0;
	while (xoffset < picWidth) {
		int w = (picWidth-xoffset < viewportWidth) ? picWidth-xoffset : viewportWidth;

		int yoffset = 0;
		while (yoffset < picHeight) {
			int h = (picHeight-yoffset < viewportHeight) ? picHeight-yoffset : viewportHeight;

			// Render the scene
			glColor3ub(redValue(currentBlankColour), greenValue(currentBlankColour), blueValue(currentBlankColour));

			glDisable (GL_TEXTURE_2D);
			glBegin(GL_QUADS);
			glVertex3f(-10.325, -1.325, 0.0);
			glVertex3f(w+1.325, -1.325, 0.0);
			glVertex3f(w+1.325, h+1.325, 0.0);
			glVertex3f(-10.325, h+1.325, 0.0);
			glEnd();

			// Copy Our ViewPort To The Texture
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, x1+xoffset, y1+yoffset, viewportOffsetX, viewportOffsetY, w, h);

			yoffset += viewportHeight;
		}
		xoffset += viewportWidth;
	}
	setPixelCoords (false);
}

void hardScroll (int distance) {
	if (abs (distance) >= sceneHeight) {
		blankScreen (0, 0, sceneWidth, sceneHeight);
		return;
	}

	if (! distance) return;

	setPixelCoords (true);

	int xoffset = 0;
	while (xoffset < sceneWidth) {
		int w = (sceneWidth-xoffset < viewportWidth) ? sceneWidth-xoffset : viewportWidth;

		int yoffset = 0;
		while (yoffset < sceneHeight) {
			int h = (sceneHeight-yoffset < viewportHeight) ? sceneHeight-yoffset : viewportHeight;

			glClear(GL_COLOR_BUFFER_BIT);	// Clear The Screen

			// Render the backdrop
			glBindTexture (GL_TEXTURE_2D, backdropTextureName);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			glEnable (GL_TEXTURE_2D);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex3f(-xoffset, -distance-yoffset, 0.0);
			glTexCoord2f(backdropTexW, 0.0); glVertex3f(sceneWidth-xoffset, -distance-yoffset, 0.0);
			glTexCoord2f(backdropTexW, backdropTexH); glVertex3f(sceneWidth-xoffset, sceneHeight-distance-yoffset, 0.0);
			glTexCoord2f(0.0, backdropTexH); glVertex3f(-xoffset, sceneHeight-distance-yoffset, 0.0);
			glEnd();


			// Copy Our ViewPort To The Texture
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, viewportOffsetX, viewportOffsetY, w, h);

			yoffset += viewportHeight;
		}
		xoffset += viewportWidth;
	}
	glDisable(GL_TEXTURE_2D);
	setPixelCoords (false);
	if (maxAntiAliasSettings.useMe) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
}

void drawVerticalLine (unsigned int x, unsigned int y1, unsigned int y2) {
	drawLine (x, y1, x, y2);
}

void drawHorizontalLine (unsigned int x1, unsigned int y, unsigned int x2) {
	drawLine (x1, y, x2, y);
}

void darkScreen () {
	setPixelCoords (true);

	int xoffset = 0;
	while (xoffset < sceneWidth) {
		int w = (sceneWidth-xoffset < viewportWidth) ? sceneWidth-xoffset : viewportWidth;

		int yoffset = 0;
		while (yoffset < sceneHeight) {
			int h = (sceneHeight-yoffset < viewportHeight) ? sceneHeight-yoffset : viewportHeight;

			// Render the scene - first the old backdrop
			glEnable (GL_TEXTURE_2D);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glBindTexture (GL_TEXTURE_2D, backdropTextureName);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex3f(-xoffset, -yoffset, 0.0);
			glTexCoord2f(backdropTexW, 0.0); glVertex3f(sceneWidth-xoffset, -yoffset, 0.0);
			glTexCoord2f(backdropTexW, backdropTexH); glVertex3f(sceneWidth-xoffset, sceneHeight-yoffset, 0.0);
			glTexCoord2f(0.0, backdropTexH); glVertex3f(-xoffset, sceneHeight-yoffset, 0.0);
			glEnd();


			// Then the darkness
			glDisable (GL_TEXTURE_2D);
			glColor4ub (0, 0, 0, 127);
			glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glBegin(GL_QUADS);
			glVertex3f(-xoffset, -yoffset, 0.0);
			glVertex3f(sceneWidth-xoffset, -yoffset, 0.0);
			glVertex3f(sceneWidth-xoffset, sceneHeight-yoffset, 0.0);
			glVertex3f(-xoffset, sceneHeight-yoffset, 0.0);
			glEnd();
			glDisable(GL_BLEND);
			glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

			// Copy Our ViewPort To The Texture
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, viewportOffsetX, viewportOffsetY, w, h);

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

inline int sortOutPCamera (int cX, int fX, int sceneMax, int boxMax) {
	return (fX == 65535) ?
		(sceneMax ? ((cX * boxMax) / sceneMax) : 0)
	:
		((cX * fX) / 100);
}


/*
/ copyToBackdrop
/ This function is used to copy the old backdrop when doing a freeze
*/
void copyToBackDrop (GLuint fromHere, int orW, int orH, int orX, int orY, parallaxLayer * parallaxS) {

	setPixelCoords (true);

	glColor4f(1.0, 1.0, 1.0, 1.0);
	glEnable (GL_TEXTURE_2D);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen

	int xoffset = 0;

	while (xoffset < winWidth) {
		int w = (winWidth-xoffset < viewportWidth) ? winWidth-xoffset : viewportWidth;

		int yoffset = 0;
		while (yoffset < orH) {
			int h = (winHeight-yoffset < viewportHeight) ? winHeight-yoffset : viewportHeight;

			if (parallaxS) {
				parallaxLayer * ps = parallaxS;

				while (ps->next) ps = ps->next;

				while (ps) {
					ps -> cameraX = sortOutPCamera (orX, ps -> fractionX, orW - w, ps -> width - winWidth);
					ps -> cameraY = sortOutPCamera (orY, ps -> fractionY, orH - h, ps -> height - winHeight);

					glBindTexture (GL_TEXTURE_2D, ps->textureName);
					glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glBegin(GL_QUADS);

					float texw = (ps->wrapS) ? (float) orW / ps->width: 1.0;
					float wt = (ps->wrapS) ? orW : ps->width;
					float texh = (ps->wrapT) ? (float) orH / ps->height: 1.0;
					float ht = (ps->wrapT) ? orH : ps->height;

					glTexCoord2f(0.0, 0.0); glVertex3f(-ps -> cameraX-xoffset, -ps -> cameraY-yoffset, 0.1);
					glTexCoord2f(texw, 0.0); glVertex3f(wt -ps -> cameraX-xoffset, -ps -> cameraY-yoffset, 0.1);
					glTexCoord2f(texw, texh); glVertex3f(wt -ps -> cameraX-xoffset, ht -ps -> cameraY-yoffset, 0.1);
					glTexCoord2f(0.0, texh); glVertex3f(-ps -> cameraX-xoffset, ht -ps -> cameraY-yoffset, 0.1);

					glEnd();

					ps = ps -> prev;
				}
			}


			// Render the backdrop
			glBindTexture (GL_TEXTURE_2D, fromHere);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glColor4f(1.0, 1.0, 1.0, 1.0);

			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex3f(-orX-xoffset, -orY-yoffset, 0);
			glTexCoord2f(backdropTexW, 0.0); glVertex3f(orW-orX-xoffset, -orY-yoffset, 0);
			glTexCoord2f(backdropTexW, backdropTexH); glVertex3f(orW-orX-xoffset, orH-orY-yoffset, 0);
			glTexCoord2f(0.0, backdropTexH); glVertex3f(-orX-xoffset, orH-orY-yoffset, 0);
			glEnd();

			if (maxAntiAliasSettings.useMe) {
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //GL_NEAREST
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			} else {
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			}

			// Copy Our ViewPort To The Texture
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0+xoffset, 0+yoffset, viewportOffsetX, viewportOffsetY, w, h);

			yoffset += viewportHeight;
		}
		xoffset += viewportWidth;
	}


	setPixelCoords(false);
	backdropExists = true;
}




void drawBackDrop () {

	glColor4f(1.0, 1.0, 1.0, 1.0);
	glEnable (GL_TEXTURE_2D);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (parallaxStuff) {
		parallaxLayer * ps = parallaxStuff;

		while (ps->next) ps = ps->next;

		while (ps) {
			ps -> cameraX = sortOutPCamera (cameraX, ps -> fractionX, sceneWidth - winWidth, ps -> width - winWidth);
			ps -> cameraY = sortOutPCamera (cameraY, ps -> fractionY, sceneHeight - winHeight, ps -> height - winHeight);

			glBindTexture (GL_TEXTURE_2D, ps->textureName);
			glBegin(GL_QUADS);

			float w = (ps->wrapS) ? sceneWidth : ps->width;
			float h = (ps->wrapT) ? sceneHeight : ps->height;
			float texw;
			float texh;
			if (! NPOT_textures) {
				texw = (ps->wrapS) ? (float) sceneWidth / ps->width: (float) ps->width / getNextPOT(ps->width);
				texh = (ps->wrapT) ? (float) sceneHeight / ps->height: (float) ps->height / getNextPOT(ps->height);
			} else {
				texw = (ps->wrapS) ? (float) sceneWidth / ps->width: 1.0;
				texh = (ps->wrapT) ? (float) sceneHeight / ps->height: 1.0;
			}

			glTexCoord2f(0.0, 0.0); glVertex3f(-ps -> cameraX, -ps -> cameraY, 0.1);
			glTexCoord2f(texw, 0.0); glVertex3f(w-1 -ps -> cameraX, -ps -> cameraY, 0.1);
			glTexCoord2f(texw, texh); glVertex3f(w-1 -ps -> cameraX, h -1 -ps -> cameraY, 0.1);
			glTexCoord2f(0.0, texh); glVertex3f(-ps -> cameraX, h -1 -ps -> cameraY, 0.1);

			glEnd();

			ps = ps -> prev;
		}
	}

	glBindTexture (GL_TEXTURE_2D, backdropTextureName);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-cameraX, -cameraY, 0.0);
	glTexCoord2f(backdropTexW, 0.0); glVertex3f(sceneWidth-cameraX, -cameraY, 0.0);
	glTexCoord2f(backdropTexW, backdropTexH); glVertex3f(sceneWidth-cameraX, sceneHeight-cameraY, 0.0);
	glTexCoord2f(0.0, backdropTexH); glVertex3f(-cameraX, sceneHeight-cameraY, 0.0);
	glEnd();
	glDisable(GL_BLEND);

}

bool loadLightMap (int v) {
	int newPicWidth, newPicHeight;

	setResourceForFatal (v);
	if (! openFileFromNum (v)) return fatal ("Can't open light map.");

	long file_pointer = ftell (bigDataFile);

	png_structp png_ptr;
	png_infop info_ptr, end_info;


	int fileIsPNG = true;

	// Is this a PNG file?

	char tmp[10];
	fread(tmp, 1, 8, bigDataFile);
    if (png_sig_cmp((png_byte *) tmp, 0, 8)) {
		// No, it's old-school HSI
		fileIsPNG = false;
		fseek(bigDataFile, file_pointer, SEEK_SET);

		newPicWidth = lightMap.w = get2bytes (bigDataFile);
		newPicHeight = lightMap.h = get2bytes (bigDataFile);
	} else {
		// Read the PNG header

		png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png_ptr) {
			return false;
		}

		info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr) {
			png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
			return false;
		}

		end_info = png_create_info_struct(png_ptr);
		if (!end_info) {
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			return false;
		}
		png_init_io(png_ptr, bigDataFile);		// Tell libpng which file to read
		png_set_sig_bytes(png_ptr, 8);	// 8 bytes already read

		png_read_info(png_ptr, info_ptr);

		png_uint_32 width, height;
		int bit_depth, color_type, interlace_type, compression_type, filter_method;
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

		newPicWidth = lightMap.w = width;
		newPicHeight = lightMap.h = height;

		if (bit_depth < 8) png_set_packing(png_ptr);
		png_set_expand(png_ptr);
		if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png_ptr);
		if (bit_depth == 16) png_set_strip_16(png_ptr);

		png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);

		png_read_update_info(png_ptr, info_ptr);
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

		//int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	}



	if (lightMapMode == LIGHTMAPMODE_HOTSPOT) {
		if (lightMap.w != sceneWidth || lightMap.h != sceneHeight) {
			return fatal ("Light map width and height don't match scene width and height. That is required for lightmaps in HOTSPOT mode.");
		}
	}
	if (! NPOT_textures) {
		newPicWidth = getNextPOT(lightMap.w);
		newPicHeight = getNextPOT(lightMap.h);
		lightMap.texW = (double) lightMap.w / newPicWidth;
		lightMap.texH = (double) lightMap.h / newPicHeight;
	} else {
		lightMap.texW = 1.0;
		lightMap.texH = 1.0;
	}

	killLightMap ();
	lightMapNumber = v;

	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

	if (lightMap.data) delete lightMap.data;

	lightMap.data = new GLubyte [newPicWidth*newPicHeight*4];
	if (! lightMap.data) {
		return fatal ("Out of memory loading light map.");
	}

	int t1, t2, n;
	unsigned short c;
	GLubyte * target;

	if (fileIsPNG) {
		unsigned char * row_pointers[lightMap.h];
		for (int i = 0; i<lightMap.h; i++)
			row_pointers[i] = lightMap.data + 4*i*newPicWidth;

		png_read_image(png_ptr, (png_byte **) row_pointers);
		png_read_end(png_ptr, NULL);
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	} else {

		for (t2 = 0; t2 < lightMap.h; t2 ++) {
			t1 = 0;
			while (t1 < lightMap.w) {
				c = (unsigned short) get2bytes (bigDataFile);
				if (c & 32) {
					n = fgetc (bigDataFile) + 1;
					c -= 32;
				} else {
					n = 1;
				}
				while (n --) {
					target = lightMap.data + 4*newPicWidth*t2 + t1*4;
					target[0] = (GLubyte) redValue(c);
					target[1] = (GLubyte) greenValue(c);
					target[2] = (GLubyte) blueValue(c);
					target[3] = (GLubyte) 255;
					t1++;
				}
			}
		}
	}

	if (! lightMap.name) glGenTextures (1, &lightMap.name);
	glBindTexture(GL_TEXTURE_2D, lightMap.name);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, newPicWidth, newPicHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, lightMap.data);

	finishAccess ();
	setResourceForFatal (-1);

	return true;
}

void reloadParallaxTextures () {
	parallaxLayer * nP = parallaxStuff;
	if (! nP) return;

	while (nP) {
		//fprintf (stderr, "Reloading parallax. (%d, %d) ", nP->width, nP->height);
		nP->textureName = 0;
		glGenTextures (1, &nP->textureName);
		glBindTexture (GL_TEXTURE_2D, nP->textureName);
		if (nP -> wrapS)
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		else
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		if (nP -> wrapT)
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		else
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		if (maxAntiAliasSettings.useMe) {
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		} else {
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		if (! NPOT_textures) {
			glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, getNextPOT(nP->width), getNextPOT(nP->height), 0, GL_RGBA, GL_UNSIGNED_BYTE, nP->texture);
		} else {
			glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, nP->width, nP->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nP->texture);
		}
		nP = nP->next;
	}
}

bool loadParallax (unsigned short v, unsigned short fracX, unsigned short fracY) {
	setResourceForFatal (v);
	if (! openFileFromNum (v)) return fatal ("Can't open parallax image");

	parallaxLayer * nP = new parallaxLayer;
	if (! checkNew (nP)) return false;

	nP -> next = parallaxStuff;
	parallaxStuff = nP;
	if (nP -> next) {
		nP -> next -> prev = nP;
	}
	nP -> prev = NULL;

	int picWidth;
	int picHeight;
	
	long file_pointer = ftell (bigDataFile);
	
	png_structp png_ptr;
	png_infop info_ptr, end_info;
	
	
	int fileIsPNG = true;
	
	// Is this a PNG file?
	
	char tmp[10];
	fread(tmp, 1, 8, bigDataFile);
    if (png_sig_cmp((png_byte *) tmp, 0, 8)) {
		// No, it's old-school HSI
		fileIsPNG = false;
		fseek(bigDataFile, file_pointer, SEEK_SET);
		
		picWidth = nP -> width = get2bytes (bigDataFile);
		picHeight = nP -> height = get2bytes (bigDataFile);
	} else {
		// Read the PNG header
		
		png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png_ptr) {
			return false;
		}
		
		info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr) {
			png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
			return false;
		}
		
		end_info = png_create_info_struct(png_ptr);
		if (!end_info) {
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			return false;
		}
		png_init_io(png_ptr, bigDataFile);		// Tell libpng which file to read
		png_set_sig_bytes(png_ptr, 8);	// 8 bytes already read
		
		png_read_info(png_ptr, info_ptr);
		
		png_uint_32 width, height;
		int bit_depth, color_type, interlace_type, compression_type, filter_method;
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);
		
		picWidth = nP -> width = width;
		picHeight = nP -> height = height;
		
		if (bit_depth < 8) png_set_packing(png_ptr);
		png_set_expand(png_ptr);
		if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png_ptr);
		if (bit_depth == 16) png_set_strip_16(png_ptr);
		
		png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
		
		png_read_update_info(png_ptr, info_ptr);
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);
		
		//int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
		
	}
	
	if (! NPOT_textures) {
		picWidth = getNextPOT(picWidth);
		picHeight = getNextPOT(picHeight);
	}
	
	nP -> fileNum = v;
	nP -> fractionX = fracX;
	nP -> fractionY = fracY;

	if (fracX == 65535) {
		nP -> wrapS = false;
		if (nP -> width < winWidth) {
			fatal ("For AUTOFIT parallax backgrounds, the image must be at least as wide as the game window/screen.");
			return false;
		}
	} else {
		nP -> wrapS = true;
	}

	if (fracY == 65535) {
		nP -> wrapT = false;
		if (nP -> height < winHeight) {
			fatal ("For AUTOFIT parallax backgrounds, the image must be at least as tall as the game window/screen.");
			return false;
		}
	} else {
		nP -> wrapT = true;
	}

	nP -> texture = new GLubyte [picHeight * picWidth * 4];

	if (! checkNew (nP -> texture)) return false;

	if (fileIsPNG) {
		unsigned char * row_pointers[nP -> height];
		for (int i = 0; i < nP -> height; i++)
			row_pointers[i] = nP -> texture + 4*i*picWidth;
		
		png_read_image(png_ptr, (png_byte **) row_pointers);
		png_read_end(png_ptr, NULL);
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	} else {
			
		int t1, t2, n;
		unsigned short c;
		GLubyte * target;

		for (t2 = 0; t2 < nP -> height; t2 ++) {
			t1 = 0;
			while (t1 < nP -> width) {
				c = (unsigned short) get2bytes (bigDataFile);
				if (c & 32) {
					n = fgetc (bigDataFile) + 1;
					c -= 32;
				} else {
					n = 1;
				}
				while (n--) {
					target = nP -> texture + 4*picWidth*t2 + t1*4;
					if (c == 63519 || c == 2015) {
						target[0] = (GLubyte) 0;
						target[1] = (GLubyte) 0;
						target[2] = (GLubyte) 0;
						target[3] = (GLubyte) 0;
					} else {
						target[0] = (GLubyte) redValue(c);
						target[1] = (GLubyte) greenValue(c);
						target[2] = (GLubyte) blueValue(c);
						target[3] = (GLubyte) 255;
					}
					t1 ++;
				}
			}
		}
	}

	glGenTextures (1, &nP->textureName);
	glBindTexture (GL_TEXTURE_2D, nP->textureName);
	if (nP -> wrapS)
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	else
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	if (nP -> wrapT)
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	else
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if (maxAntiAliasSettings.useMe) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, picWidth, picHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nP->texture);

	finishAccess ();
	setResourceForFatal (-1);
	return true;
}

extern int viewportOffsetX, viewportOffsetY;


bool loadHSI (FILE * fp, int x, int y, bool reserve) {

	int t1, t2, n;
	unsigned short c;
	GLubyte * target;
	int32_t transCol = reserve ? -1 : 63519;
	int picWidth;
	int picHeight;
	int realPicWidth, realPicHeight;
	long file_pointer = ftell (fp);

	png_structp png_ptr;
	png_infop info_ptr, end_info;


	int fileIsPNG = true;

	// Is this a PNG file?

	char tmp[10];
	fread(tmp, 1, 8, fp);
    if (png_sig_cmp((png_byte *) tmp, 0, 8)) {
		// No, it's old-school HSI
		fileIsPNG = false;
		fseek(fp, file_pointer, SEEK_SET);

		picWidth = realPicWidth = get2bytes (fp);
		picHeight = realPicHeight = get2bytes (fp);
	} else {
		// Read the PNG header

		png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png_ptr) {
			return false;
		}

		info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr) {
			png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
			return false;
		}

		end_info = png_create_info_struct(png_ptr);
		if (!end_info) {
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			return false;
		}
		png_init_io(png_ptr, fp);		// Tell libpng which file to read
		png_set_sig_bytes(png_ptr, 8);	// 8 bytes already read

		png_read_info(png_ptr, info_ptr);

		png_uint_32 width, height;
		int bit_depth, color_type, interlace_type, compression_type, filter_method;
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

		picWidth = realPicWidth = width;
		picHeight = realPicHeight = height;

		if (bit_depth < 8) png_set_packing(png_ptr);
		png_set_expand(png_ptr);
		if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png_ptr);
		if (bit_depth == 16) png_set_strip_16(png_ptr);

		png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);

		png_read_update_info(png_ptr, info_ptr);
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

		//int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	}


	double texCoordW = 1.0;
	double texCoordH = 1.0;
	if (! NPOT_textures) {
		picWidth = getNextPOT(picWidth);
		picHeight = getNextPOT(picHeight);
		texCoordW = ((double)realPicWidth) / picWidth;
		texCoordH = ((double)realPicHeight) / picHeight;
	}

	if (reserve) {
		if (! resizeBackdrop (realPicWidth, realPicHeight)) return false;
	}

	if (x == IN_THE_CENTRE) x = (sceneWidth - realPicWidth) >> 1;
	if (y == IN_THE_CENTRE) y = (sceneHeight - realPicHeight) >> 1;
	if (x < 0 || x + realPicWidth > sceneWidth || y < 0 || y + realPicHeight > sceneHeight) return false;

	if (fileIsPNG) {
		unsigned char * row_pointers[realPicHeight];
		for (int i = 0; i<realPicHeight; i++)
			row_pointers[i] = backdropTexture + 4*i*picWidth;

		png_read_image(png_ptr, (png_byte **) row_pointers);
		png_read_end(png_ptr, NULL);
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	} else {
		for (t2 = 0; t2 < realPicHeight; t2 ++) {
			t1 = 0;
			while (t1 < realPicWidth) {
				c = (unsigned short) get2bytes (fp);
				if (c & 32) {
					n = fgetc (fp) + 1;
					c -= 32;
				} else {
					n = 1;
				}
				while (n --) {
					target = backdropTexture + 4*picWidth*t2 + t1*4;
					if (c == transCol || c == 2015) {
						target[0] = (GLubyte) 0;
						target[1] = (GLubyte) 0;
						target[2] = (GLubyte) 0;
						target[3] = (GLubyte) 0;
					} else {
						target[0] = (GLubyte) redValue(c);
						target[1] = (GLubyte) greenValue(c);
						target[2] = (GLubyte) blueValue(c);
						target[3] = (GLubyte) 255;
					}
					t1++;
				}
			}
		}
	}

	GLuint tmpTex;

	glGenTextures (1, &tmpTex);
	glBindTexture(GL_TEXTURE_2D, tmpTex);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, picWidth, picHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, backdropTexture);

	glEnable (GL_TEXTURE_2D);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	
	float btx1;
	float btx2;
	float bty1;
	float bty2;
	if (! NPOT_textures) {
		btx1 = backdropTexW * x / sceneWidth;
		btx2 = backdropTexW * (x+realPicWidth) / sceneWidth;
		bty1 = backdropTexH * y / sceneHeight;
		bty2 = backdropTexH * (y+realPicHeight) / sceneHeight;
	} else {
		btx1 = (float) x / sceneWidth;
		btx2 = (float) (x+realPicWidth) / sceneWidth;
		bty1 = (float) y / sceneHeight;
		bty2 = (float) (y+realPicHeight) / sceneHeight;
	}
	
	
	setPixelCoords (true);

	int xoffset = 0;
	while (xoffset < realPicWidth) {
		int w = (realPicWidth-xoffset < viewportWidth) ? realPicWidth-xoffset : viewportWidth;

		int yoffset = 0;
		while (yoffset < realPicHeight) {
			int h = (realPicHeight-yoffset < viewportHeight) ? realPicHeight-yoffset : viewportHeight;

			glClear(GL_COLOR_BUFFER_BIT);	// Clear The Screen

			if (backdropExists) {
				// Render the sprite to the backdrop 
				// (using mulitexturing, so the old backdrop is seen where alpha < 1.0)
				glActiveTexture(GL_TEXTURE2);
				glBindTexture (GL_TEXTURE_2D, backdropTextureName);
				glActiveTexture(GL_TEXTURE0);
				
				glUseProgram(shader.fixScaleSprite);
				GLint uniform = glGetUniformLocation(shader.fixScaleSprite, "useLightTexture");
				if (uniform >= 0) glUniform1i(uniform, 0); // No lighting
				
				glColor4f(1.0, 1.0, 1.0, 1.0);
				glBindTexture(GL_TEXTURE_2D, tmpTex);
				glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				
				glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0); glMultiTexCoord2f(GL_TEXTURE2, btx1, bty1); 	glVertex3f(-xoffset, -yoffset, 0.0);
				glTexCoord2f(texCoordW, 0.0); glMultiTexCoord2f(GL_TEXTURE2, btx2, bty1); 	glVertex3f(realPicWidth-xoffset, -yoffset, 0.0);
				glTexCoord2f(texCoordW, texCoordH); glMultiTexCoord2f(GL_TEXTURE2, btx2, bty2); 	glVertex3f(realPicWidth-xoffset, realPicHeight-yoffset, 0.0);
				glTexCoord2f(0.0, texCoordH); glMultiTexCoord2f(GL_TEXTURE2, btx1, bty2); 	glVertex3f(-xoffset, realPicHeight-yoffset, 0.0);
				glEnd();
				
				glUseProgram(0);
				
			} else {
				// It's all new - nothing special to be done.

				glBindTexture(GL_TEXTURE_2D, tmpTex);

				glColor4f(1.0, 0.0, 0.0, 0.0);
				glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0); glVertex3f(-xoffset, -yoffset, 0.0);
				glTexCoord2f(texCoordW, 0.0); glVertex3f(realPicWidth-xoffset, -yoffset, 0.0);
				glTexCoord2f(texCoordW, texCoordH); glVertex3f(realPicWidth-xoffset, -yoffset+realPicHeight, 0.0);
				glTexCoord2f(0.0, texCoordH); glVertex3f(-xoffset, -yoffset+realPicHeight, 0.0);
				glEnd();
				
			}

			// Copy Our ViewPort To The Texture
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, x+xoffset, y+yoffset, viewportOffsetX, viewportOffsetY, w, h);

			yoffset += viewportHeight;
		}

		xoffset += viewportWidth;
	}
	glDeleteTextures(1, &tmpTex);

	setPixelCoords (false);
	if (maxAntiAliasSettings.useMe) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	backdropExists = true;
	return true;
}

bool mixHSI (FILE * fp, int x, int y) {
	int realPicWidth, realPicHeight;
	int picWidth;
	int picHeight;

	long file_pointer = ftell (fp);

	png_structp png_ptr;
	png_infop info_ptr, end_info;


	int fileIsPNG = true;

	// Is this a PNG file?

	char tmp[10];
	fread(tmp, 1, 8, fp);
    if (png_sig_cmp((png_byte *) tmp, 0, 8)) {
		// No, it's old-school HSI
		fileIsPNG = false;
		fseek(fp, file_pointer, SEEK_SET);

		picWidth = realPicWidth = get2bytes (fp);
		picHeight = realPicHeight = get2bytes (fp);
	} else {
		// Read the PNG header

		png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png_ptr) {
			return false;
		}

		info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr) {
			png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
			return false;
		}

		end_info = png_create_info_struct(png_ptr);
		if (!end_info) {
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			return false;
		}
		png_init_io(png_ptr, fp);		// Tell libpng which file to read
		png_set_sig_bytes(png_ptr, 8);	// 8 bytes already read

		png_read_info(png_ptr, info_ptr);

		png_uint_32 width, height;
		int bit_depth, color_type, interlace_type, compression_type, filter_method;
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

		picWidth = realPicWidth = width;
		picHeight = realPicHeight = height;

		if (bit_depth < 8) png_set_packing(png_ptr);
		png_set_expand(png_ptr);
		if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png_ptr);
		if (bit_depth == 16) png_set_strip_16(png_ptr);

		png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);

		png_read_update_info(png_ptr, info_ptr);
		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

		//int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

	}

	if (x == IN_THE_CENTRE) x = (sceneWidth - realPicWidth) >> 1;
	if (y == IN_THE_CENTRE) y = (sceneHeight - realPicHeight) >> 1;
	if (x < 0 || x + realPicWidth > sceneWidth || y < 0 || y + realPicHeight > sceneHeight) return false;

	float btx1, tx1;
	float btx2, tx2;
	float bty1, ty1;
	float bty2, ty2;
	if (! NPOT_textures) {
		tx1 = 0.0;
		ty1 = 0.0;
		tx2 = ((double)picWidth) / getNextPOT(picWidth);
		ty2 = ((double)picHeight) / getNextPOT(picHeight);
		picWidth = getNextPOT(picWidth);
		picHeight = getNextPOT(picHeight);
		btx1 = backdropTexW * x / sceneWidth;
		btx2 = backdropTexW * (x+picWidth) / sceneWidth;
		bty1 = backdropTexH * y / sceneHeight;
		bty2 = backdropTexH * (y+picHeight) / sceneHeight;
	} else {
		tx1 = 0.0;
		ty1 = 0.0;
		tx2 = 1.0;
		ty2 = 1.0;
		btx1 = (float) x / sceneWidth;
		btx2 = (float) (x+picWidth) / sceneWidth;
		bty1 = (float) y / sceneHeight;
		bty2 = (float) (y+picHeight) / sceneHeight;
	}
	int t1, t2, n;
	unsigned short c;
	GLubyte * target;
	int32_t transCol = 63519;


	if (fileIsPNG) {
		unsigned char * row_pointers[realPicHeight];
		for (int i = 0; i<realPicHeight; i++)
			row_pointers[i] = backdropTexture + 4*i*picWidth;

		png_read_image(png_ptr, (png_byte **) row_pointers);
		png_read_end(png_ptr, NULL);
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	} else {
		for (t2 = 0; t2 < realPicHeight; t2 ++) {
			t1 = 0;
			while (t1 < realPicWidth) {
				c = (unsigned short) get2bytes (fp);
				if (c & 32) {
					n = fgetc (fp) + 1;
					c -= 32;
				} else {
					n = 1;
				}
				while (n --) {
					target = backdropTexture + 4*picWidth*t2 + t1*4;
					if (c == transCol || c == 2015) {
						target[0] = (GLubyte) 0;
						target[1] = (GLubyte) 0;
						target[2] = (GLubyte) 0;
						target[3] = (GLubyte) 0;
					} else {
						target[0] = (GLubyte) redValue(c);
						target[1] = (GLubyte) greenValue(c);
						target[2] = (GLubyte) blueValue(c);
						target[3] = (GLubyte) 255;
					}
					t1++;
				}
			}
		}
	}

	GLuint tmpTex;

	glGenTextures (1, &tmpTex);
	glBindTexture(GL_TEXTURE_2D, tmpTex);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, picWidth, picHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, backdropTexture);

	glEnable (GL_TEXTURE_2D);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	setPixelCoords (true);
	
	
	int xoffset = 0;
	while (xoffset < picWidth) {
		int w = (picWidth-xoffset < viewportWidth) ? picWidth-xoffset : viewportWidth;

		int yoffset = 0;
		while (yoffset < picHeight) {
			int h = (picHeight-yoffset < viewportHeight) ? picHeight-yoffset : viewportHeight;

			glClear(GL_COLOR_BUFFER_BIT);	// Clear The Screen

			// Render the sprite to the backdrop 
			// (using mulitexturing, so the backdrop is seen where alpha < 1.0)
			glActiveTexture(GL_TEXTURE2);
			glBindTexture (GL_TEXTURE_2D, backdropTextureName);
			glActiveTexture(GL_TEXTURE0);
			
			glUseProgram(shader.fixScaleSprite);
			GLint uniform = glGetUniformLocation(shader.fixScaleSprite, "useLightTexture");
			if (uniform >= 0) glUniform1i(uniform, 0); // No lighting
			
			glColor4f(1.0, 1.0, 1.0, 0.5);
			glBindTexture(GL_TEXTURE_2D, tmpTex);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			
			glBegin(GL_QUADS);
			glTexCoord2f(tx1, ty1); glMultiTexCoord2f(GL_TEXTURE2, btx1, bty1); 	glVertex3f(-xoffset, -yoffset, 0.0);
			glTexCoord2f(tx2, ty1); glMultiTexCoord2f(GL_TEXTURE2, btx2, bty1); 	glVertex3f(picWidth-xoffset, -yoffset, 0.0);
			glTexCoord2f(tx2, ty2); glMultiTexCoord2f(GL_TEXTURE2, btx2, bty2); 	glVertex3f(picWidth-xoffset, picHeight-yoffset, 0.0);
			glTexCoord2f(tx1, ty2); glMultiTexCoord2f(GL_TEXTURE2, btx1, bty2); 	glVertex3f(-xoffset, picHeight-yoffset, 0.0);
			glEnd();
			
			// Copy Our ViewPort To The Texture
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glUseProgram(0);
			
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, (int) ((x<0) ? xoffset: x+xoffset), (int) ((y<0) ? yoffset: y+yoffset), (int) ((x<0) ?viewportOffsetX-x:viewportOffsetX), (int) ((y<0) ?viewportOffsetY-y:viewportOffsetY), w, h);

			yoffset += viewportHeight;
		}

		xoffset += viewportWidth;
	}
	glDeleteTextures(1, &tmpTex);
	setPixelCoords (false);
	if (maxAntiAliasSettings.useMe) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	return true;
}

void saveCorePNG  (FILE * writer, GLuint texture, int w, int h) {
	GLint texw, texh;

	glBindTexture (GL_TEXTURE_2D, texture);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texw);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texh);

	GLubyte* image = new GLubyte [texw*texh*3];
	glPixelStorei (GL_PACK_ALIGNMENT, 1);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	png_structp png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		fatal ("Error saving image!");
		return;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr){
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		fatal ("Error saving image!");
		return;
	}

	png_init_io(png_ptr, writer);

	png_set_IHDR(png_ptr, info_ptr, w, h,
				 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	unsigned char * row_pointers[h];

	for (int i = 0; i < h; i++) {
		row_pointers[i] = image + 3*i*w;
	}

	png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	delete image;
	image = NULL;
}

void saveCoreHSI (FILE * writer, GLuint texture, int w, int h) {

	GLint texw, texh;

	glBindTexture (GL_TEXTURE_2D, texture);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texw);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texh);

	GLushort* image = new GLushort [texw*texh];
	glPixelStorei (GL_PACK_ALIGNMENT, 1);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, image);

	int x, y, lookAhead;
	unsigned short int * fromHere, * lookPointer;

	put2bytes (w, writer);
	put2bytes (h, writer);

	for (y = 0; y < h; y ++) {
		fromHere = image +(y*w);
		x = 0;
		while (x < w) {
			lookPointer = fromHere + 1;
			for (lookAhead = x + 1; lookAhead < w; lookAhead ++) {
				if (lookAhead - x == 256) break;
				if (* fromHere != * lookPointer) break;
				lookPointer ++;
			}
			if (lookAhead == x + 1) {
				put2bytes ((* fromHere) & 65503, writer);
			} else {
				put2bytes (* fromHere | 32, writer);
				fputc (lookAhead - x - 1, writer);
			}
			fromHere = lookPointer;
			x = lookAhead;
		}
	}
	delete image;
	image = NULL;
}

void saveHSI (FILE * writer) {
	if (gameVersion >= VERSION(2,0))
		saveCorePNG  (writer, backdropTextureName, sceneWidth, sceneHeight);
	else
		saveCoreHSI (writer, backdropTextureName, sceneWidth, sceneHeight);
}

bool getRGBIntoStack (unsigned int x, unsigned int y, stackHandler * sH) {
	if (x >= sceneWidth || y >= sceneHeight) {
		return fatal ("Co-ordinates are outside current scene!");
	}

	variable newValue;

	newValue.varType = SVT_NULL;

	glBindTexture (GL_TEXTURE_2D, backdropTextureName);
	glGetTexImage (GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, backdropTexture);

	GLubyte * target;
	if (! NPOT_textures) {
		target = backdropTexture + 4*getNextPOT(sceneWidth)*y + x*4;
	} else {
		target = backdropTexture + 4*sceneWidth*y + x*4;
	}

	setVariable (newValue, SVT_INT, target[2]);
	if (! addVarToStackQuick (newValue, sH -> first)) return false;
	sH -> last = sH -> first;

	setVariable (newValue, SVT_INT, target[1]);
	if (! addVarToStackQuick (newValue, sH -> first)) return false;

	setVariable (newValue, SVT_INT, target[0]);
	if (! addVarToStackQuick (newValue, sH -> first)) return false;

	return true;
}

void saveParallaxRecursive (parallaxLayer * me, FILE * fp) {
	if (me) {
		saveParallaxRecursive (me -> next, fp);
		fputc (1, fp);
		put2bytes (me->fileNum, fp);
		put2bytes (me ->fractionX, fp);
		put2bytes (me->fractionY, fp);
	}
}

