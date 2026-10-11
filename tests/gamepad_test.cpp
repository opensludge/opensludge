/*
 * Tests of playing with a game controller (source/Engine/gamepad.cpp), with
 * a virtual controller from SDL.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL3/SDL.h>

#include "allfiles.h"
#include "gamepad.h"
#include "objtypes.h"
#include "people.h"
#include "region.h"
#include "sludger.h"

// The engine state that we use in gamepad.cpp.
inputType input;
unsigned int winWidth = 640, winHeight = 480;
float cameraZoom = 1.0f;
int cameraX = 0, cameraY = 0;
onScreenPerson * allPeople = NULL;
screenRegion * allScreenRegions = NULL;
SDL_Window * sludgeWindow = NULL;

void debugOut (const char * a, ...) {
	va_list args;
	va_start (args, a);
	vfprintf (stderr, a, args);
	va_end (args);
}

void gameToWindowCoords (float gameX, float gameY, float * x, float * y) {
	*x = gameX;
	*y = gameY;
}

static int failures = 0;

static void check (bool condition, const char * description) {
	printf ("%s: %s\n", condition ? "ok" : "FAILED", description);
	if (! condition) failures ++;
}

static SDL_Joystick * controller;

static void handleEvents () {
	SDL_UpdateJoysticks ();
	SDL_Event event;
	while (SDL_PollEvent (&event)) handleGamepadEvent (event);
}

static void pressButton (SDL_GamepadButton button) {
	SDL_SetJoystickVirtualButton (controller, button, true);
	handleEvents ();
	SDL_SetJoystickVirtualButton (controller, button, false);
	handleEvents ();
}

static void resetInput () {
	SDL_zero (input);
	input.mouseX = 320;
	input.mouseY = 240;
}

// Hold the left stick in a position for a while, then let it go.
static void holdStick (Sint16 x, Sint16 y, int milliseconds) {
	SDL_SetJoystickVirtualAxis (controller, SDL_GAMEPAD_AXIS_LEFTX, x);
	SDL_SetJoystickVirtualAxis (controller, SDL_GAMEPAD_AXIS_LEFTY, y);
	handleEvents ();
	Uint64 end = SDL_GetTicks () + milliseconds;
	while (SDL_GetTicks () < end) {
		updateGamepadPointer ();
		SDL_Delay (10);
	}
	SDL_SetJoystickVirtualAxis (controller, SDL_GAMEPAD_AXIS_LEFTX, 0);
	SDL_SetJoystickVirtualAxis (controller, SDL_GAMEPAD_AXIS_LEFTY, 0);
	handleEvents ();
	updateGamepadPointer ();
}

int main (int argc, char * argv[]) {
	if (! SDL_Init (SDL_INIT_GAMEPAD)) {
		fprintf (stderr, "SDL_Init failed: %s\n", SDL_GetError ());
		return 1;
	}
	initGamepads ();

	SDL_VirtualJoystickDesc desc;
	SDL_INIT_INTERFACE (&desc);
	desc.type = SDL_JOYSTICK_TYPE_GAMEPAD;
	desc.naxes = SDL_GAMEPAD_AXIS_COUNT;
	desc.nbuttons = SDL_GAMEPAD_BUTTON_COUNT;
	desc.name = "Virtual test controller";
	SDL_JoystickID id = SDL_AttachVirtualJoystick (&desc);
	controller = SDL_OpenJoystick (id);
	check (controller != NULL, "we can connect a virtual controller");
	if (! controller) return 1;
	handleEvents ();
	check (SDL_GetGamepadFromID (id) != NULL, "we open a controller when it is connected");

	resetInput ();
	pressButton (SDL_GAMEPAD_BUTTON_SOUTH);
	check (input.leftClick && input.leftRelease && ! input.rightClick, "A is a left click");

	resetInput ();
	pressButton (SDL_GAMEPAD_BUTTON_EAST);
	check (input.rightClick && input.rightRelease && ! input.leftClick, "B is a right click");

	resetInput ();
	pressButton (SDL_GAMEPAD_BUTTON_START);
	check (input.keyPressed == 27, "Start is the Escape key");

	resetInput ();
	holdStick (32767, 0, 300);
	check (input.mouseX > 400 && input.mouseY == 240 && input.justMoved, "pushing the stick right moves the pointer right");

	resetInput ();
	holdStick (8000 / 4, 0, 300);
	check (input.mouseX == 320 && ! input.justMoved, "a small push inside the dead zone does not move the pointer");

	resetInput ();
	holdStick (7000, 7000, 1000);
	check (input.mouseX > 320 && input.mouseX - 320 == input.mouseY - 240, "a slight diagonal push moves the pointer diagonally");

	resetInput ();
	holdStick (-32768, -32768, 3000);
	check (input.mouseX == 0 && input.mouseY == 0, "the pointer stays inside the screen");

	// Two screen regions and a character, from left to right.
	screenRegion right = { 500, 100, 540, 140, 0, 0, 0, NULL, NULL };
	screenRegion left = { 100, 300, 140, 340, 0, 0, 0, NULL, &right };
	allScreenRegions = &left;
	objectType personType;
	SDL_zero (personType);
	onScreenPerson person;
	SDL_zero (person);
	person.x = 300;
	person.y = 400;
	person.height = 100;
	person.scale = 1;
	person.show = true;
	person.thisType = &personType;
	allPeople = &person;

	resetInput ();
	pressButton (SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
	check (input.mouseX == 520 && input.mouseY == 120, "the right shoulder moves the pointer to the next hotspot on the right");
	pressButton (SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
	check (input.mouseX == 120 && input.mouseY == 320, "after the last hotspot, the right shoulder goes back to the first");
	pressButton (SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
	check (input.mouseX == 300 && input.mouseY == 350, "the middle of a character is a hotspot");
	pressButton (SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
	check (input.mouseX == 120 && input.mouseY == 320, "the left shoulder moves the pointer to the previous hotspot");

	// With the camera moved right, the left region is outside the screen, so
	// the first hotspot is the character.
	cameraX = 200;
	resetInput ();
	pressButton (SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
	check (input.mouseX == 100 && input.mouseY == 350, "we only go to hotspots inside the screen, after moving the camera");

	SDL_CloseJoystick (controller);
	SDL_DetachVirtualJoystick (id);
	SDL_Quit ();

	printf ("%d failures\n", failures);
	return failures ? 1 : 0;
}
