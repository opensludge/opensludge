#if defined(_WIN32)
#include "winstuff.h"
#elif defined(__unix__) || defined(__APPLE__)
#include "posixstuff.h"
#else
#error "Unsupported platform"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include <exception>
#include <stdexcept>

#include <time.h>
#include <sys/time.h>

#include "sludge_gl.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "debug.h"
#include "platform-dependent.h"
#include "language.h"
#include "stringy.h"
#include "sludger.h"
#include "backdrop.h"
#include "language.h"
#include "newfatal.h"
#include "people.h"
#include "floor.h"
#include "objtypes.h"
#include "talk.h"
#include "statusba.h"
#include "transition.h"
#include "specialsettings.h"
#include "timing.h"
#include "sound.h"
#include "sludger.h"
#include "graphics.h"
#include "helpers.h"
#include "gamepad.h"
#include "pointer.h"
#include "keys.h"
#include "controllersettings.h"


#if defined(_WIN32)
#define PATHSLASH '\\'
#elif defined(__unix__) || defined(__APPLE__)
#define PATHSLASH '/'
#endif

extern bool runningFullscreen;

#ifndef MAX_PATH
#define MAX_PATH        1024          // maximum size of a path name
#endif

#if defined(_WIN32)
HWND hMainWindow = NULL;
#endif


int realWinWidth = 640, realWinHeight = 480;

extern float cameraZoom;

extern int specialSettings;
extern inputType input;
extern variableStack * noStack;

int dialogValue = 0;

char * gameName = NULL;
char * gamePath = NULL;
char *bundleFolder;

void setGameFilePath (char * f) {
	char currentDir[1000];

	if (! getcwd (currentDir, 998)) {
		debugOut("Can't get current directory.\n");
	}

	int got = -1, a;

	for (a = 0; f[a]; a ++) {
		if (f[a] == PATHSLASH) got = a;
	}

	if (got != -1) {
		f[got] = 0;
		if (chdir (f)) {
			debugOut("Error: Failed changing to directory %s\n", f);
		}
		f[got] = PATHSLASH;
	}

	gamePath = new char[400];
	if (! checkNew (gamePath)) return;

	if (! getcwd (gamePath, 398)) {
		debugOut( "Can't get game directory.\n");
	}

	if (chdir (currentDir)) {
		debugOut("Error: Failed changing to directory %s\n", currentDir);
	}
}


void saveHSI (FILE * writer);

extern bool reallyWantToQuit;

int weAreDoneSoQuit;

static void setMousePosition (float x, float y) {
	windowToGameCoords (x, y, &input.mouseX, &input.mouseY);
}

// Return the SLUDGE key code of a key that types no text, such as an arrow
// key. Return 0 for other keys, because we read their characters from text
// input events.
static int specialKeyCode (SDL_Keycode key) {
	switch (key) {
		case SDLK_BACKSPACE:
		case SDLK_DELETE:	// Ok, mapping these to the same key is weird, I admit. But good?
			return KEY_BACKSPACE;
		case SDLK_TAB:		return KEY_TAB;
		case SDLK_RETURN:
		case SDLK_KP_ENTER:	return KEY_ENTER;
		case SDLK_ESCAPE:	return KEY_ESCAPE;
		case SDLK_PAGEUP:	return KEY_PAGE_UP;
		case SDLK_PAGEDOWN:	return KEY_PAGE_DOWN;
		case SDLK_END:		return KEY_END;
		case SDLK_HOME:		return KEY_HOME;
		case SDLK_LEFT:		return KEY_LEFT;
		case SDLK_UP:		return KEY_UP;
		case SDLK_RIGHT:	return KEY_RIGHT;
		case SDLK_DOWN:		return KEY_DOWN;
		default:
			if (key >= SDLK_F1 && key <= SDLK_F12) return KEY_F1 + (key - SDLK_F1);
			return 0;
	}
}

void checkInput() {
	static bool fakeRightclick = false;
	// We set this when we handle a key combination, so that we don't pass
	// the text it types to the game, for example the å of Option+A on a Mac.
	static bool swallowText = false;

	SDL_Event event;

	/* Check for events */
	while ( SDL_PollEvent(&event) ) {
		switch (event.type) {

			case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
				updateViewport ();
				break;

			case SDL_EVENT_MOUSE_MOTION:
				input.justMoved = true;
				setMousePosition (event.motion.x, event.motion.y);
				break;

			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					if (SDL_GetModState() & SDL_KMOD_CTRL) {
						input.rightClick = true;
						fakeRightclick = true;
					} else {
						input.leftClick = true;
						fakeRightclick = false;
					}
				}
				if (event.button.button == SDL_BUTTON_RIGHT) input.rightClick = true;
				setMousePosition (event.button.x, event.button.y);
				break;

			case SDL_EVENT_MOUSE_BUTTON_UP:
				if (event.button.button == SDL_BUTTON_LEFT) {
					if (fakeRightclick) {
						fakeRightclick = false;
						input.rightRelease = true;
					} else {
						input.leftRelease = true;
					}
				}
				if (event.button.button == SDL_BUTTON_RIGHT) input.rightRelease = true;
				setMousePosition (event.button.x, event.button.y);
				break;

			case SDL_EVENT_KEY_DOWN: {
				SDL_Keycode key = event.key.key;
				SDL_Keymod mod = event.key.mod;
				swallowText = false;
#if defined(_WIN32) || defined(__unix__)
				// When the player presses a Windows key, we leave fullscreen. On
				// macOS, this key is the Command key, which is for shortcuts.
				if (runningFullscreen && (key == SDLK_LGUI || key == SDLK_RGUI)) {
					setGraphicsWindow(! runningFullscreen);
				}
#endif
				// Ignore Command keypresses - they're for the OS to handle.
				if (mod & SDL_KMOD_GUI) {
					swallowText = true;
					// Command+F - let's switch to/from full screen
					if (key == SDLK_F) {
						setGraphicsWindow(! runningFullscreen);
					}
					break;
				} else if (mod & SDL_KMOD_ALT) {
					swallowText = true;
					// Alt + Enter also switches full screen mode
					if (key == SDLK_RETURN) {
						setGraphicsWindow(! runningFullscreen);
					}
					if (key == SDLK_A) {
						gameSettings.antiAlias = ! gameSettings.antiAlias;
						break;
					}
					if (key == SDLK_C) {
						openControllerSettings ();
						break;
					}
					// Allow Alt+F4 to quit
					if (key == SDLK_F4) {
						SDL_Event quitEvent;
						SDL_zero(quitEvent);
						quitEvent.type = SDL_EVENT_QUIT;
						SDL_PushEvent(&quitEvent);
					}
					break;
				}
				int code = specialKeyCode (key);
				if (code) input.keyPressed = code;
				break;
			}

			case SDL_EVENT_TEXT_INPUT: {
				if (swallowText) break;
				// We pass the last typed character to the game, as a Unicode
				// code point.
				const char * text = event.text.text;
				Uint32 c;
				while ((c = SDL_StepUTF8 (&text, NULL)) != 0) {
					if (c != SDL_INVALID_UNICODE_CODEPOINT) input.keyPressed = c;
				}
				break;
			}

			case SDL_EVENT_GAMEPAD_ADDED:
			case SDL_EVENT_GAMEPAD_REMOVED:
			case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
			case SDL_EVENT_GAMEPAD_BUTTON_UP:
			case SDL_EVENT_GAMEPAD_AXIS_MOTION:
				if (! handleControllerSettingsEvent (event)) handleGamepadEvent (event);
				break;

			case SDL_EVENT_QUIT:
				if (reallyWantToQuit) {
					// The game file has requested that we quit
					weAreDoneSoQuit = 1;
				} else {
					// The request is from elsewhere - ask for confirmation.
					setGraphicsWindow(false);
					//fprintf (stderr, "%s %s\n", gameName, getNumberedString(2));
					if (msgBoxQuestion (gameName, getNumberedString(2))) {
						weAreDoneSoQuit = 1;
					}
				}
				break;

			default:
				break;
		}
	}
}


int main(int argc, char *argv[]) try
{
	/* Dimensions of our window. */
    winWidth = 640;
    winHeight = 480;

	char * sludgeFile;

	time_t t;
	srand((unsigned) time(&t));

	// When no game file is given, we look for one in bundleFolder. This is
	// the folder containing the executable, or the Resources folder of a
	// macOS application bundle.
	const char * basePath = SDL_GetBasePath();
	bundleFolder = copyString(basePath ? basePath : "");

#if defined(__unix__) || defined(__APPLE__)
	if (! parseCmdlineParameters(argc, argv) ) {
		printCmdlineUsage();
		return 0;
	}
	// After getopt_long, the options are before the other arguments, and
	// optind is the index of the first other argument, which is the game
	// file.
	bool haveGameFileArgument = optind < argc;
#elif defined(_WIN32)
	bool haveGameFileArgument = argc > 1;
#else
#error "No command line parsing on this platform"
#endif

	if (haveGameFileArgument) {
		sludgeFile = argv[argc - 1];
	} else {
		sludgeFile = joinStrings (bundleFolder, "gamedata.slg");
		if (! ( fileExists (sludgeFile) ) ) {
			delete sludgeFile;
			sludgeFile = joinStrings (bundleFolder, "gamedata");
			if (! ( fileExists (sludgeFile) ) ) {
				sludgeFile = grabFileName ();
			}
		}
	}

	// The player pressed cancel in the file selection dialogue,
	// so we should quit now.
	if (! sludgeFile) return 0;

	if (! fileExists(sludgeFile) ) {
		msgBox("Game file not found.", sludgeFile);
#if defined __unix__ || defined __APPLE__
		printCmdlineUsage();
#endif
		return 0;
	}

	// OK, so we DO want to start up, then...
	setGameFilePath (sludgeFile);
	if (! initSludge (sludgeFile)) return 0;

	/* Initialize the SDL library */
	if (! SDL_Init(SDL_INIT_VIDEO)) {
		msgBox("Startup Error: Couldn't initialize SDL.", SDL_GetError());
		exit(1);
	}

	setupOpenGLStuff();
	initGamepads();
	setGamepadUseHandler (noteControllerUse);

	if (gameIcon) {
		if (SDL_Surface * programIcon = SDL_CreateSurfaceFrom(iconW, iconH, SDL_PIXELFORMAT_RGBA32, gameIcon, iconW*4)) {
			SDL_SetWindowIcon(sludgeWindow, programIcon);
			SDL_DestroySurface(programIcon);
		}
		delete [] gameIcon;
		gameIcon = NULL;
	}

#ifdef _WIN32
	hMainWindow = (HWND) SDL_GetPointerProperty(SDL_GetWindowProperties(sludgeWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
#endif

	registerWindowForFatal ();


	if (! resizeBackdrop (winWidth, winHeight)) return fatal ("Couldn't allocate memory for backdrop");
	blankScreen (0, 0, winWidth, winHeight);
	if (! initPeople ()) return fatal ("Couldn't initialise people stuff");
	if (! initFloor ()) return fatal ("Couldn't initialise floor stuff");
	if (! initObjectTypes ()) return fatal ("Couldn't initialise object type stuff");
	initSpeech ();
	initStatusBar ();
	resetRandW ();

	gameName = getNumberedString(1);
		
	SDL_SetWindowTitle(sludgeWindow, gameName);

	if ( (specialSettings & (SPECIAL_MOUSE_1 | SPECIAL_MOUSE_2)) == SPECIAL_MOUSE_1) {
		//	Hide the standard mouse cursor!
		SDL_HideCursor();
	}

	if (! (specialSettings & SPECIAL_SILENT)) {
		initSoundStuff ();
	}

	startNewFunctionNum (0, 0, NULL, noStack);
	Init_Timer();

	// We read typed characters from text input events.
	SDL_StartTextInput(sludgeWindow);

	weAreDoneSoQuit = 0;
	while ( !weAreDoneSoQuit ) {

		checkInput();
		if (controllerSettingsAreOpen ()) {
			updateControllerSettings ();
			continue;
		}
		updateGamepadPointer();
		walkAllPeople ();
		handleInput ();
		sludgeDisplay ();
		Wait_Frame (movePointerBetweenFrames);

	}

	debugOut( "Bye!\n\n");

	delete [] gamePath;

	killSoundStuff ();

	/* Clean up the SDL library */
	SDL_Quit();
	displayFatal ();
	return(0);
}

catch (std::exception & ex) //NOTE by reference, not value
{
    fprintf (stderr, "std::exception caught: %s\n", ex.what ());
    return -1;
}
catch (...)
{
    fprintf (stderr, "Unknown exception was never caught\n");
    return -2;
}



