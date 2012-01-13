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

void drawQuad(GLint program, const GLfloat* vertices, int numTexCoords, ...);

void setPMVMatrix(GLint program);

void setPixelCoords (bool pixels);
void setGraphicsWindow(bool fullscreen, bool restoreGraphics = true, bool resize = false);

void setupOpenGLStuff();

int getNextPOT(int n);

void saveTexture (GLuint tex, GLubyte * data);

void dcopyTexImage2D(GLenum target,  GLint level,  GLenum internalformat,  GLint x,  GLint y,  GLsizei width,  GLsizei height,  GLint border, GLuint name, const char *file, int line);
void dcopyTexSubImage2D(GLenum target,  GLint level,  GLint xoffset,  GLint yoffset,  GLint x,  GLint y,  GLsizei width,  GLsizei height, GLuint name, const char *file, int line);
void dtexImage2D(GLenum target,  GLint level,  GLint internalformat,  GLsizei width,  GLsizei height,  GLint border,  GLenum format,  GLenum type,  const GLvoid * data, GLuint name, const char *file, int line);
void dtexSubImage2D(GLenum target,  GLint level,  GLint xoffset,  GLint yoffset,  GLsizei width,  GLsizei height,  GLenum format,  GLenum type,  const GLvoid * data, GLuint name, const char *file, int line);

#define copyTexImage2D(target, level, internalformat, x, y,  width, height, border, name) dcopyTexImage2D(target,  level,  internalformat,  x,  y,  width,height, border, name, __FILE__, __LINE__)

#define copyTexSubImage2D(target,  level,  xoffset,yoffset, x,  y,   width,   height, name) dcopyTexSubImage2D(target,  level,  xoffset,  yoffset,  x,  y,   width,  height, name, __FILE__, __LINE__)

#define texImage2D(target,  level,  internalformat,  width,  height,  border,  format, type,  data,name) dtexImage2D( target,   level,  internalformat, width, height, border,  format,  type,  data, name, __FILE__, __LINE__)

#define texSubImage2D( target,  level,   xoffset,   yoffset,   width,  height, format,  type,   data,name) dtexSubImage2D( target, level,   xoffset,  yoffset,  width,  height, format,  type,  data,  name, __FILE__, __LINE__)

void deleteTextures(GLsizei n,  const GLuint * textures);

void getTextureDimensions(GLuint name, GLint *width,  GLint *height);

int printOglError (const char *file, int         line);
#define printOpenGLError() printOglError(__FILE__, __LINE__)

#endif
