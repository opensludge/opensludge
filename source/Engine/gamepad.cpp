/*
 * Playing with a game controller.
 *
 * Players play SLUDGE games with the mouse, so we use a controller to move
 * the mouse pointer, to click and to press keys. The player moves the
 * pointer with the left stick and the d-pad. For the other buttons and the
 * triggers, we look up the action in the controller mapping
 * (controllermap.cpp) and do it, for example a left click, the Escape key,
 * or moving the pointer to the previous or next thing to click on.
 */

#include <math.h>
#include <stdlib.h>

#include <SDL3/SDL.h>

#include "allfiles.h"
#include "controllermap.h"
#include "debug.h"
#include "gamepad.h"
#include "graphics.h"
#include "objtypes.h"
#include "people.h"
#include "region.h"
#include "sludger.h"

extern inputType input;
extern float cameraZoom;
extern int cameraX, cameraY;
extern onScreenPerson * allPeople;
extern screenRegion * allScreenRegions;

// We treat stick positions closer to the centre than this as the centre.
static const float STICK_DEAD_ZONE = 0.2f;
// At full speed, the pointer crosses the screen in this many seconds.
static const float SECONDS_TO_CROSS_SCREEN = 1.0f;
// With the d-pad, the pointer moves at this fraction of full speed.
static const float DPAD_SPEED = 0.6f;

// The pointer position in game coordinates, with the fraction that we lose
// in input.mouseX and input.mouseY.
static float pointerX, pointerY;
static Uint64 lastUpdate = 0;

static float screenWidth () {
	return (float) winWidth / cameraZoom;
}

static float screenHeight () {
	return (float) winHeight / cameraZoom;
}

// Move the system pointer to the game pointer, so that the player continues
// from there with the mouse. On some platforms, such as Wayland, moving the
// system pointer is not possible.
static void moveSystemPointer () {
	if (! sludgeWindow) return;
	float windowX, windowY;
	gameToWindowCoords ((float) input.mouseX, (float) input.mouseY, &windowX, &windowY);
	SDL_WarpMouseInWindow (sludgeWindow, windowX, windowY);
}

// Put the pointer at a position in game coordinates.
static void movePointerTo (float x, float y) {
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x > screenWidth () - 1) x = screenWidth () - 1;
	if (y > screenHeight () - 1) y = screenHeight () - 1;
	pointerX = x;
	pointerY = y;
	input.mouseX = (int) x;
	input.mouseY = (int) y;
	input.justMoved = true;

	// When the game shows the system pointer instead of drawing its own, we
	// move the system pointer all the time. Otherwise, we move it when the
	// stick or d-pad stops, because after each move of the system pointer we
	// read back a mouse position rounded to the window coordinates, and we
	// would lose the fraction of a pixel in pointerX and pointerY.
	if (SDL_CursorVisible ()) moveSystemPointer ();
}

// A place that the player can click on, in game coordinates.
struct hotspot {
	float x, y;
};

static int compareHotspots (const void * a, const void * b) {
	const hotspot * h1 = (const hotspot *) a;
	const hotspot * h2 = (const hotspot *) b;
	if (h1->x != h2->x) return h1->x < h2->x ? -1 : 1;
	if (h1->y != h2->y) return h1->y < h2->y ? -1 : 1;
	return 0;
}

// Move the pointer to the next thing that the player can click on, from
// left to right, or to the previous one when forward is false. These are the
// screen regions and the characters on the screen.
static void moveToNextHotspot (bool forward) {
	int count = 0;
	for (screenRegion * r = allScreenRegions; r; r = r->next) count ++;
	for (onScreenPerson * p = allPeople; p; p = p->next) count ++;
	if (! count) return;

	hotspot * spots = new hotspot[count];
	int n = 0;
	for (screenRegion * r = allScreenRegions; r; r = r->next) {
		spots[n].x = (r->x1 + r->x2) / 2.0f - cameraX;
		spots[n].y = (r->y1 + r->y2) / 2.0f - cameraY;
		n ++;
	}
	for (onScreenPerson * p = allPeople; p; p = p->next) {
		if (! p->show || ! p->thisType) continue;
		// The middle of the character, whose position is at the feet.
		spots[n].x = p->x - cameraX;
		spots[n].y = p->y - cameraY - p->scale * (p->height - p->floaty) / 2;
		n ++;
	}

	// We only go to hotspots inside the screen.
	int onScreen = 0;
	for (int i = 0; i < n; i ++) {
		if (spots[i].x >= 0 && spots[i].y >= 0 && spots[i].x < screenWidth () && spots[i].y < screenHeight ()) {
			spots[onScreen ++] = spots[i];
		}
	}
	if (onScreen) {
		qsort (spots, onScreen, sizeof (hotspot), compareHotspots);
		hotspot current = { (float) input.mouseX, (float) input.mouseY };
		int target;
		if (forward) {
			target = 0;
			for (int i = 0; i < onScreen; i ++) {
				if (compareHotspots (&spots[i], &current) > 0) {
					target = i;
					break;
				}
			}
		} else {
			target = onScreen - 1;
			for (int i = onScreen - 1; i >= 0; i --) {
				if (compareHotspots (&spots[i], &current) < 0) {
					target = i;
					break;
				}
			}
		}
		movePointerTo (spots[target].x, spots[target].y);
		moveSystemPointer ();
	}
	delete [] spots;
}

static const Sint16 TRIGGER_DOWN = 16384;
static const Sint16 TRIGGER_UP = 8192;
static bool triggerDown[2] = { false, false };

static void (* useHandler) (bool unmappedInput) = NULL;

void setGamepadUseHandler (void (* handler) (bool unmappedInput)) {
	useHandler = handler;
}

static void reportUse (bool unmappedInput) {
	if (useHandler) useHandler (unmappedInput);
}

// Do the action of a controller button when the player presses it (down)
// or lets it go.
static void doAction (controllerAction action, bool down) {
	switch (action.type) {
		case ACTION_LEFT_CLICK:
			if (down) input.leftClick = true;
			else input.leftRelease = true;
			break;
		case ACTION_RIGHT_CLICK:
			if (down) input.rightClick = true;
			else input.rightRelease = true;
			break;
		case ACTION_PREVIOUS_HOTSPOT:
			if (down) moveToNextHotspot (false);
			break;
		case ACTION_NEXT_HOTSPOT:
			if (down) moveToNextHotspot (true);
			break;
		case ACTION_KEY:
			if (down) input.keyPressed = action.key;
			break;
		case ACTION_NOTHING:
			break;
	}
}

void initGamepads () {
	if (! SDL_InitSubSystem (SDL_INIT_GAMEPAD)) {
		debugOut ("Failed to initialise game controllers: %s\n", SDL_GetError ());
	}
}

void handleGamepadEvent (const SDL_Event & event) {
	switch (event.type) {
		case SDL_EVENT_GAMEPAD_ADDED:
			if (SDL_OpenGamepad (event.gdevice.which)) {
				debugOut ("Game controller connected: %s\n", SDL_GetGamepadNameForID (event.gdevice.which));
			}
			break;

		case SDL_EVENT_GAMEPAD_REMOVED:
			SDL_CloseGamepad (SDL_GetGamepadFromID (event.gdevice.which));
			break;

		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		case SDL_EVENT_GAMEPAD_BUTTON_UP: {
			bool down = event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN;
			controllerInput mapped = inputForButton ((SDL_GamepadButton) event.gbutton.button);
			if (mapped != NUM_CONTROLLER_INPUTS) {
				controllerAction action = actionForInput (mapped);
				if (down) reportUse (action.type == ACTION_NOTHING);
				doAction (action, down);
			} else if (down && event.gbutton.button != SDL_GAMEPAD_BUTTON_GUIDE) {
				reportUse (false);
			}
			break;
		}

		case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
			controllerInput mapped = inputForAxis ((SDL_GamepadAxis) event.gaxis.axis);
			if (mapped == NUM_CONTROLLER_INPUTS) break;
			// We treat a trigger as a button that is down when the player
			// pulls it more than halfway. It is up again when the player
			// lets it go back to less than a quarter, so that we do not press
			// the button again and again when the trigger is near halfway.
			bool & down = triggerDown[mapped == INPUT_LEFT_TRIGGER ? 0 : 1];
			if (! down && event.gaxis.value > TRIGGER_DOWN) {
				down = true;
				reportUse (actionForInput (mapped).type == ACTION_NOTHING);
				doAction (actionForInput (mapped), true);
			} else if (down && event.gaxis.value < TRIGGER_UP) {
				down = false;
				doAction (actionForInput (mapped), false);
			}
			break;
		}

		default:
			break;
	}
}

// Add the speed of the pointer from the left stick of a controller to dx
// and dy, from -1 to 1 in each direction.
static void addStickSpeed (SDL_Gamepad * gamepad, float * dx, float * dy) {
	float x = SDL_GetGamepadAxis (gamepad, SDL_GAMEPAD_AXIS_LEFTX) / 32767.0f;
	float y = SDL_GetGamepadAxis (gamepad, SDL_GAMEPAD_AXIS_LEFTY) / 32767.0f;
	// We measure the dead zone around the centre in every direction, so
	// that the pointer moves in the direction of the stick, also diagonally.
	float distance = sqrtf (x * x + y * y);
	if (distance < STICK_DEAD_ZONE) return;
	float speed = (distance - STICK_DEAD_ZONE) / (1 - STICK_DEAD_ZONE);
	if (speed > 1) speed = 1;
	// For precision, the pointer moves slowly with small stick movements.
	speed *= speed;
	*dx += x / distance * speed;
	*dy += y / distance * speed;
}

void updateGamepadPointer () {
	Uint64 now = SDL_GetTicks ();
	float seconds = lastUpdate ? (now - lastUpdate) / 1000.0f : 0;
	lastUpdate = now;
	if (seconds > 0.1f) seconds = 0.1f;

	float dx = 0, dy = 0;
	int count;
	SDL_JoystickID * ids = SDL_GetGamepads (&count);
	if (! ids) return;
	for (int i = 0; i < count; i ++) {
		SDL_Gamepad * gamepad = SDL_GetGamepadFromID (ids[i]);
		if (! gamepad) continue;
		addStickSpeed (gamepad, &dx, &dy);
		if (SDL_GetGamepadButton (gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT)) dx -= DPAD_SPEED;
		if (SDL_GetGamepadButton (gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT)) dx += DPAD_SPEED;
		if (SDL_GetGamepadButton (gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP)) dy -= DPAD_SPEED;
		if (SDL_GetGamepadButton (gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN)) dy += DPAD_SPEED;
	}
	SDL_free (ids);

	static bool moving = false;
	if (dx == 0 && dy == 0) {
		if (moving && ! SDL_CursorVisible ()) moveSystemPointer ();
		moving = false;
		return;
	}
	moving = true;
	reportUse (false);

	// Start from the mouse position, because the player may have moved the
	// mouse since the last update.
	if ((int) pointerX != input.mouseX || (int) pointerY != input.mouseY) {
		pointerX = (float) input.mouseX;
		pointerY = (float) input.mouseY;
	}
	float speed = screenWidth () / SECONDS_TO_CROSS_SCREEN * seconds;
	movePointerTo (pointerX + dx * speed, pointerY + dy * speed);
}
