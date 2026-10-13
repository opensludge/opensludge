#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <getopt.h>
#include <string.h>

#include <SDL3/SDL.h>

#include "posixstuff.h"
#include "platform-dependent.h"
#include "allfiles.h"
#include "language.h" // for settings
#include "debug.h"
#include "helpers.h"
#include "stringy.h"
#include "graphics.h"

extern settingsStruct gameSettings;
cmdlineSettingsStruct cmdlineSettings;

extern char **languageName;

/*
 * Functions declared in posixstuff.h:
 */

void printCmdlineUsage() {
	fprintf(stdout, "OpenSLUDGE engine, usage: sludge-engine [<options>] <gamefile name>\n\n");
	fprintf(stdout, "Options:\n");
	fprintf(stdout, "-f,		--fullscreen		Set display mode to fullscreen\n");
	fprintf(stdout, "-w,		--window		Set display mode to windowed\n");
	fprintf(stdout, "-L,		--list-languages	Print available languages and their indices\n");
	fprintf(stdout, "-l<index>,	--language=<index>	Set language to <index> (look up with -L)\n");
	fprintf(stdout, "-a<number>,	--antialias=<number>	Turn antialiasing on (1) or off (0)\n");
	fprintf(stdout, "					or choose linear interpolation (-1)\n");
	fprintf(stdout, "-d<number>,	--debug=<number>	Turn debug mode on (1) or off (0)\n");
	fprintf(stdout, "-h,		--help			Print this help message\n\n");
	fprintf(stdout, "Options are saved, so you don't need to specify them every time.\n");
	fprintf(stdout, "If you entered a wrong language number, use -l0 to reset the language to the default setting.\n");
	fprintf(stdout, "You can always toggle between fullscreen and windowed mode with \"Alt+Enter\"\n");
	fprintf(stdout, "or antialiasing on and off with \"Alt+A\". With \"Alt+C\", you open\n");
	fprintf(stdout, "the controller settings.\n");
}

void printLanguageTable() {
	if (gameSettings.numLanguages) {
		fprintf(stdout, "Index		Language\n");
		for (unsigned int i=0; i <= gameSettings.numLanguages; i++) {
			if (languageName[i]) { 
				fprintf(stdout, "%d		%s\n", i, languageName[i]);	
			} else {
				fprintf(stdout, "%d		Language %d\n", i, i);	
			}
		}
	} else {
		fprintf(stdout, "No translations available.\n");
	}
}

bool parseCmdlineParameters(int argc, char *argv[]) {
	int retval = true;
	cmdlineSettings.fullscreenSet = false;
	cmdlineSettings.languageSet = false;
	cmdlineSettings.aaSet = false;
	cmdlineSettings.debugModeSet = false;
	cmdlineSettings.listLanguages = false;
	while (1)
	{
		static struct option long_options[] =
		{
			{"fullscreen",		no_argument,	   0, 'f' },
			{"window",		no_argument,	   0, 'w' },
			{"list-languages",	no_argument,	   0, 'L' },
			{"language",		required_argument, 0, 'l' },
			{"antialias",		required_argument, 0, 'a' },
			{"debug",		required_argument, 0, 'd' },
			{"help",		no_argument,	   0, 'h' },
			{0,0,0,0} /* This is a filler for -1 */
		};
		int option_index = 0;
		int c = getopt_long (argc, argv, "fwLl:a:d:h", long_options, &option_index);
		if (c == -1) break;
			switch (c) {
		case 'f':
			cmdlineSettings.fullscreenSet = true;
			cmdlineSettings.userFullScreen = true;
			break;
		case 'w':
			cmdlineSettings.fullscreenSet = true;
			cmdlineSettings.userFullScreen = false;
			break;
		case 'L':
			cmdlineSettings.listLanguages = true;
			break;
		case 'l':
			cmdlineSettings.languageSet = true;
			cmdlineSettings.languageID = atoi(optarg);
			break;
		case 'a':
			cmdlineSettings.aaSet = true;
			cmdlineSettings.antiAlias = atoi(optarg);
			break;
		case 'd':
			cmdlineSettings.debugModeSet = true;
			cmdlineSettings.debugMode = atoi(optarg);
			break;
		case 'h':
		default:
			retval = false;
			break;
		}
	}
	return retval;
}

/*
 * Functions declared in platform-dependent.h:
 */

// We ask the player to choose a game file when none was given, for example
// when someone opens the engine from the Finder or a desktop menu.
static void SDLCALL fileChosen (void * userdata, const char * const * files, int filter) {
	char ** result = (char **) userdata;
	*result = (files && files[0]) ? copyString (files[0]) : copyString ("");
}

char * grabFileName () {
	if (! SDL_InitSubSystem (SDL_INIT_VIDEO)) return NULL;

	static const SDL_DialogFileFilter filters[] = {
		{ "SLUDGE games", "slg;dat" },
		{ "All files", "*" },
	};
	char * result = NULL;
	SDL_ShowOpenFileDialog (fileChosen, &result, NULL, filters, 2, NULL, false);
	while (! result) {
		SDL_Event event;
		SDL_WaitEventTimeout (&event, 100);
	}
	SDL_QuitSubSystem (SDL_INIT_VIDEO);

	if (! result[0]) {
		delete [] result;
		return NULL;
	}
	return result;
}

int showSetupWindow() {
	if (cmdlineSettings.listLanguages) {
		printLanguageTable();
		return 0;
	}
	if (cmdlineSettings.languageSet) {
		if (cmdlineSettings.languageID <= gameSettings.numLanguages) {
			gameSettings.languageID = cmdlineSettings.languageID;
		} else {
			fprintf(stdout, "Language index %d doesn't exist. Please specify an index between 0 and %d.\n\n",
									cmdlineSettings.languageID, gameSettings.numLanguages);
			printLanguageTable();
			return 0;
		}
	}
	if (cmdlineSettings.fullscreenSet) {
		gameSettings.userFullScreen = cmdlineSettings.userFullScreen;
	}
	if (cmdlineSettings.aaSet) {
		gameSettings.antiAlias = cmdlineSettings.antiAlias;
	}
	if (cmdlineSettings.debugModeSet) {
		gameSettings.debugMode = cmdlineSettings.debugMode;
	}
	return 1;
}

void msgBox (const char * head, const char * msg) {
	fprintf(stderr, "%s\n%s\n", head, msg);
	SDL_ShowSimpleMessageBox (SDL_MESSAGEBOX_ERROR, head, msg, sludgeWindow);
}

int msgBoxQuestion (const char * head, const char * msg) {
	const SDL_MessageBoxButtonData buttons[] = {
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Yes" },
		{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "No" },
	};
	SDL_MessageBoxData data;
	SDL_zero (data);
	data.flags = SDL_MESSAGEBOX_INFORMATION;
	data.window = sludgeWindow;
	data.title = head;
	data.message = msg;
	data.numbuttons = 2;
	data.buttons = buttons;
	int answer;
	// Without a message box, for example with no display, we answer yes.
	if (! SDL_ShowMessageBox (&data, &answer)) return 1;
	return answer == 1;
}

void changeToUserDir () {
#if defined(__APPLE__)
	// ~/Library/Application Support/SLUDGE
	char * dir = SDL_GetPrefPath ("", "SLUDGE");
	if (! dir || chdir (dir)) {
		debugOut("Error: Failed changing to directory %s\n", dir ? dir : SDL_GetError ());
	}
	SDL_free (dir);
#elif defined(__unix__)
	if (chdir (getenv ("HOME"))) {
		debugOut("Error: Failed changing to directory %s\n", getenv ("HOME"));
	}
	mkdir (".sludge-engine", 0000777);
	if (chdir (".sludge-engine")) {
		debugOut("Error: Failed changing to directory %s\n", ".sludge-engine");
	}
#else
#error "No folder for settings and saved games on this platform"
#endif
}

// Open a web page, or open a file with the program the user has chosen for
// files of its type.
bool launch(char * filename) {
	debugOut("Trying to launch: %s\n", filename);

	bool isURL = strncmp (filename, "http:", 5) == 0 || strncmp (filename, "https:", 6) == 0;
	char * url;
	if (isURL) {
		url = copyString (filename);
	} else {
		char * path = realpath (filename, NULL);
		if (! path) return false;
		url = joinStrings ("file://", path);
		free (path);
	}

	bool ok = SDL_OpenURL (url);
	if (! ok) debugOut("Launching failed: %s\n", SDL_GetError ());
	delete [] url;
	return ok;
}

bool defaultUserFullScreen() {
	return false;
}
