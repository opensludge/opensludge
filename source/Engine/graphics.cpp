#include <SDL/SDL.h>

#include "debug.h"
#include "platform-dependent.h"
#include "specialsettings.h"
#include "graphics.h"
#include "language.h"
#include "sprbanks.h"
#include "zbuffer.h"
#include "backdrop.h"
#include "shaders.h"

#include "language.h" // for settings

unsigned int winWidth, winHeight;
int viewportHeight, viewportWidth;
int viewportOffsetX = 0, viewportOffsetY = 0;

extern float cameraZoom;

bool NPOT_textures = true;

extern int specialSettings;

extern GLuint backdropTextureName;
extern GLuint snapshotTextureName;
extern unsigned int sceneWidth, sceneHeight;
extern zBufferData zBuffer;
extern int lightMapNumber;

shaders shader;

void sludgeDisplay ();


// This is for swapping settings between rendering to texture or to the screen
void setPixelCoords (bool pixels) {
	static int current = -1;
//	if (current == pixels) return;
	current = pixels;

	glBindTexture (GL_TEXTURE_2D, backdropTextureName);

	if (pixels) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, viewportWidth, 0, viewportHeight, 1.0, -1.0);

		glMatrixMode(GL_MODELVIEW);

		glClear(GL_COLOR_BUFFER_BIT);
	} else {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		GLdouble w = (GLdouble) winWidth / cameraZoom;
		GLdouble h = (GLdouble) winHeight / cameraZoom;

		glOrtho(0, w, h, 0, 1.0, -1.0);

//		glOrtho(0, winWidth, winHeight, 0, 1.0, -1.0);

		glMatrixMode(GL_MODELVIEW);
	}
}

int desktopW = 0, desktopH = 0;
bool runningFullscreen = false;

// This is for setting windowed or fullscreen graphics.
// Used for switching, and for initial window creation.
void setGraphicsWindow(bool fullscreen, bool restoreGraphics) {

	GLubyte *snapTexture = NULL;

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
		if (snapshotTextureName) {
			int picWidth = winWidth;
			int picHeight = winHeight;
			if (! NPOT_textures) {
				picWidth = getNextPOT(picWidth);
				picHeight = getNextPOT(picHeight);
			}
			snapTexture = new GLubyte [picHeight*picWidth*4];

			glBindTexture (GL_TEXTURE_2D, snapshotTextureName);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, snapTexture);
		}
	}

	/*
	 * Set the graphics mode
	 */
	float winAspect = (float) winWidth / winHeight;

	if (fullscreen) {
		specialSettings &= ~SPECIAL_INVISIBLE;
		videoflags = SDL_OPENGL | SDL_FULLSCREEN;

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
		videoflags = SDL_OPENGL;

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

	if( SDL_SetVideoMode( realWinWidth, realWinHeight, 32, videoflags ) == 0 ) {
		msgBox("Startup Error: Couldn't set video mode.", SDL_GetError());
		SDL_Quit();
		exit(2);
	}
	debugOut( "Video mode %d %d set successfully.\n", realWinWidth, realWinHeight);

	GLint uniform;

	const GLchar *Vertex = shaderFileRead("scale.vert");
	const GLchar *Fragment = shaderFileRead("scale.frag");

	if (! Vertex || ! Fragment) {
		debugOut("Error loading shader program!\n");
		shader.smartScaler = 0;
	} else {

		shader.smartScaler = buildShaders (Vertex, Fragment);
		debugOut( "Built shader program: %d (smartScaler)\n", shader.smartScaler);
		glUseProgram(shader.smartScaler);
		uniform = glGetUniformLocation(shader.smartScaler, "Texture");
		if (uniform >= 0) glUniform1i(uniform, 0);
		uniform = glGetUniformLocation(shader.smartScaler, "lightTexture");
		if (uniform >= 0) glUniform1i(uniform, 1);
		uniform = glGetUniformLocation(shader.smartScaler, "useLightTexture");
		if (uniform >= 0) glUniform1i(uniform, 0);
		uniform = glGetUniformLocation(shader.smartScaler, "scale");
		float scale = (float)realWinWidth/(float)winWidth*0.25;
		if (scale > 1.0) scale = 1.0;
		if (uniform >= 0) glUniform1f(uniform, scale);

	}
	Vertex = shaderFileRead("fixScaleSprite.vert");
	Fragment = shaderFileRead("fixScaleSprite.frag");

	if (! Vertex || ! Fragment) {
		debugOut( "Error loading shader program!\n");
		shader.paste = 0;
	} else {

		shader.paste = buildShaders (Vertex, Fragment);
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

	glUseProgram(0);



	glViewport (viewportOffsetX, viewportOffsetY, viewportWidth, viewportHeight);

	/*
	 * Set up OpenGL for 2D rendering.
	 */
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			// Restore the backdrop
			glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, sceneWidth, sceneHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, backdropTexture);

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
			glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, winWidth, winHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, snapTexture);
			delete snapTexture;
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

	/* Check for graphics capabilities... */
	if (GLEE_VERSION_2_0) {
		// Yes! Textures can be any size!
		NPOT_textures = true;
		debugOut( "OpenGL 2.0! All is good.\n");
	} else {
		if (GLEE_VERSION_1_5) {
			debugOut("OpenGL 1.5!\n");
		}
		else if (GLEE_VERSION_1_4) {
			debugOut("OpenGL 1.4!\n");
		}
		else if (GLEE_VERSION_1_3) {
			debugOut( "OpenGL 1.3!\n");
		}
		else if (GLEE_VERSION_1_2) {
			debugOut( "OpenGL 1.2!\n");
		}
		if (GLEE_ARB_texture_non_power_of_two) {
			// Yes! Textures can be any size!
			NPOT_textures = true;
		} else {
			// Workaround needed for lesser graphics cards. Let's hope this works...
			NPOT_textures = false;
			debugOut( "Warning: Old graphics card! GLEE_ARB_texture_non_power_of_two not supported.\n");
		}

		if (GLEE_ARB_shading_language_100) {
			debugOut("ARB_shading_language_100 supported.\n");
		} else {
			debugOut("Warning: Old graphics card! ARB_shading_language_100 not supported. Try updating your drivers.\n");
		}
		if (GLEE_ARB_shader_objects) {
			debugOut("ARB_shader_objects supported.\n");
		} else {
			debugOut("Warning: Old graphics card! ARB_shader_objects not supported.\n");
		}
		if (GLEE_ARB_vertex_shader) {
			debugOut("ARB_vertex_shader supported.\n");
		} else {
			debugOut("Warning: Old graphics card! ARB_vertex_shader not supported.\n");
		}
		if (GLEE_ARB_fragment_shader) {
			debugOut("ARB_fragment_shader supported.\n");
		} else {
			debugOut("Warning: Old graphics card! ARB_fragment_shader not supported.\n");
		}
	}

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
