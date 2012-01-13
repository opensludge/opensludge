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

#include "language.h" // for settings

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
int textureVertexLoc, textureTexCoordLoc;

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
		if (gameSettings.antiAlias < 0) {
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		} else {
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}

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


#ifdef _WIN32
// Replacement for glGetTexImage, because some ATI drivers are buggy.
void saveTexture (GLuint tex, GLubyte * data) {
	setPixelCoords (true);

	glBindTexture (GL_TEXTURE_2D, tex);

	GLint tw, th;

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tw);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &th);

	glEnable (GL_TEXTURE_2D);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	int xoffset = 0;
	while (xoffset < tw) {
		int w = (tw-xoffset < viewportWidth) ? tw-xoffset : viewportWidth;

		int yoffset = 0;
		while (yoffset < th) {
			int h = (th-yoffset < viewportHeight) ? th-yoffset : viewportHeight;

			glClear(GL_COLOR_BUFFER_BIT);	// Clear The Screen

			const GLint vertices[] = { 
				-xoffset, -yoffset, 0, 
				tw-xoffset, -yoffset, 0, 
				tw-xoffset, -yoffset+th, 0, 
				-xoffset, -yoffset+th, 0
			};

			const GLfloat texCoords[] = { 
				0.0f, 0.0f,
				1.0f, 0.0f,
				1.0f, 1.0f, 
				0.0f, 1.0f
			}; 
	
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

			glVertexPointer(3, GL_INT, 0, vertices);
			glTexCoordPointer(2, GL_FLOAT, 0, texCoords);

			glDrawArrays(GL_QUADS, 0, 4);

			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);

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
		videoflags = SDL_OPENGL/* | SDL_RESIZABLE*/;

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

	GLint uniform;


    const GLchar *Vertex;
    const GLchar *Fragment;

	if (gameSettings.antiAlias > 0) {

        Vertex = shaderFileRead("scale.vert");
        Fragment = shaderFileRead("scale.frag");

        if (! Vertex || ! Fragment) {
            msgBox ("Error loading \"scale\" shader program!", "Try re-installing the game. Advanced anti-aliasing is not possible. Using linear anti-aliasing instead.");
            gameSettings.antiAlias = -1;
            shader.smartScaler = 0;
        } else {

            shader.smartScaler = buildShaders (Vertex, Fragment);
			
			if (! shader.smartScaler) {
				msgBox ("Error building \"scale\" shader program!", "Advanced anti-aliasing is not possible. Using linear anti-aliasing instead.");
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
				uniform = glGetUniformLocation(shader.smartScaler, "scale");
				float scale = (float)realWinWidth/(float)winWidth*0.25;
				if (scale > 1.0) scale = 1.0;
				if (uniform >= 0) glUniform1f(uniform, scale);
			}
        }
	}

	Vertex = shaderFileRead("fixScaleSprite.vert");
	Fragment = shaderFileRead("fixScaleSprite.frag");

	if (! Vertex || ! Fragment) {
		msgBox( "Error loading \"fixScaleSprite\" shader program!", "Try re-installing the game. The game will run anyway, but some graphics may be corrupted.");
		shader.paste = 0;
	} else {

		shader.paste = buildShaders (Vertex, Fragment);
		if (! shader.paste) {
			msgBox( "Error building \"fixScaleSprite\" shader program!", "Try updating the drivers for your graphics card. If that doesn't help - sorry, your graphics card simply doesn't have all features needed for this game. It will run anyway, but some graphics may be corrupted.");
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

	if (! Vertex || ! Fragment) {
		msgBox( "Error loading \"yuv\" shader program!", "Try re-installing the game. It will run anyway, but any movies will be greyscale.");
		shader.yuv = 0;
	} else {

		shader.yuv = buildShaders (Vertex, Fragment);
		if (! shader.yuv) {
			msgBox( "Error building \"yuv\" shader program!", "Try updating the drivers for your graphics card. If that doesn't help - sorry, your graphics card simply doesn't have all features needed for this game. It will run anyway, but any movies will be greyscale.");			
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

	if (! Vertex || ! Fragment) {
		msgBox( "Error loading \"texture\" shader program!", "Try re-installing the game. The game will run anyway, but some graphics may be corrupted.");
		shader.texture = 0;
	} else {

		shader.texture = buildShaders (Vertex, Fragment);
		if (! shader.texture) {
			msgBox( "Error building \"texture\" shader program!", "Try updating the drivers for your graphics card. If that doesn't help - sorry, your graphics card simply doesn't have all features needed for this game. It will run anyway, but some graphics may be corrupted.");
		} else {
			debugOut( "Built shader program: %d (texture)\n", shader.texture);
			textureVertexLoc = glGetAttribLocation(shader.texture, "myVertex");
 			textureTexCoordLoc = glGetAttribLocation(shader.texture, "myUV");
			glUseProgram(shader.texture);
			uniform = glGetUniformLocation(shader.texture, "sampler2d");
			if (uniform >= 0) glUniform1i(uniform, 0);
		}
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

			if (gameSettings.antiAlias < 0) {
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			} else {
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			}

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
