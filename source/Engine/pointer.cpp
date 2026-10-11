/*
 * Moving the mouse pointer between game frames.
 *
 * Each game has its own frame rate, for example 18 frames per second in Out
 * Of Order, and we draw the mouse cursor of the game in each frame. So that
 * the pointer moves smoothly, we keep a copy of each game frame without the
 * cursor.
 * Between game frames, when the pointer moves, we show that copy again with
 * the cursor at the new position, at most once per refresh of the display.
 */

#include <SDL3/SDL.h>

#include "sludge_gl.h"

#include "allfiles.h"
#include "cursors.h"
#include "gamepad.h"
#include "graphics.h"
#include "movie.h"
#include "pointer.h"
#include "sludger.h"

extern inputType input;
extern float cameraZoom;

// The last game frame without the cursor, and the size of the viewport when
// we saved it.
static GLuint frameTexture = 0;
static int frameTextureWidth = 0, frameTextureHeight = 0;
static int frameWidth, frameHeight;
static bool haveFrame = false;

// The cursor position in the frame that the player sees, and when we
// showed that frame.
static int shownX, shownY;
static Uint64 shownAt = 0;

// The last mouse position that we read, in window coordinates.
static float lastMouseX = -1, lastMouseY = -1;

void saveFrameWithoutCursor () {
	if (! frameTexture) glGenTextures (1, &frameTexture);
	glBindTexture (GL_TEXTURE_2D, frameTexture);

	int width = NPOT_textures ? viewportWidth : getNextPOT (viewportWidth);
	int height = NPOT_textures ? viewportHeight : getNextPOT (viewportHeight);
	if (width != frameTextureWidth || height != frameTextureHeight) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		frameTextureWidth = width;
		frameTextureHeight = height;
	}
	glCopyTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, viewportOffsetX, viewportOffsetY, viewportWidth, viewportHeight);

	frameWidth = viewportWidth;
	frameHeight = viewportHeight;
	haveFrame = true;
	shownX = input.mouseX;
	shownY = input.mouseY;
	shownAt = SDL_GetTicksNS ();
}

void forgetFrameWithoutCursor () {
	haveFrame = false;
}

bool drawFrameWithoutCursor () {
	glClear (GL_COLOR_BUFFER_BIT);
	if (! haveFrame) return false;

	glDisable (GL_DEPTH_TEST);
	glDisable (GL_BLEND);
	setPixelCoords (false);

	GLfloat w = (GLfloat) winWidth / cameraZoom;
	GLfloat h = (GLfloat) winHeight / cameraZoom;
	const GLfloat vertices[] = {
		0, 0, 0,
		w, 0, 0,
		0, h, 0,
		w, h, 0
	};
	// The rows that we copied into the texture start at the bottom of the
	// window, so we draw the texture upside down.
	GLfloat tw = (GLfloat) frameWidth / frameTextureWidth;
	GLfloat th = (GLfloat) frameHeight / frameTextureHeight;
	const GLfloat texCoords[] = {
		0, th,
		tw, th,
		0, 0,
		tw, 0
	};
	glUseProgram (shader.texture);
	setPMVMatrix (shader.texture);
	glBindTexture (GL_TEXTURE_2D, frameTexture);
	drawQuad (shader.texture, vertices, 1, texCoords);
	glUseProgram (0);
	return true;
}

// Return the time between two refreshes of the display, in nanoseconds.
static Uint64 refreshInterval () {
	const SDL_DisplayMode * mode = SDL_GetCurrentDisplayMode (SDL_GetDisplayForWindow (sludgeWindow));
	float rate = (mode && mode -> refresh_rate > 0) ? mode -> refresh_rate : 60.0f;
	return (Uint64) (SDL_NS_PER_SECOND / rate);
}

void movePointerBetweenFrames (Uint32 millisecondsLeft) {
	if (movieIsPlaying) return;

	// We read the latest position of the mouse and of the controllers. We
	// leave the events in the queue for checkInput.
	SDL_PumpEvents ();
	updateGamepadPointer ();

	if (! haveFrame || ! hasCursorSprite ()) return;
	// After a change to the window size, we wait for the next game frame.
	if (frameWidth != viewportWidth || frameHeight != viewportHeight) return;
	if (SDL_GetMouseFocus () == sludgeWindow) {
		float x, y;
		SDL_GetMouseState (&x, &y);
		if (x != lastMouseX || y != lastMouseY) {
			lastMouseX = x;
			lastMouseY = y;
			windowToGameCoords (x, y, &input.mouseX, &input.mouseY);
		}
	}
	if (input.mouseX == shownX && input.mouseY == shownY) return;

	// With vertical sync, we wait for the next refresh of the display when
	// we show a frame, so we only show one when the next game frame is due
	// after that refresh.
	Uint64 interval = refreshInterval ();
	Uint64 now = SDL_GetTicksNS ();
	if (now - shownAt < interval) return;
	if ((Uint64) millisecondsLeft * SDL_NS_PER_MS < interval) return;

	drawFrameWithoutCursor ();
	drawCursor (input.mouseX, input.mouseY);
	showFrame ();

	shownX = input.mouseX;
	shownY = input.mouseY;
	shownAt = SDL_GetTicksNS ();
}
