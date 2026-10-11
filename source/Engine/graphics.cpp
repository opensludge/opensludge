#include <stdarg.h>
#include <stdlib.h>
#include <SDL3/SDL.h>

#include "debug.h"
#include "platform-dependent.h"
#include "specialsettings.h"
#include "graphics.h"
#include "dxgipresent.h"
#include "language.h"
#include "newfatal.h"
#include "sprbanks.h"
#include "zbuffer.h"
#include "backdrop.h"
#include "shaders.h"
#include "movie.h"
#include "stringy.h"

#include "language.h" // for settings

#if defined(SLUDGE_FRAMEBUFFER_VSYNC)
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#ifndef FBIO_WAITFORVSYNC
#define FBIO_WAITFORVSYNC _IOW('F', 0x20, __u32)
#endif
#endif

unsigned int winWidth, winHeight;
int viewportHeight, viewportWidth;
int viewportOffsetX = 0, viewportOffsetY = 0;

extern float cameraZoom;

bool NPOT_textures = true;

extern int specialSettings;

void setMovieViewport();

extern GLuint backdropTextureName;

shaders shader;
GLfloat aPMVMatrix[16];

void sludgeDisplay ();

GLfloat primaryColor[4];
GLfloat secondaryColor[4];

struct textureList *firstTexture = NULL;

textureList * addTexture () {
	textureList * newTexture = new textureList;
	newTexture -> next = firstTexture;
	firstTexture = newTexture;
	return newTexture;
}

void deleteTextures(GLsizei n,  const GLuint * textures)
{
	if (firstTexture == NULL) {
		//debugOut("Deleting texture while list is already empty.\n");
	} else {
		for (int i = 0; i < n; i++) {
			bool found = false;
			textureList *list = firstTexture;
			if (list->name == textures[i]) {
				found = true;
				firstTexture = list->next;
				delete list;
				continue;
			}

			while (list->next) {
				if (list->next->name == textures[i]) {
					found = true;
					textureList *deleteMe = list->next;
					list->next = list->next->next;
					delete deleteMe;
					break;
				}
				list = list->next;
			}
			//if (!found)
			//	debugOut("Deleting texture that was not in list.\n");
		}
	}

	glDeleteTextures(n, textures);
}

void getTextureDimensions(GLuint name, GLint *width,  GLint *height)
{
	textureList *list = firstTexture;
	while (list) {
		if (list->name == name)  {
			*width = list->width;
			*height = list->height;
#if !defined(HAVE_GLES2)
			//For the following test it is assumed that glBindTexture is always 
			//called for the right texture before getTextureDimensions.
			GLint tw, th;
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tw); 
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &th);
			if (tw != *width || th != *height) {
				debugOut ("Warning: Texture dimensions don't match: They are %ix%i, but SLUDGEs bookkeeping says %ix%i.\n", tw, th, *width, *height);
			}
#endif
			return;
		}
		list = list->next;
	}
	fatal("Texture not found in list.\n");
}

void storeTextureDimensions(GLuint name, GLsizei width,  GLsizei height, const char *file, int line)
{
	if (! NPOT_textures && !(((height & (height - 1)) == 0) || ((width & (width - 1)) == 0))) {
		debugOut("I was told to create a texture with dimensions %ix%i in %s @ line %d although NPOT textures are disabled.\n", width, height, file, line);
		//height = getNextPOT(height);
		//width = getNextPOT(width);
	}

	textureList *list = firstTexture;
	while (list) {
		if (list->name == name)  {
			//debugOut("Texture dimensions are overwritten.\n");
			break;
		}
		list = list->next;
	}
	if (list == NULL) {
		list = addTexture();
	}
	list->name = name;
	list->width = width;
	list->height = height;
}
#ifdef HAVE_GLES2
void glesCopyTexSubImage2D(GLenum target,  GLint level, GLint xoffset, GLint yoffset, GLint x,  GLint y,  GLsizei width,  GLsizei height)
{
	// Work around for broken glCopy(Sub)TexImage2D...
    void* tmp = malloc(width*height*4);
    glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
    glTexSubImage2D(target, level, xoffset, yoffset, width, height, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
    free(tmp);
}
void glesCopyTexImage2D(GLenum target,  GLint level,  GLenum internalformat,  GLint x,  GLint y,  GLsizei width,  GLsizei height,  GLint border)
{
	// Work around for broken glCopy(Sub)TexImage2D...
	void* tmp = malloc(width*height*4);
	glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
	glTexImage2D(target, level, GL_RGBA, width, height, border, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
	free(tmp);
}
#endif

void dcopyTexImage2D(GLenum target,  GLint level,  GLenum internalformat,  GLint x,  GLint y,  GLsizei width,  GLsizei height,  GLint border, GLuint name, const char *file, int line)
{
	glBindTexture(GL_TEXTURE_2D, name);
	#ifdef HAVE_GLES2_
	glesCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
	#else
	glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
	#endif
}

void dcopyTexSubImage2D(GLenum target,  GLint level,  GLint xoffset,  GLint yoffset,  GLint x,  GLint y,  GLsizei width,  GLsizei height, GLuint name, const char *file, int line)
{
	glBindTexture(GL_TEXTURE_2D, name);
	#ifdef HAVE_GLES2_
	glesCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
	#else
	glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
	#endif
}

void dtexImage2D(GLenum target,  GLint level,  GLint internalformat,  GLsizei width,  GLsizei height, 
		GLint border,  GLenum format,  GLenum type,  const GLvoid * data, GLuint name, const char *file, int line)
{
	storeTextureDimensions(name, width,  height, file, line);
	glBindTexture(GL_TEXTURE_2D, name);
	glTexImage2D(target, level, internalformat, width, height, border, format, type,  data);
}

void dtexSubImage2D(GLenum target,  GLint level,  GLint xoffset,  GLint yoffset,  GLsizei width,  GLsizei height,
		GLenum format,  GLenum type,  const GLvoid * data, GLuint name, const char *file, int line) 
{
	storeTextureDimensions(name, width,  height, file, line);
	glBindTexture(GL_TEXTURE_2D, name);
	glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, data);
}

void setPrimaryColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	primaryColor[0] = r;
	primaryColor[1] = g;
	primaryColor[2] = b;
	primaryColor[3] = a;
}

void setSecondaryColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	secondaryColor[0] = r;
	secondaryColor[1] = g;
	secondaryColor[2] = b;
	secondaryColor[3] = a;
}

void drawQuad(GLint program, const GLfloat* vertices, int numTexCoords, ...)
{
	int i, vertexLoc, texCoordLocs[numTexCoords];
	const GLfloat* texCoords[numTexCoords];

	va_list vl;
	va_start(vl,numTexCoords);
	for (i=0;i<numTexCoords;i++)
	{
		texCoords[i]=va_arg(vl,const GLfloat*);
	}
	va_end(vl);

	glUniform4f(glGetUniformLocation(program, "myColor"), primaryColor[0], primaryColor[1], primaryColor[2], primaryColor[3]);
	if (program == shader.smartScaler || program == shader.paste)
	{
		glUniform4f(glGetUniformLocation(program, "mySecondaryColor"), secondaryColor[0], secondaryColor[1], secondaryColor[2], secondaryColor[3]);
	}

	vertexLoc = glGetAttribLocation(program, "myVertex");
	texCoordLocs[0] = glGetAttribLocation(program, "myUV0");
	if (numTexCoords > 1) texCoordLocs[1] = glGetAttribLocation(program, "myUV1");
	if (numTexCoords > 2) texCoordLocs[2] = glGetAttribLocation(program, "myUV2");
	if (numTexCoords > 3) texCoordLocs[3] = glGetAttribLocation(program, "myUV3");
 		
	glEnableVertexAttribArray(vertexLoc);
	glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, vertices);

	for (i=0;i<numTexCoords;i++)
	{
		if (texCoords[i]) {
			glEnableVertexAttribArray(texCoordLocs[i]);
			glVertexAttribPointer(texCoordLocs[i], 2, GL_FLOAT, GL_FALSE, 0, texCoords[i]);
		}
	}

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	for (i=0;i<numTexCoords;i++)
	{
		if (texCoords[i]) {
			glDisableVertexAttribArray(texCoordLocs[i]);
		}
	}

	glDisableVertexAttribArray(vertexLoc);
}


void setPMVMatrix(GLint program) {
	glUniformMatrix4fv( glGetUniformLocation(program, "myPMVMatrix"), 1, GL_FALSE, aPMVMatrix);
}

void setGameMatrix (float scale) {
	GLfloat w = (GLfloat) winWidth / (cameraZoom * scale);
	GLfloat h = (GLfloat) winHeight / (cameraZoom * scale);

	const GLfloat bPMVMatrix[] =
	{
	2.0f/w,      .0,   .0,  .0,
	    .0, -2.0f/h,   .0,  .0,
	    .0,      .0, 1.0f,  .0,
	  -1.0,    1.0f,   .0, 1.0f

	};
	for (int i = 0; i < 16; i++)
	{
		aPMVMatrix[i] = bPMVMatrix[i];
	}
}

// This is for swapping settings between rendering to texture or to the screen
void setPixelCoords (bool pixels) {
	static int current = -1;
//	if (current == pixels) return;
	current = pixels;

	glBindTexture (GL_TEXTURE_2D, backdropTextureName);

	if (pixels) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glClear(GL_COLOR_BUFFER_BIT);

		const GLfloat bPMVMatrix[] =
		{
		2.0f/viewportWidth,                  .0,   .0,  .0,
			        .0, 2.0f/viewportHeight,   .0,  .0,
			        .0,                  .0, 1.0f,  .0,
			      -1.0,               -1.0f,   .0, 1.0f

		};
		for (int i = 0; i < 16; i++)
		{
			aPMVMatrix[i] = bPMVMatrix[i];
		}
	} else {
		if (gameSettings.antiAlias < 0) {
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		} else {
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}

		setGameMatrix (1);
	}
}

bool runningFullscreen = false;
extern char * gameName;


#if defined(HAVE_GLES2)
void saveTexture (GLuint tex, GLubyte * data) {
	// use an FBO to easily grab the texture...
	static GLuint fbo = 0;
	GLuint old_fbo;
	GLint tw, th;
	GLint old_vp[4];
	if (fbo==0) {
		glGenFramebuffers(1, &fbo);
	}
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&old_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	getTextureDimensions(tex, &tw, &th);
	glGetIntegerv(GL_VIEWPORT, old_vp);
	glViewport(0,0, tw, th);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glReadPixels(0, 0, tw, th, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glViewport(old_vp[0], old_vp[1], old_vp[2], old_vp[3]);
	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}
#elif defined _WIN32
// Replacement for glGetTexImage, because some ATI drivers are buggy.
void saveTexture (GLuint tex, GLubyte * data) {
	setPixelCoords (true);

	glBindTexture (GL_TEXTURE_2D, tex);

	GLint tw, th;
	getTextureDimensions(tex, &tw, &th);

	//glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	int xoffset = 0;
	while (xoffset < tw) {
		int w = (tw-xoffset < viewportWidth) ? tw-xoffset : viewportWidth;

		int yoffset = 0;
		while (yoffset < th) {
			int h = (th-yoffset < viewportHeight) ? th-yoffset : viewportHeight;

			glClear(GL_COLOR_BUFFER_BIT);	// Clear The Screen

			const GLfloat vertices[] = { 
				(GLfloat)-xoffset, (GLfloat)-yoffset, 0.f, 
				(GLfloat)tw-xoffset, (GLfloat)-yoffset, 0.f, 
				(GLfloat)-xoffset, (GLfloat)-yoffset+th, 0.f,
				(GLfloat)tw-xoffset, (GLfloat)-yoffset+th, 0.f
			};

			const GLfloat texCoords[] = { 
				0.0f, 0.0f,
				1.0f, 0.0f,
				0.0f, 1.0f,
				1.0f, 1.0f
			}; 

			glUseProgram(shader.texture);
			setPMVMatrix(shader.texture);

			drawQuad(shader.texture, vertices, 1, texCoords);
			glUseProgram(0);

			for (int i = 0; i<h; i++)	{
				glReadPixels(viewportOffsetX, viewportOffsetY+i, w, 1, GL_RGBA, GL_UNSIGNED_BYTE, data+xoffset*4+(yoffset+i)*4*tw);
			}

			yoffset += viewportHeight;
		}

		xoffset += viewportWidth;
	}
	//glReadPixels(viewportOffsetX, viewportOffsetY, tw, th, GL_RGBA, GL_UNSIGNED_BYTE, data);

	setPixelCoords (false);

}
#else
void saveTexture (GLuint tex, GLubyte * data) {
	glBindTexture (GL_TEXTURE_2D, tex);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}
#endif

SDL_Window * sludgeWindow = NULL;
static SDL_GLContext glContext = NULL;

static void buildShaderPrograms () {
	GLint uniform;
	const char *Vertex;
	const char *Fragment;

        Vertex = shaderFileRead("scale.vert");

#if !defined(HAVE_GLES2)
        Fragment = shaderFileRead("scale.frag");
#else
/*	const GLubyte *str;
	int glDerivativesAvailable;
	str = glGetString (GL_EXTENSIONS);
	glDerivativesAvailable = (strstr((const char *)str, "GL_OES_standard_derivatives") != NULL);
	if (!glDerivativesAvailable) {
		debugOut("Extension \"GL_OES_standard_derivatives\" not available. Advanced anti-aliasing is not possible. Using linear anti-aliasing instead.");
		gameSettings.antiAlias = -1;
*/
        	Fragment = shaderFileRead("scale_noaa.frag");
//	}

	Fragment = joinStrings("precision mediump float;\n", Fragment);
#endif

        if (! Vertex || ! Fragment) {
            fatal ("Error loading \"scale\" shader program!", "Try re-installing the game. (scale.frag, scale_noaa.frag or scale.vert was not found.)");
            gameSettings.antiAlias = -1;
            shader.smartScaler = 0;
        } else {

            shader.smartScaler = buildShaders (Vertex, Fragment);
			
			if (! shader.smartScaler) {
				fatal ("Error building \"scale\" shader program!");
				gameSettings.antiAlias = -1;
				shader.smartScaler = 0;
			} else {
				debugOut( "Built shader program: %d (smartScaler)\n", shader.smartScaler);
				glUseProgram(shader.smartScaler);
				uniform = glGetUniformLocation(shader.smartScaler, "Texture");
				if (uniform >= 0) glUniform1i(uniform, 0);
				uniform = glGetUniformLocation(shader.smartScaler, "lightTexture");
				if (uniform >= 0) glUniform1i(uniform, 1);
				uniform = glGetUniformLocation(shader.smartScaler, "useLightTexture");
				if (uniform >= 0) glUniform1i(uniform, 0);
				uniform = glGetUniformLocation(shader.smartScaler, "antialias");
				if (uniform >= 0) glUniform1i(uniform, 0);
			}
        }

	Vertex = shaderFileRead("fixScaleSprite.vert");
	Fragment = shaderFileRead("fixScaleSprite.frag");

#if defined(HAVE_GLES2)
	Fragment = joinStrings("precision mediump float;\n", Fragment);
#endif

	if (! Vertex || ! Fragment) {
		fatal ("Error loading \"fixScaleSprite\" shader program!", "Try re-installing the game. (fixScaleSprite.frag or fixScaleSprite.vert was not found.)");
		shader.paste = 0;
	} else {

		shader.paste = buildShaders (Vertex, Fragment);
		if (! shader.paste) {
			fatal( "Error building \"fixScaleSprite\" shader program!");
		} else {
			debugOut( "Built shader program: %d (fixScaleSprite)\n", shader.paste);
			glUseProgram(shader.paste);
			uniform = glGetUniformLocation(shader.paste, "tex0");
			if (uniform >= 0) glUniform1i(uniform, 0);
			uniform = glGetUniformLocation(shader.paste, "tex1");
			if (uniform >= 0) glUniform1i(uniform, 1);
			uniform = glGetUniformLocation(shader.paste, "tex2");
			if (uniform >= 0) glUniform1i(uniform, 2);
			uniform = glGetUniformLocation(shader.paste, "useLightTexture");
			if (uniform >= 0) glUniform1i(uniform, 0);
		}
	}

	Vertex = shaderFileRead("yuv.vert");
	Fragment = shaderFileRead("yuv.frag");

#if defined(HAVE_GLES2)
	Fragment = joinStrings("precision mediump float;\n", Fragment);
#endif

	if (! Vertex || ! Fragment) {
		fatal ("Error loading \"yuv\" shader program!", "Try re-installing the game. (yuv.frag or yuv.vert was not found.)");
		shader.yuv = 0;
	} else {

		shader.yuv = buildShaders (Vertex, Fragment);
		if (! shader.yuv) {
			fatal( "Error building \"yuv\" shader program!");			
		} else {
			debugOut( "Built shader program: %d (yuv)\n", shader.yuv);
			glUseProgram(shader.yuv);
			uniform = glGetUniformLocation(shader.yuv, "Ytex");
			if (uniform >= 0) glUniform1i(uniform, 0);
			uniform = glGetUniformLocation(shader.yuv, "Utex");
			if (uniform >= 0) glUniform1i(uniform, 1);
			uniform = glGetUniformLocation(shader.yuv, "Vtex");
			if (uniform >= 0) glUniform1i(uniform, 2);
		}
	}

	Vertex = shaderFileRead("texture.vert");
	Fragment = shaderFileRead("texture.frag");

#if defined(HAVE_GLES2)
	Fragment = joinStrings("precision mediump float;\n", Fragment);
#endif

	if (! Vertex || ! Fragment) {
		fatal ("Error loading \"texture\" shader program!", "Try re-installing the game. (texture.frag or texture.vert was not found.)");
		shader.texture = 0;
	} else {

		shader.texture = buildShaders (Vertex, Fragment);
		if (! shader.texture) {
			fatal( "Error building \"texture\" shader program!");
		} else {
			debugOut( "Built shader program: %d (texture)\n", shader.texture);
			glUseProgram(shader.texture);
			uniform = glGetUniformLocation(shader.texture, "sampler2d");
			if (uniform >= 0) glUniform1i(uniform, 0);
			uniform = glGetUniformLocation(shader.texture, "zBuffer");
			if (uniform >= 0) glUniform1i(uniform, 0);
			uniform = glGetUniformLocation(shader.texture, "zBufferLayer");
			if (uniform >= 0) glUniform1f(uniform, 0.);
			uniform = glGetUniformLocation(shader.texture, "modulateColor");
			if (uniform >= 0) glUniform1i(uniform, 0);
		}
	}

	Vertex = shaderFileRead("color.vert");
	Fragment = shaderFileRead("color.frag");

#if defined(HAVE_GLES2)
	Fragment = joinStrings("precision mediump float;\n", Fragment);
#endif

	if (! Vertex || ! Fragment) {
		fatal ("Error loading \"color\" shader program!", "Try re-installing the game. (color.frag or color.vert was not found.)");
		shader.color = 0;
	} else {

		shader.color = buildShaders (Vertex, Fragment);
		if (! shader.color) {
			fatal( "Error building \"color\" shader program!");
		} else {
			debugOut( "Built shader program: %d (color)\n", shader.color);
			glUseProgram(shader.color);
		}
	}

	glUseProgram(0);
}

// Set the area of the window in which we draw the game. We keep the aspect
// ratio of the game and leave black bars in the rest of the window. With
// fixed pixels, we scale the game by a whole number only. All sizes are in
// framebuffer pixels, which on a high-DPI display are more than the window
// size in points.
void updateViewport () {
	if (! sludgeWindow) return;

	SDL_GetWindowSizeInPixels (sludgeWindow, &realWinWidth, &realWinHeight);
	if (realWinWidth < 1) realWinWidth = 1;
	if (realWinHeight < 1) realWinHeight = 1;
#if defined(_WIN32) && ! defined(HAVE_GLES2)
	resizeDXGIPresentation (realWinWidth, realWinHeight);
#endif

	if (gameSettings.fixedPixels) {
		int scale = realWinWidth / (int) winWidth;
		if (realWinHeight / (int) winHeight < scale) scale = realWinHeight / (int) winHeight;
		if (scale < 1) scale = 1;
		viewportWidth = winWidth * scale;
		viewportHeight = winHeight * scale;
	} else {
		float winAspect = (float) winWidth / winHeight;
		float realAspect = (float) realWinWidth / realWinHeight;
		if (realAspect > winAspect) {
			viewportHeight = realWinHeight;
			viewportWidth = (int) (realWinHeight * winAspect);
		} else {
			viewportWidth = realWinWidth;
			viewportHeight = (int) ((float) realWinWidth / winAspect);
		}
	}
	viewportOffsetX = (realWinWidth - viewportWidth) / 2;
	viewportOffsetY = (realWinHeight - viewportHeight) / 2;

	glViewport (viewportOffsetX, viewportOffsetY, viewportWidth, viewportHeight);

	if (shader.smartScaler) {
		glUseProgram (shader.smartScaler);
		GLint uniform = glGetUniformLocation (shader.smartScaler, "scale");
		float scale = (float) viewportWidth / (float) winWidth * 0.25;
		if (scale > 1.0) scale = 1.0;
		if (uniform >= 0) glUniform1f (uniform, scale);
		glUseProgram (0);
	}

	setPixelCoords (false);

	if (movieIsPlaying)
		setMovieViewport ();

	debugOut ("Window is %dx%d pixels, game drawn at %d,%d size %dx%d\n", realWinWidth, realWinHeight, viewportOffsetX, viewportOffsetY, viewportWidth, viewportHeight);
}

// Convert a position in window coordinates, as in mouse events, to game
// coordinates.
void windowToGameCoords (float x, float y, int * gameX, int * gameY) {
	float density = sludgeWindow ? SDL_GetWindowPixelDensity (sludgeWindow) : 1.0f;
	if (density <= 0) density = 1.0f;

	// In OpenGL, the viewport offset is from the bottom of the window. Mouse
	// positions are from the top.
	float px = x * density - viewportOffsetX;
	float py = y * density - (realWinHeight - viewportOffsetY - viewportHeight);

	*gameX = (int) (px * ((float) winWidth / cameraZoom) / viewportWidth);
	*gameY = (int) (py * ((float) winHeight / cameraZoom) / viewportHeight);
}

// Convert a position in game coordinates to window coordinates.
void gameToWindowCoords (float gameX, float gameY, float * x, float * y) {
	float density = sludgeWindow ? SDL_GetWindowPixelDensity (sludgeWindow) : 1.0f;
	if (density <= 0) density = 1.0f;

	float px = gameX * viewportWidth / ((float) winWidth / cameraZoom) + viewportOffsetX;
	float py = gameY * viewportHeight / ((float) winHeight / cameraZoom) + (realWinHeight - viewportOffsetY - viewportHeight);
	*x = px / density;
	*y = py / density;
}

static void createGraphicsWindow (bool fullscreen) {
	SDL_Rect usable;
	if (! SDL_GetDisplayUsableBounds (SDL_GetPrimaryDisplay (), &usable)) {
		usable.w = 1024;
		usable.h = 768;
	}

	int w, h;
	if (gameSettings.fixedPixels) {
		w = winWidth;
		h = winHeight;
	} else {
		// Fill three quarters of the screen height by default.
		float winAspect = (float) winWidth / winHeight;
		h = usable.h * 3 / 4;
		w = (int) (h * winAspect);
		if (w > usable.w) {
			w = usable.w;
			h = (int) ((float) w / winAspect);
		}
	}

	// We show the window when it is ready.
	SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN;
	if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN;
	const char * title = gameName ? gameName : "SLUDGE";

#if defined(_WIN32) && ! defined(HAVE_GLES2)
	// On Windows, we show the frames through DXGI where we can, in a
	// window without OpenGL (see dxgipresent.cpp).
	sludgeWindow = SDL_CreateWindow (title, w, h, flags);
	if (sludgeWindow) {
		glContext = startDXGIPresentation (sludgeWindow);
		if (! glContext) {
			SDL_DestroyWindow (sludgeWindow);
			sludgeWindow = NULL;
		}
	}
#endif

	if (! sludgeWindow) {
		sludgeWindow = SDL_CreateWindow (title, w, h, flags | SDL_WINDOW_OPENGL);
		if (! sludgeWindow) {
			msgBox ("Startup Error: Couldn't create the game window.", SDL_GetError ());
			SDL_Quit ();
			exit (2);
		}
		glContext = SDL_GL_CreateContext (sludgeWindow);
		if (! glContext) {
			msgBox ("Startup Error: Couldn't create an OpenGL context.", SDL_GetError ());
			SDL_Quit ();
			exit (2);
		}
		SDL_GL_MakeCurrent (sludgeWindow, glContext);
		SDL_GL_SetSwapInterval (1);
	}
	SDL_SetWindowMinimumSize (sludgeWindow, 160, 120);
	SDL_ShowWindow (sludgeWindow);

	const char * missing = loadGLFunctions ();
	if (missing) {
		msgBox ("Startup Error: Your graphics driver doesn't support OpenGL 2.0.", missing);
		SDL_Quit ();
		exit (1);
	}

	debugOut ("OpenGL vendor: %s\n", glGetString (GL_VENDOR));
	debugOut ("OpenGL renderer: %s\n", glGetString (GL_RENDERER));
	debugOut ("OpenGL version: %s\n", glGetString (GL_VERSION));

	buildShaderPrograms ();

	/*
	 * Set up OpenGL for 2D rendering.
	 */
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

// Switch between windowed and fullscreen, and create the window the first
// time. We keep the same OpenGL context when switching, so the textures stay
// loaded.
void setGraphicsWindow(bool fullscreen, bool restoreGraphics) {
#if defined(SLUDGE_ALWAYS_FULLSCREEN)
	fullscreen = true;
#endif
	if (fullscreen) specialSettings &= ~SPECIAL_INVISIBLE;

	if (! sludgeWindow) {
		debugHeader();
		createGraphicsWindow (fullscreen);
	} else if (fullscreen != runningFullscreen) {
		SDL_SetWindowFullscreen (sludgeWindow, fullscreen);
		SDL_SyncWindow (sludgeWindow);
	}
	runningFullscreen = fullscreen;

	updateViewport ();

	if (restoreGraphics) sludgeDisplay ();
}

void setupOpenGLStuff() {

	/*
	 * We want *at least* 8 bits of red, green, blue and alpha, a 16-bit
	 * depth buffer and a double buffered window.
	 */
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
#if defined(HAVE_GLES2)
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
	// The shaders are in GLSL 1.10, which is only available in a
	// compatibility context. On macOS, that is an OpenGL 2.1 context.
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
#endif

	setGraphicsWindow(gameSettings.userFullScreen, false);

#if !defined(HAVE_GLES2)
	/* Check for graphics capabilities... */
	if (getGLVersion () >= 20) {
		// Yes! Textures can be any size!
		NPOT_textures = true;
		debugOut( "OpenGL 2.0! All is good.\n");
	} else {
		NPOT_textures = false;
		debugOut( "Warning: OpenGL version older than 2.0. Expect trouble.\n");
	}
#else
	NPOT_textures = false;
#endif

	int n;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, (GLint *) &n);
	debugOut("Max texture image units: %d\n", n);
}

#if defined(SLUDGE_FRAMEBUFFER_VSYNC)
// Wait for the next vertical sync of the framebuffer. Without /dev/fb0, we
// do not wait.
static void waitForFramebufferVSync () {
	static int framebuffer = open ("/dev/fb0", O_RDONLY);
	if (framebuffer < 0) return;
	int arg = 0;
	ioctl (framebuffer, FBIO_WAITFORVSYNC, &arg);
}
#endif

void swapGraphicsBuffers () {
#if defined(SLUDGE_FRAMEBUFFER_VSYNC)
	waitForFramebufferVSync ();
#endif
#if defined(_WIN32) && ! defined(HAVE_GLES2)
	if (presentingWithDXGI ()) {
		presentWithDXGI ();
		return;
	}
#endif
	SDL_GL_SwapWindow (sludgeWindow);
}


// I found this function on a coding forum on the 'net.
// Looks a bit weird, but it should work.
int getNextPOT(int n) {
	--n;
	n |= n >> 16;
	n |= n >> 8;
	n |= n >> 4;
	n |= n >> 2;
	n |= n >> 1;
	++n;
	return n;
}

int printOglError (const char *file, int         line)
{
	/* Returns 1 if an OpenGL error occurred, 0 otherwise. */
	GLenum glErr;
	int    retCode = 0;

	glErr = glGetError ();
	while (glErr != GL_NO_ERROR)
    {
		debugOut("glError in file %s @ line %d: error code 0x%x\n", file, line, glErr);
		retCode = 1;
		glErr = glGetError ();
    }
	return retCode;
}
