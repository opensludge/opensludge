#if defined __unix__ && !(defined __APPLE__)
#include "linuxstuff.h"
#endif

#ifdef _WIN32
#include "winstuff.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>

#include <time.h>
#include <sys/time.h>
#include "GLee.h"
#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>

// For unicode conversion
#include <iconv.h>

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


#ifdef _WIN32
#define PATHSLASH '\\'
#else
#define PATHSLASH '/'
#endif

extern bool runningFullscreen;

#ifndef MAX_PATH
#define MAX_PATH        1024          // maximum size of a path name
#endif

HWND hMainWindow = NULL;


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
		f[got] = NULL;
		if (chdir (f)) {
			debugOut("Error: Failed changing to directory %s\n", f);
		}
		f[got] = PATHSLASH;
	}

	gamePath = new char[400];

	if (! getcwd (gamePath, 398)) {
		debugOut( "Can't get game directory.\n");
	}

	if (chdir (currentDir)) {
		debugOut("Error: Failed changing to directory %s\n", currentDir);
	}
}


void saveHSI (FILE * writer);

extern bool reallyWantToQuit;

bool fileExists(char * file) {
	FILE * tester;
	bool retval = false;
	tester = fopen (file, "rb");
	if (tester) {
		retval = true;
		fclose (tester);
	}
	return retval;
}

#ifdef _WIN32
#undef main
#endif

int weAreDoneSoQuit;

void checkInput() {
	static bool fakeRightclick = false;
	
	SDL_Event event;

	/* Check for events */
	while ( SDL_PollEvent(&event) ) {
		switch (event.type) {
				
			case SDL_VIDEORESIZE:
				realWinWidth = event.resize.w;
				realWinHeight = event.resize.h;
				setGraphicsWindow(false, true, true);
				break;
				
			case SDL_MOUSEMOTION:
				input.justMoved = true;
				input.mouseX = event.motion.x * ((float)winWidth/cameraZoom) / realWinWidth;
				input.mouseY = event.motion.y * ((float)winHeight/cameraZoom) / realWinHeight;
				break;
				
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					if (SDL_GetModState() & KMOD_CTRL) {
						input.rightClick = true;
						fakeRightclick = true;
					} else {
						input.leftClick = true;
						fakeRightclick = false;
					}
				}
				if (event.button.button == SDL_BUTTON_RIGHT) input.rightClick = true;
				input.mouseX = event.motion.x * ((float)winWidth/cameraZoom) / realWinWidth;
				input.mouseY = event.motion.y * ((float)winHeight/cameraZoom) / realWinHeight;
				break;
				
			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_LEFT) {
					if (fakeRightclick) {
						fakeRightclick = false;
						input.rightRelease = true;
					} else {
						input.leftRelease = true;
					}
				}
				if (event.button.button == SDL_BUTTON_RIGHT) input.rightRelease = true;
				input.mouseX = event.motion.x * ((float)winWidth/cameraZoom) / realWinWidth;
				input.mouseY = event.motion.y * ((float)winHeight/cameraZoom) / realWinHeight;
				break;
				
			case SDL_KEYDOWN:
				// A Windows key is pressed - let's leave fullscreen.
				if (runningFullscreen) {
					if (event.key.keysym.sym == SDLK_LSUPER || event.key.keysym.sym == SDLK_LSUPER) {
						setGraphicsWindow(! runningFullscreen);
					}
				}
				// Ignore Command keypresses - they're for the OS to handle.
				if (event.key.keysym.mod & KMOD_META) {
					// Command+F - let's switch to/from full screen
					if ('f' == event.key.keysym.unicode) {
						setGraphicsWindow(! runningFullscreen);
					}
					break;
				} else if (event.key.keysym.mod & KMOD_ALT) {
					// Alt + Enter also switches full screen mode
					if (SDLK_RETURN == event.key.keysym.sym) {
						setGraphicsWindow(! runningFullscreen);
					}
					if (SDLK_a == event.key.keysym.sym) {
						gameSettings.antiAlias = ! gameSettings.antiAlias;
						break;
					}
					// Allow Alt+F4 to quit
					if (SDLK_F4 ==  event.key.keysym.sym) {
						SDL_Event event;
						event.type = SDL_QUIT;
						SDL_PushEvent(&event);
					}
					
					break;
				}
				switch (event.key.keysym.sym) {
					case SDLK_BACKSPACE:
					case SDLK_DELETE:	// Ok, mapping these to the same key is weird, I admit. But good?
						input.keyPressed = 127; break;
					case SDLK_TAB:
						input.keyPressed = 9; break;
					case SDLK_RETURN:
						input.keyPressed = 13; break;
					case SDLK_ESCAPE:
						input.keyPressed = 27; break;
					case SDLK_PAGEUP:
						input.keyPressed = 63276; break;
					case SDLK_PAGEDOWN:
						input.keyPressed = 63277; break;
					case SDLK_END:
						input.keyPressed = 63275; break;
					case SDLK_HOME:
						input.keyPressed = 63273; break;
					case SDLK_LEFT:
						input.keyPressed = 63234; break;
					case SDLK_UP:
						input.keyPressed = 63232; break;
					case SDLK_RIGHT:
						input.keyPressed = 63235; break;
					case SDLK_DOWN:
						input.keyPressed = 63233; break;
					case SDLK_F1:
						input.keyPressed = 63236; break;
					case SDLK_F2:
						input.keyPressed = 63237; break;
					case SDLK_F3:
						input.keyPressed = 63238; break;
					case SDLK_F4:
						input.keyPressed = 63239; break;
					case SDLK_F5:
						input.keyPressed = 63240; break;
					case SDLK_F6:
						input.keyPressed = 63241; break;
					case SDLK_F7:
						input.keyPressed = 63242; break;
					case SDLK_F8:
						input.keyPressed = 63243; break;
					case SDLK_F9:
						input.keyPressed = 63244; break;
					case SDLK_F10:
						input.keyPressed = 63245; break;
					case SDLK_F11:
						input.keyPressed = 63246; break;
					case SDLK_F12:
						input.keyPressed = 63247; break;
					default:
						input.keyPressed = event.key.keysym.unicode;
						break;
				}
				break;
				
			case SDL_QUIT:
				if (reallyWantToQuit) {
					// The game file has requested that we quit
					weAreDoneSoQuit = 1;
				} else {
					// The request is from elsewhere - ask for confirmation.
					setGraphicsWindow(false);
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

	// bundleFolder is used to look for the game file
	// and later to find the shader programs
#ifdef __APPLE__
	// bundleFolder is set in applicationDidFinishLaunching.
#elif defined __unix__
	bundleFolder = copyString(DATADIR);	// DATADIR is defined in the Makefile.
#else
	bundleFolder = copyString(argv[0]);
	int lastSlash = -1;
	for (int i = 0; bundleFolder[i]; i ++) {
		if (bundleFolder[i] == PATHSLASH) lastSlash = i;
	}
	bundleFolder[lastSlash+1] = NULL;
#endif

	if (argc > 1) {
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

#if defined __unix__ && !(defined __APPLE__)
	if (! parseCmdlineParameters(argc, argv) ) {
		printCmdlineUsage();
		return 0;
	}
	if (! fileExists(sludgeFile) ) {
		fprintf(stderr, "Game file not found.\n");
		printCmdlineUsage();
		return 0;
	}
#endif

	// The player pressed cancel in the file selection dialogue,
	// so we should quit now.
	if (! sludgeFile) return 0;

	// OK, so we DO want to start up, then...
	setGameFilePath (sludgeFile);
	if (! initSludge (sludgeFile)) return 0;

	/* Initialize the SDL library */
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		msgBox("Startup Error: Couldn't initialize SDL.", SDL_GetError());
		exit(1);
	}

    if (gameIcon) {
        if (SDL_Surface * programIcon = SDL_CreateRGBSurfaceFrom(gameIcon, iconW, iconH, 32, iconW*4, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000)) {
            SDL_WM_SetIcon(programIcon, NULL);
            SDL_FreeSurface(programIcon);
        }
        delete gameIcon;
    }

	// Needed to make menu shortcuts work (on Mac), i.e. Command+Q for quit
	SDL_putenv((char *)"SDL_ENABLEAPPEVENTS=1");

	setupOpenGLStuff();


#ifdef _WIN32
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWMInfo(&wmInfo);
	hMainWindow = wmInfo.window;
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

	// Let's convert the game name to Unicode, or we will crash.
	char * gameNameWin = getNumberedString(1);
	gameName = new char[ 1024];
	char **tmp1 = (char **) &gameNameWin;
	char **tmp2;
	tmp2 = &gameName;
	char * nameOrig = gameNameWin;
	char * gameNameOrig = gameName;

	iconv_t convert = iconv_open ("UTF-8", "CP1252");
	size_t len1 = strlen(gameNameWin)+1;
	size_t len2 = 1023;
	//size_t numChars =
#ifdef _WIN32
	iconv (convert,(const char **) tmp1, &len1, tmp2, &len2);
#else
	iconv (convert,(char **) tmp1, &len1, tmp2, &len2);
#endif
	iconv_close (convert);

	gameNameWin = nameOrig;
	gameName = gameNameOrig;

	delete gameNameWin;
	gameNameWin = NULL;

	SDL_WM_SetCaption(gameName, gameName);

	if ( (specialSettings & (SPECIAL_MOUSE_1 | SPECIAL_MOUSE_2)) == SPECIAL_MOUSE_1) {
	//	Hide the standard mouse cursor!
	// This is done in a weird way because there's bugs with SDL_ShowCursor(SDL_DISABLE);
        SDL_Cursor *cursor = NULL;
        Uint8 data = 0;
        SDL_FreeCursor(cursor);
        cursor = SDL_CreateCursor(&data, &data, 1, 1, 0, 0);
        SDL_SetCursor(cursor);
    }

	fprintf (stderr, "About to init sound...");
	if (! (specialSettings & SPECIAL_SILENT)) {
		initSoundStuff (hMainWindow);
		fprintf (stderr, "done!");
	}
	fprintf(stderr, "\n");

	startNewFunctionNum (0, 0, NULL, noStack);
	Init_Timer();

	SDL_EnableUNICODE(1);

	weAreDoneSoQuit = 0;
	while ( !weAreDoneSoQuit ) {

		checkInput();
		walkAllPeople ();
		handleInput ();
		sludgeDisplay ();
		Wait_Frame();

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
    std::cerr << "std::exception caught: " << ex.what() << std::endl;
    return -1;
}
catch (...)
{
    std::cerr << "Unknown exception was never caught" << std::endl;
    return -2;
}



