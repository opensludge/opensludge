#include <SDL/SDL.h>

#include "platform-dependent.h"
#include "specialsettings.h"
#include "graphics.h"
#include "sprites_aa.h"
#include "sprbanks.h"
#include "zbuffer.h"
#include "backdrop.h"
#include "shaders.h"

#include "language.h" // for settings

int winWidth, winHeight;
int viewportHeight, viewportWidth;
int viewportOffsetX = 0, viewportOffsetY = 0;

bool NPOT_textures = true;

extern int specialSettings;
extern settingsStruct gameSettings;

extern GLuint backdropTextureName;
extern int sceneWidth, sceneHeight;
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
		if (maxAntiAliasSettings.useMe) {
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //GL_NEAREST
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		} else {
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, winWidth, winHeight, 0, 1.0, -1.0);

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

        if (gameSettings.fixedPixels) {
            viewportHeight = realWinWidth = winWidth;
            viewportWidth = realWinHeight = winHeight;
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
            viewportHeight = realWinWidth = winWidth;
            viewportWidth = realWinHeight = winHeight;
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
	fprintf (stderr, "Video mode %d %d set successfully.\n", realWinWidth, realWinHeight);

	const GLchar VertexFixScaleSprite[] =
		"void main() {"
		"	gl_TexCoord[0] = gl_MultiTexCoord0;"
		"	gl_TexCoord[1] = gl_MultiTexCoord1;"
		"	gl_TexCoord[2] = gl_MultiTexCoord2;"
		"	gl_FrontColor = gl_Color;"
		"	gl_FrontSecondaryColor = gl_SecondaryColor;"
		"	gl_Position = ftransform();"
		"}";
	const GLchar FragmentFixScaleSprite[] =
		"uniform sampler2D tex0;"
		"uniform sampler2D tex1;"
		"uniform sampler2D tex2;"
		"uniform bool useLightTexture;"
		"void main()"
		"{"
		"	vec4 texture = texture2D (tex0, gl_TexCoord[0].xy);"
		"	vec4 texture2 = texture2D (tex2, gl_TexCoord[2].xy);"
		"	vec3 col;"
		"	if (useLightTexture) {"
		"		vec4 texture1 = texture2D (tex1, gl_TexCoord[1].xy);"
		"		col = texture1.rgb * texture.rgb;"
		"	} else {"
		"		col = gl_Color.rgb * texture.rgb;"
		"	}"
		"	col += vec3(gl_SecondaryColor);"
		"	vec4 color = vec4 (col, gl_Color.a * texture.a);"
		"	col = mix (texture2.rgb, color.rgb, color.a);"
		"	gl_FragColor = vec4 (col, max(texture.a, texture2.a));"
		"}";
	
	shader.fixScaleSprite = buildShaders (VertexFixScaleSprite, FragmentFixScaleSprite);
	fprintf (stderr, "Built shader program: %d (fixScaleSprite)\n", shader.fixScaleSprite);
	glUseProgram(shader.fixScaleSprite);
	GLint uniform = glGetUniformLocation(shader.fixScaleSprite, "tex0");
	if (uniform >= 0) glUniform1i(uniform, 0);
	uniform = glGetUniformLocation(shader.fixScaleSprite, "tex1");
	if (uniform >= 0) glUniform1i(uniform, 1);
	uniform = glGetUniformLocation(shader.fixScaleSprite, "tex2");
	if (uniform >= 0) glUniform1i(uniform, 2);
	uniform = glGetUniformLocation(shader.fixScaleSprite, "useLightTexture");
	if (uniform >= 0) glUniform1i(uniform, 0);

	glUseProgram(0);



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
		fprintf (stderr, "OpenGL 2.0! All is good.\n");
	} else {
		if (GLEE_VERSION_1_5) {
			fprintf (stderr, "OpenGL 1.5!\n");
		}
		else if (GLEE_VERSION_1_4) {
			fprintf (stderr, "OpenGL 1.4!\n");
		}
		else if (GLEE_VERSION_1_3) {
			fprintf (stderr, "OpenGL 1.3!\n");
		}
		else if (GLEE_VERSION_1_2) {
			fprintf (stderr, "OpenGL 1.2!\n");
		}
		if (GLEE_ARB_texture_non_power_of_two) {
			// Yes! Textures can be any size!
			NPOT_textures = true;
		} else {
			// Workaround needed for lesser graphics cards. Let's hope this works...
			NPOT_textures = false;
			fprintf (stderr, "Warning: Old graphics card! GLEE_ARB_texture_non_power_of_two not supported.\n");
		}

		if (GLEE_ARB_shading_language_100) {
			fprintf (stderr, "ARB_shading_language_100 supported.\n");
		} else {
			fprintf (stderr, "Warning: Old graphics card! ARB_shading_language_100 not supported. Try updating your drivers.\n");
		}
		if (GLEE_ARB_shader_objects) {
			fprintf (stderr, "ARB_shader_objects supported.\n");
		} else {
			fprintf (stderr, "Warning: Old graphics card! ARB_shader_objects not supported.\n");
		}
		if (GLEE_ARB_vertex_shader) {
			fprintf (stderr, "ARB_vertex_shader supported.\n");
		} else {
			fprintf (stderr, "Warning: Old graphics card! ARB_vertex_shader not supported.\n");
		}
		if (GLEE_ARB_fragment_shader) {
			fprintf (stderr, "ARB_fragment_shader supported.\n");
		} else {
			fprintf (stderr, "Warning: Old graphics card! ARB_fragment_shader not supported.\n");
		}
	}

	int n;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, (GLint *) &n);
	fprintf (stderr, "Max texture image units: %d\n", n);


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
