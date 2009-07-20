#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

//#include <SDL_opengl.h>
#include "glee.h"
#include <SDL.h>

//#ifndef _WIN32
// For unicode conversion
#include <iconv.h>
//#endif

#include "language.h"
#include "stringy.h"
#include "sludger.h"
#include "backdrop.h"
#include "newfatal.h"
#include "people.h"
#include "floor.h"
#include "objtypes.h"
#include "talk.h"
#include "statusba.h"
#include "transition.h"
#include "specialSettings.h"
#include "timing.h"
#include "sound.h"
#include "Sludger.h"
#include "Graphics.h"

void msgBox (const char * head, const char * msg);
int msgBoxQuestion (const char * head, const char * msg);

extern bool runningFullscreen;

#ifndef MAX_PATH
#define MAX_PATH        1024          // maximum size of a path name
#endif

HWND hMainWindow = NULL;

#ifdef _WIN32
#else
char * grabFileName ();
#endif

int realWinWidth = 640, realWinHeight = 480;

extern int specialSettings;
extern inputType input;
extern variableStack * noStack;

settingsStruct gameSettings;

int dialogValue = 0;

void fixDir (char * f) {
	int got = -1, a;
	
	for (a = 0; f[a]; a ++) {
		if (f[a] == '\\' || f[a] == '/') got = a;
	}
	
	if (got != -1) {
		f[got] = NULL;
#ifdef _MSC_VER
		_chdir (f);
#else
		chdir (f);
#endif
#ifdef _WIN32
		f[got] = '\\';
#else
		f[got] = '/';
#endif
	}
}


void tick () {	
	walkAllPeople ();
//	if (! 
		handleInput ()
		//) return
			;
	sludgeDisplay ();
}


extern bool reallyWantToQuit;

int main(int argc, char *argv[])
{
	/* Dimensions of our window. */
    winWidth = 640;
    winHeight = 480;
	
	static bool fakeRightclick = false;	
	
	int    done;
	SDL_Event event;

	char * sludgeFile;
	
	time_t t;
	srand((unsigned) time(&t));
	
	
	if (argc > 1) {
		sludgeFile = argv[1];
	} else {
		char exeFolder[MAX_PATH+1];
		strcpy (exeFolder, argv[0]);
		
		int lastSlash = -1;
		for (int i = 0; exeFolder[i]; i ++) {
			if (exeFolder[i] == '\\' || exeFolder[i] == '/') lastSlash = i;
		}
		exeFolder[lastSlash+1] = NULL;
		sludgeFile = joinStrings (exeFolder, "gamedata");
	//	fprintf(stderr, "%s", sludgeFile);
		FILE * tester = fopen (sludgeFile, "rb");
		
		if (tester) fclose (tester);
		else
#ifdef _WIN32			
			sludgeFile = grabFileName (hInstance);
#else
			sludgeFile = grabFileName ();
#endif
	}
	
	if (! sludgeFile) return 0;
	
	fixDir (sludgeFile);
		
	/* Initialize the SDL library */
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		msgBox("Startup Error: Couldn't initialize SDL.", SDL_GetError());
		exit(1);
	}
	
	// OK, so we DO want to start up, then...
	if (! initSludge (sludgeFile)) return 0;
	
	/*
	 * Now, we want to setup our requested window attributes for our OpenGL window.
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
		
	// Needed to make menu shortcuts work (on Mac), i.e. Command+Q for quit
	SDL_putenv("SDL_ENABLEAPPEVENTS=1");

	setGraphicsWindow(gameSettings.userFullScreen, false);

	/* Here's a good place to check for graphics capabilities... */
	if (false) { // && GLEE_VERSION_2_0 || GLEE_ARB_texture_non_power_of_two) {
		// Yes! Textures can be any size!
		NPOT_textures = true;
		fprintf (stderr, "Textures any size available!\n");
	} else {
		// Workaround needed for lesser graphics cards. Let's hope this works...
		NPOT_textures = false;
		fprintf (stderr, "Warning: Old graphics card!\n");
	}
		
#ifdef _WIN32
	SDL_SysWMinfo wmInfo;
	SDL_GetWMInfo(&wmInfo);
	hMainWindow = wmInfo.window;
#endif
	
	registerWindowForFatal ();	
	
	
	if (! resizeBackdrop (winWidth, winHeight)) return fatal ("Couldn't allocate memory for backdrop");
	fprintf (stderr, "Backdrop resized.\n");
	blankScreen (0, 0, winWidth, winHeight);
	if (! initPeople ()) return fatal ("Couldn't initialise people stuff");
	if (! initFloor ()) return fatal ("Couldn't initialise floor stuff");
	if (! initObjectTypes ()) return fatal ("Couldn't initialise object type stuff");
	initSpeech ();
	initStatusBar ();
	resetRandW ();

	fprintf (stderr, "Initialization done.\n");	
	
	// Let's convert the game name to Unicode, or we will crash.
	char * gameNameWin = getNumberedString(1);
	char * gameName = new char[ 1024];
	const char **tmp1 = (const char **) &gameNameWin;
	char **tmp2;
	tmp2 = &gameName;
	char * nameOrig = gameNameWin;
	char * gameNameOrig = gameName;
	
	iconv_t convert = iconv_open ("UTF-8", "CP1252");
	size_t len1 = strlen(gameNameWin)+1;
	size_t len2 = 1023;
	//size_t numChars = 
		iconv (convert, tmp1, &len1, tmp2, &len2);
	iconv_close (convert);
	
	gameNameWin = nameOrig;
	gameName = gameNameOrig;
	
	delete gameNameWin;
	
	
	SDL_WM_SetCaption(gameName, gameName);
	if ( (specialSettings & (SPECIAL_MOUSE_1 | SPECIAL_MOUSE_2)) == SPECIAL_MOUSE_1) 
		SDL_ShowCursor(SDL_DISABLE);

	if (! (specialSettings & SPECIAL_SILENT)) {
		initSoundStuff (hMainWindow);
	}
	
	startNewFunctionNum (0, 0, NULL, noStack);		
	Init_Timer();	

	SDL_EnableUNICODE(1);
	
	done = 0;
	while ( !done ) {

		/* Check for events */
		while ( SDL_PollEvent(&event) ) {
			switch (event.type) {
				/*
				case SDL_VIDEORESIZE:
					realWinWidth = event.resize.w;
					realWinHeight = event.resize.h;
					//setupRenderWindow(realWinWidth, realWinHeight);

					glViewport (viewportOffsetX, viewportOffsetY, viewportWidth, viewportHeight);	
					setPixelCoords (false);
					
					break;
*/
				case SDL_MOUSEMOTION:
					input.justMoved = true;
					input.mouseX = event.motion.x * winWidth / realWinWidth;
					input.mouseY = event.motion.y * winHeight / realWinHeight;					
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == SDL_BUTTON_LEFT) 
						if (SDL_GetModState() & KMOD_CTRL) {
							input.rightClick = true;
							fakeRightclick = true;
						} else {
							input.leftClick = true;
							fakeRightclick = false;
						}
					if (event.button.button == SDL_BUTTON_RIGHT) input.rightClick = true;
					input.mouseX = event.motion.x * winWidth / realWinWidth;
					input.mouseY = event.motion.y * winHeight / realWinHeight;					
						
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
					input.mouseX = event.motion.x * winWidth / realWinWidth;
					input.mouseY = event.motion.y * winHeight / realWinHeight;					
					break;
				case SDL_KEYDOWN:
					// Ignore Command keypresses - they're for the OS to handle.
					if (event.key.keysym.mod & KMOD_META) {
						if ('f' == event.key.keysym.unicode) {
							setGraphicsWindow(! runningFullscreen);
						}
						break;
					}
					input.keyPressed = event.key.keysym.unicode;
					break;
				case SDL_QUIT:
					if (reallyWantToQuit) {
						// The game file has requested that we quit
						done = 1;
					} else {
						// The request is from elsewhere - ask for confirmation.
						setGraphicsWindow(false);
						if (msgBoxQuestion (gameName, getNumberedString(2))) {
							done = 1;
						}
					}
					break;
				default:
					break;
			}
		}
		tick ();			
		Wait_Frame();

	}

	killSoundStuff ();
	
	/* Clean up the SDL library */
	SDL_Quit();
	displayFatal ();	
	return(0);
}
