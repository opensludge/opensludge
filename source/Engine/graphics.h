/*
 *  Graphics.h
 *  SdlSludge
 *
 *  Created by Rikard Peterson on 2009-05-01.
 *
 */

#ifndef __SLUDGE_GRAPHICS_H__
#define __SLUDGE_GRAPHICS_H__

#if !defined(HAVE_GLES2)
#include "GLee.h"
#else
#include <GLES2/gl2.h>
#include "eglport/eglport.h"
#endif

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
	GLuint color;
};

struct textureList {
	GLuint name;
	GLsizei width;
	GLsizei height; 
	struct textureList * next;
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
extern GLfloat aPMVMatrix[];

void setPrimaryColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void setSecondaryColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);

/* FIXME: remove this
void drawTexturedQuad(const GLfloat* vertices, const GLfloat* texCoords);
void drawTexturedQuad(const GLint* vertices, const GLfloat* texCoords);
*/
void drawTexturedQuadNew(GLint program, const GLfloat* vertices, int numTexCoords, ...);

void setPMVMatrix(GLint program);

void setPixelCoords (bool pixels);
void setGraphicsWindow(bool fullscreen, bool restoreGraphics = true, bool resize = false);

void setupOpenGLStuff();

int getNextPOT(int n);

void saveTexture (GLuint tex, GLubyte * data);

void copyTexImage2D(GLenum target,  GLint level,  GLenum internalformat,  GLint x,  GLint y,  GLsizei width,  GLsizei height,  GLint border, GLuint name);
void copyTexSubImage2D(GLenum target,  GLint level,  GLint xoffset,  GLint yoffset,  GLint x,  GLint y,  GLsizei width,  GLsizei height, GLuint name);
void texImage2D(GLenum target,  GLint level,  GLint internalformat,  GLsizei width,  GLsizei height,  GLint border,  GLenum format,  GLenum type,  const GLvoid * data, GLuint name);
void texSubImage2D(GLenum target,  GLint level,  GLint xoffset,  GLint yoffset,  GLsizei width,  GLsizei height,  GLenum format,  GLenum type,  const GLvoid * data, GLuint name);

int printOglError (const char *file, int         line);
#define printOpenGLError() printOglError(__FILE__, __LINE__)

#endif
