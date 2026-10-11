/*
 * The controller settings, in which the player chooses the action of each
 * controller button.
 *
 * We draw the settings over the last game frame, darkened, with the font
 * and the status bar colours of the game, so that they look like a part of
 * the game. The game does not run while the settings are open. The player
 * opens them with the Back button of the controller (View, Create, Select
 * or Minus) or with Alt+C (Option+C on a Mac), and moves through them with
 * the d-pad, the left stick, the arrow keys or the mouse.
 *
 * So that players find the settings, we show a hint about how to open them
 * the first time that the player uses a controller, when the player
 * presses an unmapped button, and while the game is frozen, for example in
 * its menu, after the player has used a controller.
 */

#include <ctype.h>

#include <SDL3/SDL.h>

#include "sludge_gl.h"

#include "allfiles.h"
#include "controllermap.h"
#include "controllersettings.h"
#include "cursors.h"
#include "graphics.h"
#include "keys.h"
#include "movie.h"
#include "pointer.h"
#include "sludger.h"
#include "sprites.h"
#include "fonttext.h"
#include "timing.h"

extern inputType input;
extern float cameraZoom;
extern int fontHeight;
extern spritePalette verbLinePalette, litVerbLinePalette;
struct frozenStuffStruct;
extern frozenStuffStruct * frozenStuff;

// The rows of the settings: one row for each input, then these two.
enum {
	ROW_RESET = NUM_CONTROLLER_INPUTS,
	ROW_DONE,
	NUM_ROWS
};

// When the player holds a direction, we move once, and then again and
// again after these times, in milliseconds.
static const Uint64 REPEAT_DELAY = 400;
static const Uint64 REPEAT_INTERVAL = 120;
static const float STICK_THRESHOLD = 0.5f;
static const Uint64 FIRST_USE_HINT_MILLISECONDS = 8000;
static const Uint64 NOTHING_HINT_MILLISECONDS = 5000;

static bool settingsOpen = false;
static int selectedRow = 0;
static int firstVisibleRow = 0;
static bool waitingForKey = false;
static bool mouseUsed = false;

enum direction { NO_DIRECTION, UP, DOWN, LEFT, RIGHT };
static direction heldDirection = NO_DIRECTION;
static Uint64 nextRepeat = 0;

// The controller buttons and mouse button that the player pressed while the
// settings were open. We act when the player lets a button go, so that we
// do not pass to the game the release of the button with which the player
// closed the settings.
static Uint32 buttonsPressed = 0;
static bool mousePressed = false;

// The hint about how to open the settings.
enum hintReason { NO_HINT, HINT_FIRST_USE, HINT_UNMAPPED_BUTTON };
static hintReason pendingHint = NO_HINT;
static hintReason shownHint = NO_HINT;
static Uint64 hintUntil = 0;
static bool controllerUsed = false;
static bool controllerUsedSinceConnect = false;

// The actions that the player can choose with left and right.
static controllerAction choices[NUM_CONTROLLER_INPUTS * 2 + 8];
static int numChoices = 0;

static spritePalette textPalette, selectedPalette;

bool controllerSettingsAreOpen () {
	return settingsOpen;
}

// Labels of controller buttons

enum controllerFamily { XBOX_360, XBOX_ONE, PLAYSTATION_3, PLAYSTATION_4, PLAYSTATION_5, NINTENDO, OTHER_CONTROLLER };

// The labels of the inputs that are not face buttons, and the name of the
// Back button, for each family of controllers.
static const struct {
	const char * start, * leftShoulder, * rightShoulder, * leftTrigger, * rightTrigger;
	const char * leftStick, * rightStick, * back;
} labels[] = {
	{ "Start", "LB", "RB", "LT", "RT", "LS", "RS", "Back" },					// XBOX_360
	{ "Menu", "LB", "RB", "LT", "RT", "LS", "RS", "View" },					// XBOX_ONE
	{ "Start", "L1", "R1", "L2", "R2", "L3", "R3", "Select" },				// PLAYSTATION_3
	{ "Options", "L1", "R1", "L2", "R2", "L3", "R3", "Share" },				// PLAYSTATION_4
	{ "Options", "L1", "R1", "L2", "R2", "L3", "R3", "Create" },			// PLAYSTATION_5
	{ "Plus", "L", "R", "ZL", "ZR", "Left stick", "Right stick", "Minus" },	// NINTENDO
	{ "Start", "LB", "RB", "LT", "RT", "LS", "RS", "Back" },					// OTHER_CONTROLLER
};

// Return the type of the first connected controller.
static SDL_GamepadType controllerType () {
	SDL_GamepadType type = SDL_GAMEPAD_TYPE_STANDARD;
	int count;
	SDL_JoystickID * ids = SDL_GetGamepads (&count);
	if (ids && count) type = SDL_GetGamepadTypeForID (ids[0]);
	SDL_free (ids);
	return type;
}

static controllerFamily familyOfType (SDL_GamepadType type) {
	switch (type) {
		case SDL_GAMEPAD_TYPE_XBOX360:	return XBOX_360;
		case SDL_GAMEPAD_TYPE_XBOXONE:	return XBOX_ONE;
		case SDL_GAMEPAD_TYPE_PS3:		return PLAYSTATION_3;
		case SDL_GAMEPAD_TYPE_PS4:		return PLAYSTATION_4;
		case SDL_GAMEPAD_TYPE_PS5:		return PLAYSTATION_5;
		case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO:
		case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT:
		case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT:
		case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
			return NINTENDO;
		default:						return OTHER_CONTROLLER;
	}
}

// Return the label of a face button on the connected controller, such as
// "A" or "Cross".
static const char * faceButtonLabel (SDL_GamepadButton button) {
	switch (SDL_GetGamepadButtonLabelForType (controllerType (), button)) {
		case SDL_GAMEPAD_BUTTON_LABEL_A:		return "A";
		case SDL_GAMEPAD_BUTTON_LABEL_B:		return "B";
		case SDL_GAMEPAD_BUTTON_LABEL_X:		return "X";
		case SDL_GAMEPAD_BUTTON_LABEL_Y:		return "Y";
		case SDL_GAMEPAD_BUTTON_LABEL_CROSS:	return "Cross";
		case SDL_GAMEPAD_BUTTON_LABEL_CIRCLE:	return "Circle";
		case SDL_GAMEPAD_BUTTON_LABEL_SQUARE:	return "Square";
		case SDL_GAMEPAD_BUTTON_LABEL_TRIANGLE:	return "Triangle";
		default:
			// The positions of A, B, X and Y on an Xbox controller.
			switch (button) {
				case SDL_GAMEPAD_BUTTON_SOUTH:	return "A";
				case SDL_GAMEPAD_BUTTON_EAST:	return "B";
				case SDL_GAMEPAD_BUTTON_WEST:	return "X";
				default:						return "Y";
			}
	}
}

static const char * inputLabel (controllerInput in) {
	controllerFamily family = familyOfType (controllerType ());
	switch (in) {
		case INPUT_SOUTH:			return faceButtonLabel (SDL_GAMEPAD_BUTTON_SOUTH);
		case INPUT_EAST:			return faceButtonLabel (SDL_GAMEPAD_BUTTON_EAST);
		case INPUT_WEST:			return faceButtonLabel (SDL_GAMEPAD_BUTTON_WEST);
		case INPUT_NORTH:			return faceButtonLabel (SDL_GAMEPAD_BUTTON_NORTH);
		case INPUT_START:			return labels[family].start;
		case INPUT_LEFT_SHOULDER:	return labels[family].leftShoulder;
		case INPUT_RIGHT_SHOULDER:	return labels[family].rightShoulder;
		case INPUT_LEFT_TRIGGER:	return labels[family].leftTrigger;
		case INPUT_RIGHT_TRIGGER:	return labels[family].rightTrigger;
		case INPUT_LEFT_STICK:		return labels[family].leftStick;
		case INPUT_RIGHT_STICK:		return labels[family].rightStick;
		default:					return "";
	}
}

// Write the keyboard shortcut for the settings into text. In a font
// without a plus sign, we join the keys with a hyphen.
static void keyboardShortcut (char * text, size_t size) {
#if defined(__APPLE__)
	const char * modifier = "Option";
#elif defined(_WIN32) || defined(__unix__)
	const char * modifier = "Alt";
#else
#error "No name for the Alt key on this platform"
#endif
	SDL_snprintf (text, size, "%s%sC", modifier, fontHasCharacter ('+') ? "+" : "-");
}

// Names of actions

static const struct {
	int key;
	const char * name;
} punctuationNames[] = {
	{ ' ', "Space" }, { '+', "Plus" }, { '-', "Minus" }, { '=', "Equals" },
	{ '.', "Full stop" }, { ',', "Comma" }, { '/', "Slash" }, { '*', "Star" },
};

// Write the name of an action, for the player, into text.
static void actionName (controllerAction action, char * text, size_t size) {
	switch (action.type) {
		case ACTION_NOTHING:			SDL_snprintf (text, size, "Nothing"); return;
		case ACTION_LEFT_CLICK:			SDL_snprintf (text, size, "Left click"); return;
		case ACTION_RIGHT_CLICK:		SDL_snprintf (text, size, "Right click"); return;
		case ACTION_PREVIOUS_HOTSPOT:	SDL_snprintf (text, size, "Previous hotspot"); return;
		case ACTION_NEXT_HOTSPOT:		SDL_snprintf (text, size, "Next hotspot"); return;
		case ACTION_KEY:				break;
	}

	const char * special = nameOfSpecialKey (action.key);
	if (special) {
		// For example, we write "PAGE UP" as "Page Up key".
		char name[32];
		SDL_strlcpy (name, special, sizeof (name));
		for (int i = 1; name[i]; i ++) {
			if (name[i - 1] != ' ') name[i] = tolower (name[i]);
		}
		SDL_snprintf (text, size, "%s key", name);
		return;
	}
	for (unsigned i = 0; i < SDL_arraysize (punctuationNames); i ++) {
		if (punctuationNames[i].key == action.key) {
			SDL_snprintf (text, size, "%s key", punctuationNames[i].name);
			return;
		}
	}
	if (action.key < 128 && isprint (action.key)) {
		SDL_snprintf (text, size, "%c key", toupper (action.key));
	} else {
		SDL_snprintf (text, size, "Key %d", action.key);
	}
}

// Text in the game font

// Copy text into fitted, without the characters that are not in the font of
// the game. For a letter that is in the font only in the other case, we use
// that case.
static void fitToFont (const char * text, char * fitted, size_t size) {
	size_t n = 0;
	for (const char * c = text; *c && n + 1 < size; c ++) {
		if (fontHasCharacter ((unsigned char) *c)) {
			fitted[n ++] = *c;
		} else if (isalpha ((unsigned char) *c) && fontHasCharacter (toupper (*c))) {
			fitted[n ++] = toupper (*c);
		} else if (isalpha ((unsigned char) *c) && fontHasCharacter (tolower (*c))) {
			fitted[n ++] = tolower (*c);
		}
	}
	fitted[n] = 0;
}

// The size of the text that we draw, as a fraction of the size of the font.
static float textScale = 1;

// Draw the next text at a fraction of the size of the font.
static void setTextScale (float scale) {
	textScale = scale;
	setGameMatrix (scale);
}

static int textWidth (const char * text) {
	char fitted[256];
	fitToFont (text, fitted, sizeof (fitted));
	return (int) (stringWidth (fitted) * textScale);
}

// Draw text with its left end at x, on the line at y, in window units.
static void drawText (const char * text, int x, int y, spritePalette & palette) {
	char fitted[256];
	fitToFont (text, fitted, sizeof (fitted));
	float units = cameraZoom * textScale;
	pasteString (fitted, (int) (x / units), (int) (y / units), palette);
}

static void drawCentredText (const char * text, int y, spritePalette & palette) {
	drawText (text, ((int) winWidth - textWidth (text)) / 2, y, palette);
}

// Draw the first text that fits in the width of the window, so that we can
// give a shorter text for a narrow window or a large font.
static void drawFittingText (const char * const texts[], int count, int y, spritePalette & palette) {
	for (int i = 0; i < count; i ++) {
		if (i == count - 1 || textWidth (texts[i]) <= (int) winWidth * 9 / 10) {
			drawCentredText (texts[i], y, palette);
			return;
		}
	}
}

// Use the colours of the status bar of the game. When the game has the
// same colour for the selected status, we use gold for the selected row.
static void preparePalettes () {
	setFontColour (textPalette, verbLinePalette.originalRed, verbLinePalette.originalGreen, verbLinePalette.originalBlue);
	if (litVerbLinePalette.originalRed == verbLinePalette.originalRed &&
			litVerbLinePalette.originalGreen == verbLinePalette.originalGreen &&
			litVerbLinePalette.originalBlue == verbLinePalette.originalBlue) {
		setFontColour (selectedPalette, 255, 210, 60);
	} else {
		setFontColour (selectedPalette, litVerbLinePalette.originalRed, litVerbLinePalette.originalGreen, litVerbLinePalette.originalBlue);
	}
	fixFont (textPalette);
	fixFont (selectedPalette);
}

// Draw a rectangle in window units, blended with the frame.
static void drawRectangle (int x1, int y1, int x2, int y2, float r, float g, float b, float a) {
	const GLfloat vertices[] = {
		x1 / cameraZoom, y1 / cameraZoom, 0,
		x2 / cameraZoom, y1 / cameraZoom, 0,
		x1 / cameraZoom, y2 / cameraZoom, 0,
		x2 / cameraZoom, y2 / cameraZoom, 0
	};
	glEnable (GL_BLEND);
	glUseProgram (shader.color);
	setPMVMatrix (shader.color);
	setPrimaryColor (r, g, b, a);
	drawQuad (shader.color, vertices, 0);
	glUseProgram (0);
	glDisable (GL_BLEND);
}

// Layout

struct layout {
	int rowHeight;
	int titleY;			// The line of the title text.
	int firstRowTop;
	int visibleRows;
	int hintY;			// The line of the hint at the bottom.
	int labelX, actionX;
};

static layout currentLayout () {
	layout l;
	// We make the rows lower than the font, down to three quarters of its
	// height, when every row then fits in the window. Most fonts have space
	// above and below the letters.
	l.rowHeight = (int) winHeight / (NUM_ROWS + 4);
	if (l.rowHeight > fontHeight + 2) l.rowHeight = fontHeight + 2;
	if (l.rowHeight < fontHeight * 3 / 4) l.rowHeight = fontHeight * 3 / 4;
	// A title row and a hint row, with half a row between them and the list.
	int available = (int) winHeight - 4 * l.rowHeight;
	l.visibleRows = available / l.rowHeight;
	if (l.visibleRows > NUM_ROWS) l.visibleRows = NUM_ROWS;
	if (l.visibleRows < 1) l.visibleRows = 1;
	int total = (l.visibleRows + 3) * l.rowHeight;
	int top = ((int) winHeight - total) / 2;
	l.titleY = top + l.rowHeight;
	l.firstRowTop = top + l.rowHeight * 3 / 2;
	l.hintY = l.firstRowTop + (l.visibleRows + 1) * l.rowHeight + l.rowHeight / 2;
	if (l.hintY > (int) winHeight - 2) l.hintY = (int) winHeight - 2;
	l.labelX = (int) winWidth / 8;
	l.actionX = (int) winWidth * 2 / 5;
	return l;
}

// Return the row at a height in window units, or -1.
static int rowAt (const layout & l, int y) {
	if (y < l.firstRowTop) return -1;
	int row = firstVisibleRow + (y - l.firstRowTop) / l.rowHeight;
	if (row >= firstVisibleRow + l.visibleRows || row >= NUM_ROWS) return -1;
	return row;
}

static void scrollToSelection (const layout & l) {
	if (selectedRow < firstVisibleRow) firstVisibleRow = selectedRow;
	if (selectedRow >= firstVisibleRow + l.visibleRows) firstVisibleRow = selectedRow - l.visibleRows + 1;
}

// Choices

static void addChoice (controllerAction action) {
	for (int i = 0; i < numChoices; i ++) {
		if (choices[i] == action) return;
	}
	if (numChoices < (int) SDL_arraysize (choices)) choices[numChoices ++] = action;
}

// The actions that the player can choose with left and right: the clicks,
// the hotspots, some common keys, and the keys in the mapping and in the
// defaults of the game.
static void makeChoices () {
	numChoices = 0;
	const controllerAction common[] = {
		{ ACTION_NOTHING, 0 },
		{ ACTION_LEFT_CLICK, 0 },
		{ ACTION_RIGHT_CLICK, 0 },
		{ ACTION_PREVIOUS_HOTSPOT, 0 },
		{ ACTION_NEXT_HOTSPOT, 0 },
		{ ACTION_KEY, KEY_ESCAPE },
		{ ACTION_KEY, ' ' },
		{ ACTION_KEY, KEY_ENTER },
	};
	for (unsigned i = 0; i < SDL_arraysize (common); i ++) addChoice (common[i]);
	for (int i = 0; i < NUM_CONTROLLER_INPUTS; i ++) {
		addChoice (defaultActionForInput ((controllerInput) i));
		addChoice (actionForInput ((controllerInput) i));
	}
}

// Change the action of the selected input to the next or previous choice.
static void changeAction (int step) {
	if (selectedRow >= NUM_CONTROLLER_INPUTS) return;
	controllerInput in = (controllerInput) selectedRow;
	int current = 0;
	for (int i = 0; i < numChoices; i ++) {
		if (choices[i] == actionForInput (in)) current = i;
	}
	setActionForInput (in, choices[(current + step + numChoices) % numChoices]);
}

// Opening, closing and moving through the settings

void openControllerSettings () {
	if (settingsOpen || movieIsPlaying || ! fontHasCharacter ('A')) return;
	settingsOpen = true;
	selectedRow = 0;
	firstVisibleRow = 0;
	waitingForKey = false;
	mouseUsed = false;
	heldDirection = NO_DIRECTION;
	buttonsPressed = 0;
	mousePressed = false;
	pendingHint = NO_HINT;
	hintUntil = 0;
	makeChoices ();
}

static void closeSettings () {
	saveControllerMapping ();
	settingsOpen = false;
	waitingForKey = false;
	// We do not pass to the game the input that we used in the settings.
	input.leftClick = input.rightClick = false;
	input.leftRelease = input.rightRelease = false;
	input.keyPressed = 0;
	input.justMoved = true;
	// We start counting the time of game frames again from now.
	Init_Timer ();
}

static void moveSelection (int step) {
	selectedRow = (selectedRow + step + NUM_ROWS) % NUM_ROWS;
}

// Do what the player chose with A, Enter or a click on the selected row.
static void activateSelectedRow () {
	if (selectedRow == ROW_DONE) {
		closeSettings ();
	} else if (selectedRow == ROW_RESET) {
		for (int i = 0; i < NUM_CONTROLLER_INPUTS; i ++) {
			setActionForInput ((controllerInput) i, defaultActionForInput ((controllerInput) i));
		}
	} else {
		waitingForKey = true;
	}
}

// Leave the key choice, or close the settings, with B or Escape.
static void goBack () {
	if (waitingForKey) {
		waitingForKey = false;
	} else {
		closeSettings ();
	}
}

static void moveInDirection (direction d) {
	if (waitingForKey) return;
	switch (d) {
		case UP:	moveSelection (-1); break;
		case DOWN:	moveSelection (1); break;
		case LEFT:	changeAction (-1); break;
		case RIGHT:	changeAction (1); break;
		default:	break;
	}
}

void noteControllerUse (bool unmappedInput) {
	controllerUsed = true;
	if (! controllerUsedSinceConnect) {
		controllerUsedSinceConnect = true;
		pendingHint = HINT_FIRST_USE;
	}
	if (unmappedInput) pendingHint = HINT_UNMAPPED_BUTTON;
}

bool handleControllerSettingsEvent (const SDL_Event & event) {
	switch (event.type) {
		case SDL_EVENT_GAMEPAD_ADDED:
			controllerUsedSinceConnect = false;
			return false;

		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
			if (! settingsOpen) {
				if (event.gbutton.button != SDL_GAMEPAD_BUTTON_BACK) return false;
				openControllerSettings ();
				return true;
			}
			buttonsPressed |= 1u << event.gbutton.button;
			return true;

		case SDL_EVENT_GAMEPAD_BUTTON_UP: {
			Uint32 bit = 1u << event.gbutton.button;
			if (! (buttonsPressed & bit)) {
				// The player lets go of Back after opening the settings.
				return settingsOpen || event.gbutton.button == SDL_GAMEPAD_BUTTON_BACK;
			}
			buttonsPressed &= ~bit;
			if (! settingsOpen) return true;
			switch (event.gbutton.button) {
				case SDL_GAMEPAD_BUTTON_SOUTH:	if (! waitingForKey) activateSelectedRow (); break;
				case SDL_GAMEPAD_BUTTON_EAST:	goBack (); break;
				case SDL_GAMEPAD_BUTTON_BACK:	closeSettings (); break;
				default:						break;
			}
			return true;
		}

		case SDL_EVENT_GAMEPAD_AXIS_MOTION:
			return settingsOpen;

		default:
			return false;
	}
}

// Return the direction that the player holds on any controller, with the
// d-pad or the left stick.
static direction heldControllerDirection () {
	direction d = NO_DIRECTION;
	int count;
	SDL_JoystickID * ids = SDL_GetGamepads (&count);
	for (int i = 0; ids && i < count && d == NO_DIRECTION; i ++) {
		SDL_Gamepad * gamepad = SDL_GetGamepadFromID (ids[i]);
		if (! gamepad) continue;
		float x = SDL_GetGamepadAxis (gamepad, SDL_GAMEPAD_AXIS_LEFTX) / 32767.0f;
		float y = SDL_GetGamepadAxis (gamepad, SDL_GAMEPAD_AXIS_LEFTY) / 32767.0f;
		if (SDL_GetGamepadButton (gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP) || y < -STICK_THRESHOLD) d = UP;
		else if (SDL_GetGamepadButton (gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN) || y > STICK_THRESHOLD) d = DOWN;
		else if (SDL_GetGamepadButton (gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT) || x < -STICK_THRESHOLD) d = LEFT;
		else if (SDL_GetGamepadButton (gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT) || x > STICK_THRESHOLD) d = RIGHT;
	}
	SDL_free (ids);
	return d;
}

static void handleControllerDirection () {
	direction d = heldControllerDirection ();
	Uint64 now = SDL_GetTicks ();
	if (d != heldDirection) {
		heldDirection = d;
		nextRepeat = now + REPEAT_DELAY;
		moveInDirection (d);
	} else if (d != NO_DIRECTION && now >= nextRepeat) {
		nextRepeat = now + REPEAT_INTERVAL;
		moveInDirection (d);
	}
}

static void handleKeyboardAndMouse (const layout & l) {
	if (waitingForKey && input.keyPressed) {
		if (input.keyPressed != KEY_ESCAPE) {
			controllerAction action = { ACTION_KEY, input.keyPressed };
			setActionForInput ((controllerInput) selectedRow, action);
			addChoice (action);
		}
		waitingForKey = false;
	} else {
		switch (input.keyPressed) {
			case KEY_UP:		moveInDirection (UP); break;
			case KEY_DOWN:		moveInDirection (DOWN); break;
			case KEY_LEFT:		moveInDirection (LEFT); break;
			case KEY_RIGHT:		moveInDirection (RIGHT); break;
			case KEY_ENTER:		activateSelectedRow (); break;
			case KEY_ESCAPE:	goBack (); break;
			default:			break;
		}
	}

	if (input.justMoved) {
		mouseUsed = true;
		int row = rowAt (l, (int) (input.mouseY * cameraZoom));
		if (row >= 0 && ! waitingForKey) selectedRow = row;
	}
	if (input.leftClick || input.rightClick) mousePressed = true;
	if (mousePressed && (input.leftRelease || input.rightRelease) && ! waitingForKey && settingsOpen) {
		mousePressed = false;
		int row = rowAt (l, (int) (input.mouseY * cameraZoom));
		if (input.leftRelease && row >= 0) {
			selectedRow = row;
			if (row < NUM_CONTROLLER_INPUTS) changeAction (1);
			else activateSelectedRow ();
		} else if (input.rightRelease && row >= 0 && row < NUM_CONTROLLER_INPUTS) {
			selectedRow = row;
			changeAction (-1);
		}
	}

	input.leftClick = input.rightClick = false;
	input.leftRelease = input.rightRelease = false;
	input.keyPressed = 0;
	input.justMoved = false;
}

// Drawing

static void drawHint () {
	layout l = currentLayout ();
	char texts[3][160];
	const char * accept = faceButtonLabel (SDL_GAMEPAD_BUTTON_SOUTH);
	const char * back = faceButtonLabel (SDL_GAMEPAD_BUTTON_EAST);
	if (waitingForKey) {
		const char * label = inputLabel ((controllerInput) selectedRow);
		SDL_snprintf (texts[0], 160, "Press a key on the keyboard for %s, or %s to cancel", label, back);
		SDL_snprintf (texts[1], 160, "Press a key for %s", label);
		SDL_snprintf (texts[2], 160, "Press a key");
	} else if (selectedRow < NUM_CONTROLLER_INPUTS) {
		SDL_snprintf (texts[0], 160, "Left or right to change, %s to choose a key", accept);
		SDL_snprintf (texts[1], 160, "Left or right to change");
		SDL_snprintf (texts[2], 160, "Change");
	} else if (selectedRow == ROW_RESET) {
		SDL_snprintf (texts[0], 160, "Give every button its first action again");
		SDL_snprintf (texts[1], 160, "Back to the first actions");
		SDL_snprintf (texts[2], 160, "Reset");
	} else {
		SDL_snprintf (texts[0], 160, "Save and go back to the game");
		SDL_snprintf (texts[1], 160, "Back to the game");
		SDL_snprintf (texts[2], 160, "Done");
	}
	const char * const options[] = { texts[0], texts[1], texts[2] };
	drawFittingText (options, 3, l.hintY, textPalette);
}

static void drawSettings () {
	layout l = currentLayout ();
	scrollToSelection (l);

	drawFrameWithoutCursor ();
	setPixelCoords (false);
	drawRectangle (0, 0, winWidth, winHeight, 0, 0, 0, 0.75f);
	preparePalettes ();

	drawCentredText ("Controller", l.titleY, selectedPalette);

	for (int i = 0; i < l.visibleRows; i ++) {
		int row = firstVisibleRow + i;
		if (row >= NUM_ROWS) break;
		int top = l.firstRowTop + i * l.rowHeight;
		int line = top + (l.rowHeight + fontHeight) / 2;
		bool selected = row == selectedRow;
		if (selected) {
			drawRectangle (winWidth / 16, top, winWidth - winWidth / 16, top + l.rowHeight, 1, 1, 1, 0.12f);
		}
		spritePalette & palette = selected ? selectedPalette : textPalette;
		if (row < NUM_CONTROLLER_INPUTS) {
			controllerInput in = (controllerInput) row;
			drawText (inputLabel (in), l.labelX, line, palette);
			char action[64];
			if (selected && waitingForKey) {
				SDL_snprintf (action, sizeof (action), "...");
			} else {
				actionName (actionForInput (in), action, sizeof (action));
			}
			drawText (action, l.actionX, line, palette);
		} else if (row == ROW_RESET) {
			drawCentredText ("Reset", line, palette);
		} else {
			drawCentredText ("Done", line, palette);
		}
	}

	drawHint ();
	if (mouseUsed) drawCursor (input.mouseX, input.mouseY);
	showFrame ();
}

void updateControllerSettings () {
	layout l = currentLayout ();
	handleControllerDirection ();
	handleKeyboardAndMouse (l);
	if (! settingsOpen) return;
	drawSettings ();
	// With vertical sync, we wait for the display when we show the frame.
	// Without it, we wait 5 milliseconds here, so that we do not draw
	// frames as fast as possible.
	SDL_Delay (5);
}

void drawControllerSettingsHint () {
	if (! fontHasCharacter ('A')) return;

	Uint64 now = SDL_GetTicks ();
	if (pendingHint != NO_HINT) {
		shownHint = pendingHint;
		pendingHint = NO_HINT;
		hintUntil = now + (shownHint == HINT_FIRST_USE ? FIRST_USE_HINT_MILLISECONDS : NOTHING_HINT_MILLISECONDS);
	}
	bool timed = now < hintUntil;
	bool frozen = controllerUsed && frozenStuff;
	if (! timed && ! frozen) return;

	// We say "on your controller" and "on your keyboard", because the name
	// of a button, such as Share, could also be something in the game.
	char shortcut[16], controller[96], shortController[96], keyboard[64];
	keyboardShortcut (shortcut, sizeof (shortcut));
	const char * back = labels[familyOfType (controllerType ())].back;
	SDL_snprintf (controller, sizeof (controller), "To change the controller buttons, press %s on your controller", back);
	SDL_snprintf (shortController, sizeof (shortController), "Controller settings: %s on your controller", back);
	SDL_snprintf (keyboard, sizeof (keyboard), "or %s on your keyboard.", shortcut);

	// We give each line in a long and a short form.
	const char * lines[3][2];
	int numLines = 0;
	if (timed && shownHint == HINT_UNMAPPED_BUTTON) {
		lines[numLines][0] = lines[numLines][1] = "You can give this button an action.";
		numLines ++;
	}
	lines[numLines][0] = controller;
	lines[numLines][1] = shortController;
	numLines ++;
	lines[numLines][0] = lines[numLines][1] = keyboard;
	numLines ++;

	// We draw the hint smaller than the text of the game, but at least 12
	// pixels high in game coordinates, and we darken the top of the frame
	// behind it, so that it is easy to read on any background.
	float scale = 0.55f;
	if (fontHeight * scale < 12) scale = fontHeight > 12 ? 12.0f / fontHeight : 1.0f;
	int lineHeight = (int) (fontHeight * scale);
	preparePalettes ();
	setPixelCoords (false);
	drawRectangle (0, 0, winWidth, lineHeight * numLines + lineHeight / 2, 0, 0, 0, 0.5f);
	setTextScale (scale);
	for (int i = 0; i < numLines; i ++) {
		drawFittingText (lines[i], 2, lineHeight * (i + 1), textPalette);
	}
	setTextScale (1);
	setPixelCoords (false);
}
