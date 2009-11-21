//#include <SDL_opengl.h>
#include "glee.h"
#include "specialSettings.h"
#include <SDL.h>
#include "Graphics.h"
#include "sprites_AA.h"
#include "sprbanks.h"
#include "zbuffer.h"
#include "backdrop.h"

int winWidth, winHeight;
int viewportHeight, viewportWidth;
int viewportOffsetX = 0, viewportOffsetY = 0;

bool NPOT_textures = true;

extern int specialSettings;

extern GLuint backdropTextureName;
extern int sceneWidth, sceneHeight;
extern zBufferData zBuffer;
extern int lightMapNumber;


void msgBox (const char * head, const char * msg);
void sludgeDisplay ();

// This is for swapping settings between rendering to texture or to the screen
void setPixelCoords (bool pixels) {
	static int current = -1;
//	if (current == pixels) return;
	current = pixels;
	if (pixels) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, viewportWidth-1, 0, viewportHeight-1, 1.0, -1.0);

		glMatrixMode(GL_MODELVIEW);
	} else {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, winWidth-1, winHeight-1, 0, 1.0, -1.0);

		glMatrixMode(GL_MODELVIEW);
	}
}

int desktopW = 0, desktopH = 0;
bool runningFullscreen = false;

// This is for setting windowed or fullscreen graphics.
// Used for switching, and for initial window creation.
void setGraphicsWindow(bool fullscreen, bool restoreGraphics) {

	Uint32 videoflags = 0;

	if (! desktopW) {
		// Get video hardware information
		const SDL_VideoInfo* videoInfo = SDL_GetVideoInfo();
		desktopW = videoInfo->current_w;
		desktopH = videoInfo->current_h;
	} else if (restoreGraphics && fullscreen == runningFullscreen) return;

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

			glBindTexture (GL_TEXTURE_2D, backdropTextureName);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, backdropTexture);
		}
	}

	/*
	 * Set the graphics mode
	 */
	float winAspect = (float) winWidth / winHeight;

	if (fullscreen) {
		specialSettings &= ~SPECIAL_INVISIBLE;
		videoflags = SDL_OPENGL | SDL_FULLSCREEN;

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

	} else {
		videoflags = SDL_OPENGL;

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

	if( SDL_SetVideoMode( realWinWidth, realWinHeight, 32, videoflags ) == 0 ) {
		msgBox("Startup Error: Couldn't set video mode.", SDL_GetError());
		SDL_Quit();
		exit(2);
	}

	glViewport (viewportOffsetX, viewportOffsetY, viewportWidth, viewportHeight);

	/*
	 * Set up OpenGL for 2D rendering.
	 */
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	setPixelCoords (false);

	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, 0.0f);

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
			if (maxAntiAliasSettings.useMe) {
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //GL_NEAREST
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			} else {
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			}
			// Restore the backdrop
			glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, sceneWidth, sceneHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, backdropTexture);

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
