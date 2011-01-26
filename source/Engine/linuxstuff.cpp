#if defined __unix__ && !(defined __APPLE__)
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <getopt.h>

#include "linuxstuff.h"
#include "platform-dependent.h"
#include "allfiles.h"
#include "language.h" // for settings
#include "debug.h"

extern settingsStruct gameSettings;
cmdlineSettingsStruct cmdlineSettings;

/*
 * Functions declared in linuxstuff.h:
 */

void printCmdlineUsage() {
	fprintf(stdout, "OpenSLUDGE engine, usage: sludge-engine [<options>] <gamefile name>\n\n");
	fprintf(stdout, "Options:\n");
	fprintf(stdout, "-f,		--fullscreen		Set display mode to fullscreen\n");
	fprintf(stdout, "-w,		--window		Set display mode to windowed\n");
	fprintf(stdout, "-l<number>,	--language=<number>	Set language to <number> (>=0)\n");
	fprintf(stdout, "-a<number>,	--antialias=<number>	Turn antialiasing on (1) or off (0)\n");
	fprintf(stdout, "					or choose linear interpolation (-1)\n");
	fprintf(stdout, "-d<number>,	--debug=<number>	Turn debug mode on (1) or off (0)\n");
	fprintf(stdout, "-h,		--help			Print this help message\n\n");
	fprintf(stdout, "Options are saved, so you don't need to specify them every time.\n");
	fprintf(stdout, "If you entered a wrong language number, use -l0 to reset the language to the default setting.\n");
	fprintf(stdout, "You can always toggle between fullscreen and windowed mode with \"Alt+Enter\"\n");
	fprintf(stdout, "or antialiasing on and off with \"Alt+A\".\n");
}

bool parseCmdlineParameters(int argc, char *argv[]) {
	int retval = true;
	cmdlineSettings.fullscreenSet = false;
	cmdlineSettings.languageSet = false;
	cmdlineSettings.aaSet = false;
	cmdlineSettings.debugModeSet = false;
	while (1)
	{
		static struct option long_options[] =
		{
			{"fullscreen",	no_argument,	   0, 'f' },
			{"window",	no_argument,	   0, 'w' },
			{"language",	required_argument, 0, 'l' },
			{"antialias",	required_argument, 0, 'a' },
			{"debug",	required_argument, 0, 'd' },
			{"help",	no_argument,	   0, 'h' },
			{0,0,0,0} /* This is a filler for -1 */
		};
		int option_index = 0;
		char c = getopt_long (argc, argv, "fwl:a:d:h", long_options, &option_index);
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

char * grabFileName () {
	return NULL;
}

int showSetupWindow() {
	if (cmdlineSettings.languageSet) {
		gameSettings.languageID = cmdlineSettings.languageID;
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
}

int msgBoxQuestion (const char * head, const char * msg) {
	return 1;
}

void changeToUserDir () {
	if (chdir (getenv ("HOME"))) {
		debugOut("Error: Failed changing to directory %s\n", getenv ("HOME"));
	}
	mkdir (".sludge-engine", 0000777);
	if (chdir (".sludge-engine")) {
		debugOut("Error: Failed changing to directory %s\n", ".sludge-engine");
	}
}

uint32_t launch(char * filename) {
	fprintf(stdout, "I tried to show you something with an extern program,");
	fprintf(stdout, " but this functionality is disabled on this platform.");
	fprintf(stdout, " You might want to have a look at it:\n");
	fprintf(stdout, "%s\n", filename);
	return 0;
}
#endif
