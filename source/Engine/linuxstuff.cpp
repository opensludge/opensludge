#if defined __unix__ && !(defined __APPLE__)
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "platform-dependent.h"
#include "allfiles.h"
#include "language.h" // for settings

extern settingsStruct gameSettings;
extern cmdlineSettingsStruct cmdlineSettings;


void printCmdlineUsage() {
	fprintf(stdout, "OpenSLUDGE engine, usage: sludge-engine [<options>] <gamefile name>\n\n");
	fprintf(stdout, "Options:\n");
	fprintf(stdout, "-f,		--fullscreen		Set display mode to fullscreen\n");
	fprintf(stdout, "-w,		--window		Set display mode to windowed\n");
	fprintf(stdout, "-l<number>,	--language=<number>	Set language to <number> (>=0)\n\n");
	fprintf(stdout, "Options are saved, so you don't need to specify them every time.\n");
	fprintf(stdout, "If you entered a wrong language number, use -l0 to reset the language to the default setting.\n");
	fprintf(stdout, "You can always toggle between fullscreen and windowed mode with \"Alt+Enter\".\n");
}

char * grabFileName () {
	fprintf(stderr, "Game file not found.\n");
	printCmdlineUsage();
	return NULL;
}

int showSetupWindow() {
	if (cmdlineSettings.languageSet) {
		gameSettings.languageID = cmdlineSettings.languageID;
	}
	if (cmdlineSettings.fullscreenSet) {
		gameSettings.userFullScreen = cmdlineSettings.userFullScreen;
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
	chdir (getenv ("HOME"));
	mkdir (".sludge-engine", 0000777);
	chdir (".sludge-engine");
}

uint32_t launch(char * filename) {
	fprintf(stdout, "I tried to show you something with an extern program,");
	fprintf(stdout, " but this functionality is disabled on this platform.");
	fprintf(stdout, " You might want to have a look at it:\n");
	fprintf(stdout, "%s\n", filename);
	return 0;
}
#endif
