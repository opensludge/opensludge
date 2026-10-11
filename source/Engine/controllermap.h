#ifndef __SLUDGE_CONTROLLERMAP_H__
#define __SLUDGE_CONTROLLERMAP_H__

#include <SDL3/SDL_gamepad.h>

// The action of a controller button.
enum controllerActionType {
	ACTION_NOTHING,
	ACTION_LEFT_CLICK,
	ACTION_RIGHT_CLICK,
	ACTION_PREVIOUS_HOTSPOT,
	ACTION_NEXT_HOTSPOT,
	ACTION_KEY,
};

struct controllerAction {
	controllerActionType type;
	int key;	// The SLUDGE key code, for ACTION_KEY.
};

bool operator == (const controllerAction & a, const controllerAction & b);
inline bool operator != (const controllerAction & a, const controllerAction & b) { return ! (a == b); }

// The buttons and triggers that the player can map. The player always moves
// the pointer with the left stick and the d-pad, and opens the controller
// settings with the Back button (View, Create or Select).
enum controllerInput {
	INPUT_SOUTH,
	INPUT_EAST,
	INPUT_WEST,
	INPUT_NORTH,
	INPUT_START,
	INPUT_LEFT_SHOULDER,
	INPUT_RIGHT_SHOULDER,
	INPUT_LEFT_TRIGGER,
	INPUT_RIGHT_TRIGGER,
	INPUT_LEFT_STICK,
	INPUT_RIGHT_STICK,
	NUM_CONTROLLER_INPUTS
};

// Return the input of a button, or NUM_CONTROLLER_INPUTS for a button that
// the player cannot map.
controllerInput inputForButton (SDL_GamepadButton button);
// Return the input of a trigger, or NUM_CONTROLLER_INPUTS for other axes.
controllerInput inputForAxis (SDL_GamepadAxis axis);

// Return the action of an input in the current mapping.
controllerAction actionForInput (controllerInput input);
void setActionForInput (controllerInput input, controllerAction action);

// Return the action of an input before the player changed it: the action
// in the controller.ini file of the game, or the default of the engine.
controllerAction defaultActionForInput (controllerInput input);

// Read the mappings in a controller.ini file, in which each line contains
// an input and its action, for example "x = key space". Return false when
// there is no file.
bool readControllerMapping (const char * filename, controllerAction into[NUM_CONTROLLER_INPUTS]);
bool writeControllerMapping (const char * filename, const controllerAction from[NUM_CONTROLLER_INPUTS]);

// Set up the mapping of a game: the defaults of the engine, then
// controller.ini in gameFolder, which is part of the game, then
// controller.ini in playerFolder, where we save the changes of the player.
void loadControllerMapping (const char * gameFolder, const char * playerFolder);

// Save the current mapping in the player's controller.ini. When the mapping
// is the same as the defaults, remove that file instead.
void saveControllerMapping ();

#endif
