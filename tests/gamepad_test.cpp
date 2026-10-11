/*
 * Tests of playing with a game controller (source/Engine/gamepad.cpp), with
 * a virtual controller from SDL.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL3/SDL.h>

#include "allfiles.h"
#include "controllermap.h"
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

// What the gamepad code reported through setGamepadUseHandler.
static int usesReported = 0;
static bool lastUseUnmapped = false;

static void recordUse (bool unmappedInput) {
	usesReported ++;
	lastUseUnmapped = unmappedInput;
}

static void writeFile (const char * filename, const char * text) {
	FILE * fp = fopen (filename, "w");
	fputs (text, fp);
	fclose (fp);
}

static bool fileExists (const char * filename) {
	FILE * fp = fopen (filename, "r");
	if (fp) fclose (fp);
	return fp != NULL;
}

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

// SDL maps the axis values of a virtual joystick, from -32768 to 32767, to
// trigger values from 0 to 32767.
static void pullTrigger (SDL_GamepadAxis trigger) {
	SDL_SetJoystickVirtualAxis (controller, trigger, 32767);
	handleEvents ();
	SDL_SetJoystickVirtualAxis (controller, trigger, -32768);
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

	// A game folder and a player folder without controller.ini files, so we
	// start with the default mapping of the engine.
	const char * gameFolder = "gamepad_test_game";
	const char * playerFolder = "gamepad_test_player";
	SDL_CreateDirectory (gameFolder);
	SDL_CreateDirectory (playerFolder);
	remove ("gamepad_test_game/controller.ini");
	remove ("gamepad_test_player/controller.ini");
	loadControllerMapping (gameFolder, playerFolder);

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
	pressButton (SDL_GAMEPAD_BUTTON_WEST);
	check (input.keyPressed == ' ', "X is the Space key");

	setGamepadUseHandler (recordUse);
	usesReported = 0;
	pressButton (SDL_GAMEPAD_BUTTON_SOUTH);
	check (usesReported == 1 && ! lastUseUnmapped, "we report the use of a mapped button");
	pressButton (SDL_GAMEPAD_BUTTON_RIGHT_STICK);
	check (usesReported == 2 && lastUseUnmapped, "we report a button that is mapped to nothing");
	pressButton (SDL_GAMEPAD_BUTTON_TOUCHPAD);
	check (usesReported == 3 && ! lastUseUnmapped, "a button that the player cannot map does not count as unmapped");
	setGamepadUseHandler (NULL);

	// A game with its own mapping.
	writeFile ("gamepad_test_game/controller.ini",
		"# Out Of Order\n"
		"y = key s\n"
		"righttrigger = key +\n"
		"lefttrigger = KEY -\n"
		"rightstick = key f5\n"
		"nonsense = left click\n");
	loadControllerMapping (gameFolder, playerFolder);

	resetInput ();
	pressButton (SDL_GAMEPAD_BUTTON_NORTH);
	check (input.keyPressed == 's', "a game can map a button to a key");

	resetInput ();
	pullTrigger (SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
	check (input.keyPressed == '+', "a game can map a trigger to a key");

	resetInput ();
	pullTrigger (SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
	check (input.keyPressed == '-', "we ignore the case of the word key");

	resetInput ();
	pressButton (SDL_GAMEPAD_BUTTON_RIGHT_STICK);
	check (input.keyPressed == 63240, "a button can be a function key");

	resetInput ();
	pressButton (SDL_GAMEPAD_BUTTON_SOUTH);
	check (input.leftClick, "the buttons that the game does not map keep the defaults of the engine");

	// The player's mapping, on top of the game's.
	writeFile ("gamepad_test_player/controller.ini", "y = right click\n");
	loadControllerMapping (gameFolder, playerFolder);
	resetInput ();
	pressButton (SDL_GAMEPAD_BUTTON_NORTH);
	check (input.rightClick && ! input.keyPressed, "the player's mapping replaces the game's");
	resetInput ();
	pullTrigger (SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
	check (input.keyPressed == '+', "the inputs that the player does not map keep the game's mapping");

	// Saving the player's changes.
	controllerAction quickSave = { ACTION_KEY, 's' };
	setActionForInput (INPUT_NORTH, defaultActionForInput (INPUT_NORTH));
	setActionForInput (INPUT_LEFT_STICK, quickSave);
	saveControllerMapping ();
	loadControllerMapping (gameFolder, playerFolder);
	check (actionForInput (INPUT_LEFT_STICK) == quickSave && actionForInput (INPUT_NORTH) == quickSave,
		"we read back the mapping that we save");
	setActionForInput (INPUT_LEFT_STICK, defaultActionForInput (INPUT_LEFT_STICK));
	saveControllerMapping ();
	check (! fileExists ("gamepad_test_player/controller.ini"), "we remove the player's file when the mapping is the default");
	remove ("gamepad_test_game/controller.ini");
	loadControllerMapping (gameFolder, playerFolder);

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
