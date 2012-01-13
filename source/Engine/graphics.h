/*
 *  Graphics.h
 *  SdlSludge
 *
 *  Created by Rikard Peterson on 2009-05-01.
 *
 */

#ifndef __SLUDGE_GRAPHICS_H__
#define __SLUDGE_GRAPHICS_H__

#include "GLee.h"

#define TEXCOORD_ARRAY 1

struct texture {
	GLubyte * data;
	GLuint name;
	int w, h;
	double texW, texH;
};

struct shaders {
	GLuint paste;
	GLuint smartScaler;
	GLuint yuv;
	GLuint texture;
};

// From Backdrop.cpp, but they're here anyway
extern GLubyte * backdropTexture;
extern GLfloat backdropTexW, backdropTexH;


extern unsigned int winWidth, winHeight;
extern int viewportHeight, viewportWidth;
extern int viewportOffsetX, viewportOffsetY;
extern int realWinWidth, realWinHeight;

extern bool NPOT_textures;
extern shaders shader;

void setPixelCoords (bool pixels);
void setGraphicsWindow(bool fullscreen, bool restoreGraphics = true, bool resize = false);

void setupOpenGLStuff();

int getNextPOT(int n);

void saveTexture (GLuint tex, GLubyte * data);

#endif
