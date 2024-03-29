#include <stdarg.h>
#include <SDL/SDL.h>

#include "debug.h"
#include "platform-dependent.h"
#include "specialsettings.h"
#include "graphics.h"
#include "language.h"
#include "newfatal.h"
#include "sprbanks.h"
#include "zbuffer.h"
#include "backdrop.h"
#include "shaders.h"
#include "movie.h"
#include "stringy.h"

#include "language.h" // for settings

#if !defined(HAVE_GLES2)
#ifdef _WIN32
#include <GL\glu.h> // handy for gluErrorString
#elif defined __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
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
extern GLuint snapshotTextureName;
extern unsigned int sceneWidth, sceneHeight;
extern zBufferData zBuffer;
extern int lightMapNumber;


extern GLuint yTextureName;
extern GLuint uTextureName;
extern GLuint vTextureName;
//extern GLubyte * ytex, * utex, * vtex;

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

		GLfloat w = (GLfloat) winWidth / cameraZoom;
		GLfloat h = (GLfloat) winHeight / cameraZoom;

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
}

int desktopW = 0, desktopH = 0;
bool runningFullscreen = false;


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

// This is for setting windowed or fullscreen graphics.
// Used for switching, and for initial window creation.
void setGraphicsWindow(bool fullscreen, bool restoreGraphics, bool resize) {
#if defined(PANDORA)
	fullscreen = true;
#endif

	GLubyte *snapTexture = NULL;

	Uint32 videoflags = 0;

	if (! desktopW) {
		// Get video hardware information
		const SDL_VideoInfo* videoInfo = SDL_GetVideoInfo();
		desktopW = videoInfo->current_w;
		desktopH = videoInfo->current_h;
	} else if (restoreGraphics && fullscreen == runningFullscreen & ! resize) return;

	runningFullscreen = fullscreen;

	if (restoreGraphics) {
		/*
		 * Save the textures
		 */
		if (backdropTextureName) {
			if (backdropTexture) delete backdropTexture;
			int picWidth = sceneWidth;
			int picHeight = sceneHeight;
			if (! NPOT_textures) {
				picWidth = getNextPOT(picWidth);
				picHeight = getNextPOT(picHeight);
			}
			backdropTexture = new GLubyte [picHeight*picWidth*4];
			if (! checkNew (backdropTexture)) return;			

			saveTexture (backdropTextureName, backdropTexture);
		}
		if (snapshotTextureName) {
			int picWidth = winWidth;
			int picHeight = winHeight;
			if (! NPOT_textures) {
				picWidth = getNextPOT(picWidth);
				picHeight = getNextPOT(picHeight);
			}
			snapTexture = new GLubyte [picHeight*picWidth*4];
			if (! checkNew (snapTexture)) return;

			saveTexture (snapshotTextureName, snapTexture);
		}
	}

	/*
	 * Set the graphics mode
	 */
	float winAspect = (float) winWidth / winHeight;

	if (fullscreen) {
		specialSettings &= ~SPECIAL_INVISIBLE;
#if !defined(HAVE_GLES2)
 		videoflags = SDL_OPENGL | SDL_FULLSCREEN;
#else
		videoflags = SDL_SWSURFACE | SDL_FULLSCREEN;
#endif

        if (gameSettings.fixedPixels) {
            viewportWidth = realWinWidth = winWidth;
            viewportHeight = realWinHeight = winHeight;
            viewportOffsetY = 0;
            viewportOffsetX = 0;
        } else {
            realWinWidth = desktopW;
            realWinHeight = desktopH;

            float realAspect = (float) realWinWidth / realWinHeight;

            if (realAspect > winAspect) {
                viewportHeight = realWinHeight;
                viewportWidth = (int) (realWinHeight * winAspect);
                viewportOffsetY = 0;
                viewportOffsetX = (realWinWidth-viewportWidth)/2;
            } else {
                viewportWidth = realWinWidth;
                viewportHeight = (int)((float) realWinWidth / winAspect);
                viewportOffsetY = (realWinHeight-viewportHeight)/2;
                viewportOffsetX = 0;
            }
        }

	} else {
#if !defined(HAVE_GLES2)
 		videoflags = SDL_OPENGL/* | SDL_RESIZABLE*/;
#else
		videoflags = SDL_SWSURFACE;
#endif

		if (resize) {
            float realAspect = (float) desktopW / desktopH;

			if (realAspect > winAspect) {
				realWinWidth = (int) (realWinHeight * winAspect);
			} else {
				realWinHeight = (int) (realWinWidth / winAspect);
			}

			realAspect = (float) realWinWidth / realWinHeight;

            if (realAspect > winAspect) {
                viewportHeight = realWinHeight;
                viewportWidth = (int) (realWinHeight * winAspect);
                viewportOffsetY = 0;
                viewportOffsetX = (realWinWidth-viewportWidth)/2;
            } else {
                viewportWidth = realWinWidth;
                viewportHeight = (int)((float) realWinWidth / winAspect);
                viewportOffsetY = (realWinHeight-viewportHeight)/2;
                viewportOffsetX = 0;
            }
		} else {

			if (gameSettings.fixedPixels) {
				viewportWidth = realWinWidth = winWidth;
				viewportHeight = realWinHeight = winHeight;
				viewportOffsetY = 0;
				viewportOffsetX = 0;
			} else {
				realWinHeight = desktopH*3/4;
				realWinWidth = (int) (realWinHeight * winAspect);

				if (realWinWidth > desktopW) {
					realWinWidth = desktopW;
					realWinHeight = (int) ((float) realWinWidth / winAspect);
				}

				viewportHeight = realWinHeight;
				viewportWidth = realWinWidth;
				viewportOffsetY = 0;
				viewportOffsetX = 0;
			}
		}
	}

    debugHeader();

	if( SDL_SetVideoMode( realWinWidth, realWinHeight, 32, videoflags ) == 0 ) {
		msgBox("Startup Error: Couldn't set video mode.", SDL_GetError());
		SDL_Quit();
		exit(2);
	}
	debugOut( "Video mode %d %d set successfully.\n", realWinWidth, realWinHeight);

#if defined(HAVE_GLES2)
	if (EGL_Open()) {
		msgBox("Startup Error", "Couldn't initialize EGL.");
		SDL_Quit();
		exit(1);
	}
	EGL_Init();
#else
	GLenum glewErr = glewInit();
	if (GLEW_OK != glewErr)
	{
		msgBox("Startup Error: Couldn't initialize GLEW.", (const char*)glewGetErrorString(glewErr));
		SDL_Quit();
		exit(1);
	}
#endif

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
				uniform = glGetUniformLocation(shader.smartScaler, "scale");
				float scale = (float)realWinWidth/(float)winWidth*0.25;
				if (scale > 1.0) scale = 1.0;
				if (uniform >= 0) glUniform1f(uniform, scale);
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

	glViewport (viewportOffsetX, viewportOffsetY, viewportWidth, viewportHeight);

	/*
	 * Set up OpenGL for 2D rendering.
	 */
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	setPixelCoords (false);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	if (restoreGraphics) {
		/*
		 * Restore the textures
		 */
		if (backdropTextureName) {
			if (!glIsTexture(backdropTextureName)) {
				glGenTextures (1, &backdropTextureName);
			}
			glBindTexture (GL_TEXTURE_2D, backdropTextureName);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			if (gameSettings.antiAlias < 0) {
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			} else {
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			}

			// Restore the backdrop
			texImage2D (GL_TEXTURE_2D, 0, GL_RGBA, sceneWidth, sceneHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, backdropTexture, backdropTextureName);

		}
		if (snapshotTextureName) {
			if (!glIsTexture(snapshotTextureName)) {
				glGenTextures (1, &snapshotTextureName);
			}
			glBindTexture (GL_TEXTURE_2D, snapshotTextureName);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			// Restore the backdrop
			texImage2D (GL_TEXTURE_2D, 0, GL_RGBA, winWidth, winHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, snapTexture, snapshotTextureName);
			delete snapTexture;
		}
		if (yTextureName) {
			if (!glIsTexture(yTextureName)) {
				glGenTextures (1, &yTextureName);
				glGenTextures (1, &uTextureName);
				glGenTextures (1, &vTextureName);
			}
		}

		reloadSpriteTextures ();
		reloadParallaxTextures ();
		zBuffer.texName = 0;
		if (zBuffer.numPanels) {
			setZBuffer (zBuffer.originalNum);
		}
		lightMap.name = 0;
		if (lightMapNumber) {
			loadLightMap(lightMapNumber);
		}

		sludgeDisplay ();
	}

	if (movieIsPlaying)
		setMovieViewport();

}

void setupOpenGLStuff() {

	/*
	 * Time to setup our requested window attributes for our OpenGL window.
	 * We want *at least* 8 bits of red, green and blue. We also want at least a 16-bit
	 * depth buffer.
	 *
	 * The last thing we do is request a double buffered window. '1' turns on double
	 * buffering, '0' turns it off.
	 */
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	setGraphicsWindow(gameSettings.userFullScreen, false);

#if !defined(HAVE_GLES2)
	/* Check for graphics capabilities... */
	if (GLEW_VERSION_2_0) {
		// Yes! Textures can be any size!
		NPOT_textures = true;
		debugOut( "OpenGL 2.0! All is good.\n");
	} else {
		if (GLEW_VERSION_1_5) {
			debugOut("OpenGL 1.5!\n");
		}
		else if (GLEW_VERSION_1_4) {
			debugOut("OpenGL 1.4!\n");
		}
		else if (GLEW_VERSION_1_3) {
			debugOut( "OpenGL 1.3!\n");
		}
		else if (GLEW_VERSION_1_2) {
			debugOut( "OpenGL 1.2!\n");
		}
		if (GLEW_ARB_texture_non_power_of_two) {
			// Yes! Textures can be any size!
			NPOT_textures = true;
		} else {
			// Workaround needed for lesser graphics cards. Let's hope this works...
			NPOT_textures = false;
			debugOut( "Warning: Old graphics card! GLEW_ARB_texture_non_power_of_two not supported.\n");
		}

		if (GLEW_ARB_shading_language_100) {
			debugOut("ARB_shading_language_100 supported.\n");
		} else {
			debugOut("Warning: Old graphics card! ARB_shading_language_100 not supported. Try updating your drivers.\n");
		}
		if (GLEW_ARB_shader_objects) {
			debugOut("ARB_shader_objects supported.\n");
		} else {
			fatal("Error: Old graphics card! ARB_shader_objects not supported.\n");
		}
		if (GLEW_ARB_vertex_shader) {
			debugOut("ARB_vertex_shader supported.\n");
		} else {
			fatal("Error: Old graphics card! ARB_vertex_shader not supported.\n");
		}
		if (GLEW_ARB_fragment_shader) {
			debugOut("ARB_fragment_shader supported.\n");
		} else {
			fatal("Error: Old graphics card! ARB_fragment_shader not supported.\n");
		}
		if (GLEW_EXT_blend_func_separate) {
			debugOut("EXT_blend_func_separate supported.\n");
		} else {
			fatal("Error: Old graphics card! EXT_blend_func_separate not supported.\n");
		}
	}
#else
	NPOT_textures = false;
#endif

	int n;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, (GLint *) &n);
	debugOut("Max texture image units: %d\n", n);


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
#if !defined(HAVE_GLES2)
		debugOut("glError in file %s @ line %d: %s\n", file, line, gluErrorString (glErr));
#else
		debugOut("glError in file %s @ line %d: error code %i\n", file, line, glErr);
#endif
		retCode = 1;
		glErr = glGetError ();
    }
	return retCode;
}
