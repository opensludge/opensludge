/*
 * The mapping from controller buttons to mouse clicks and keys.
 *
 * A game can have a controller.ini file next to its data file, and we
 * save the changes of the player in controller.ini in the folder of the
 * game's settings. Each line of the file contains an input and its action:
 *
 *   x = key space
 *   righttrigger = key +
 *   leftshoulder = previous hotspot
 *
 * The inputs have the names of SDL gamepad mappings, so a, b, x and y are
 * the bottom, right, left and top buttons, whatever their labels are on the
 * controller. The actions are "nothing", "left click", "right click",
 * "previous hotspot", "next hotspot" and "key" with a key name, such as
 * "key escape", "key f1" or "key s".
 */

#include <stdio.h>

#include <SDL3/SDL.h>

#include "controllermap.h"
#include "debug.h"
#include "keys.h"

static const struct {
	const char * name;
	SDL_GamepadButton button;
	SDL_GamepadAxis axis;
} inputs[NUM_CONTROLLER_INPUTS] = {
	{ "a", SDL_GAMEPAD_BUTTON_SOUTH, SDL_GAMEPAD_AXIS_INVALID },
	{ "b", SDL_GAMEPAD_BUTTON_EAST, SDL_GAMEPAD_AXIS_INVALID },
	{ "x", SDL_GAMEPAD_BUTTON_WEST, SDL_GAMEPAD_AXIS_INVALID },
	{ "y", SDL_GAMEPAD_BUTTON_NORTH, SDL_GAMEPAD_AXIS_INVALID },
	{ "start", SDL_GAMEPAD_BUTTON_START, SDL_GAMEPAD_AXIS_INVALID },
	{ "leftshoulder", SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, SDL_GAMEPAD_AXIS_INVALID },
	{ "rightshoulder", SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, SDL_GAMEPAD_AXIS_INVALID },
	{ "lefttrigger", SDL_GAMEPAD_BUTTON_INVALID, SDL_GAMEPAD_AXIS_LEFT_TRIGGER },
	{ "righttrigger", SDL_GAMEPAD_BUTTON_INVALID, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER },
	{ "leftstick", SDL_GAMEPAD_BUTTON_LEFT_STICK, SDL_GAMEPAD_AXIS_INVALID },
	{ "rightstick", SDL_GAMEPAD_BUTTON_RIGHT_STICK, SDL_GAMEPAD_AXIS_INVALID },
};

static const controllerAction engineDefaults[NUM_CONTROLLER_INPUTS] = {
	{ ACTION_LEFT_CLICK, 0 },		// a
	{ ACTION_RIGHT_CLICK, 0 },		// b
	{ ACTION_KEY, ' ' },			// x
	{ ACTION_KEY, KEY_ENTER },		// y
	{ ACTION_KEY, KEY_ESCAPE },		// start
	{ ACTION_PREVIOUS_HOTSPOT, 0 },	// leftshoulder
	{ ACTION_NEXT_HOTSPOT, 0 },		// rightshoulder
	{ ACTION_NOTHING, 0 },			// lefttrigger
	{ ACTION_NOTHING, 0 },			// righttrigger
	{ ACTION_NOTHING, 0 },			// leftstick
	{ ACTION_NOTHING, 0 },			// rightstick
};

static const struct {
	controllerActionType type;
	const char * name;
} actionNames[] = {
	{ ACTION_NOTHING, "nothing" },
	{ ACTION_LEFT_CLICK, "left click" },
	{ ACTION_RIGHT_CLICK, "right click" },
	{ ACTION_PREVIOUS_HOTSPOT, "previous hotspot" },
	{ ACTION_NEXT_HOTSPOT, "next hotspot" },
};

static controllerAction defaults[NUM_CONTROLLER_INPUTS];
static controllerAction mapping[NUM_CONTROLLER_INPUTS];
static char * playerFile = NULL;

bool operator == (const controllerAction & a, const controllerAction & b) {
	return a.type == b.type && (a.type != ACTION_KEY || a.key == b.key);
}

controllerInput inputForButton (SDL_GamepadButton button) {
	for (int i = 0; i < NUM_CONTROLLER_INPUTS; i ++) {
		if (inputs[i].button == button) return (controllerInput) i;
	}
	return NUM_CONTROLLER_INPUTS;
}

controllerInput inputForAxis (SDL_GamepadAxis axis) {
	for (int i = 0; i < NUM_CONTROLLER_INPUTS; i ++) {
		if (inputs[i].axis == axis) return (controllerInput) i;
	}
	return NUM_CONTROLLER_INPUTS;
}

controllerAction actionForInput (controllerInput input) {
	return mapping[input];
}

void setActionForInput (controllerInput input, controllerAction action) {
	mapping[input] = action;
}

controllerAction defaultActionForInput (controllerInput input) {
	return defaults[input];
}

// Remove the spaces at the start and the end of a string, in place.
static char * trim (char * s) {
	while (*s == ' ' || *s == '\t') s ++;
	char * end = s + SDL_strlen (s);
	while (end > s && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n')) end --;
	*end = 0;
	return s;
}

static bool parseAction (const char * text, controllerAction * action) {
	for (unsigned i = 0; i < SDL_arraysize (actionNames); i ++) {
		if (SDL_strcasecmp (text, actionNames[i].name) == 0) {
			action -> type = actionNames[i].type;
			action -> key = 0;
			return true;
		}
	}
	if (SDL_strncasecmp (text, "key ", 4) == 0) {
		int key = keyForName (text + 4);
		if (key) {
			action -> type = ACTION_KEY;
			action -> key = key;
			return true;
		}
	}
	return false;
}

// Write the text of an action into text, which has room for size bytes.
static void formatAction (controllerAction action, char * text, size_t size) {
	if (action.type == ACTION_KEY) {
		const char * name = nameOfSpecialKey (action.key);
		if (name) {
			SDL_snprintf (text, size, "key %s", name);
			for (char * c = text; *c; c ++) *c = SDL_tolower (*c);
		} else if (action.key == ' ') {
			SDL_snprintf (text, size, "key space");
		} else {
			SDL_snprintf (text, size, "key %c", (char) action.key);
		}
		return;
	}
	for (unsigned i = 0; i < SDL_arraysize (actionNames); i ++) {
		if (actionNames[i].type == action.type) {
			SDL_snprintf (text, size, "%s", actionNames[i].name);
			return;
		}
	}
	SDL_snprintf (text, size, "nothing");
}

bool readControllerMapping (const char * filename, controllerAction into[NUM_CONTROLLER_INPUTS]) {
	FILE * fp = fopen (filename, "r");
	if (! fp) return false;

	char line[256];
	int lineNumber = 0;
	while (fgets (line, sizeof (line), fp)) {
		lineNumber ++;
		char * text = trim (line);
		if (! text[0] || text[0] == '#') continue;

		char * equals = SDL_strchr (text, '=');
		bool ok = false;
		if (equals) {
			*equals = 0;
			char * inputName = trim (text);
			char * actionText = trim (equals + 1);
			for (int i = 0; i < NUM_CONTROLLER_INPUTS; i ++) {
				if (SDL_strcasecmp (inputName, inputs[i].name) == 0) {
					ok = parseAction (actionText, &into[i]);
					break;
				}
			}
		}
		if (! ok) debugOut ("%s, line %d: we do not understand this mapping.\n", filename, lineNumber);
	}
	fclose (fp);
	return true;
}

bool writeControllerMapping (const char * filename, const controllerAction from[NUM_CONTROLLER_INPUTS]) {
	FILE * fp = fopen (filename, "w");
	if (! fp) return false;

	fprintf (fp, "# The controller buttons of this game. a, b, x and y are the bottom,\n");
	fprintf (fp, "# right, left and top buttons.\n");
	for (int i = 0; i < NUM_CONTROLLER_INPUTS; i ++) {
		char action[64];
		formatAction (from[i], action, sizeof (action));
		fprintf (fp, "%s = %s\n", inputs[i].name, action);
	}
	return fclose (fp) == 0;
}

void loadControllerMapping (const char * gameFolder, const char * playerFolder) {
	SDL_memcpy (defaults, engineDefaults, sizeof (defaults));
	char * gameFile;
	SDL_asprintf (&gameFile, "%s/controller.ini", gameFolder);
	readControllerMapping (gameFile, defaults);
	SDL_free (gameFile);

	SDL_memcpy (mapping, defaults, sizeof (mapping));
	SDL_free (playerFile);
	SDL_asprintf (&playerFile, "%s/controller.ini", playerFolder);
	readControllerMapping (playerFile, mapping);
}

static bool mappingIsDefault () {
	for (int i = 0; i < NUM_CONTROLLER_INPUTS; i ++) {
		if (mapping[i] != defaults[i]) return false;
	}
	return true;
}

void saveControllerMapping () {
	if (! playerFile) return;
	if (mappingIsDefault ()) {
		remove (playerFile);
	} else if (! writeControllerMapping (playerFile, mapping)) {
		debugOut ("We could not save the controller mapping in %s.\n", playerFile);
	}
}
