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

uint32_t launch(char * launchMe) {
	fprintf(stdout, "I tried to show you something with an extern program,");
	fprintf(stdout, " but this functionality is disabled on this platform.");
	fprintf(stdout, " You might want to have a look at it:\n");
	fprintf(stdout, "%s\n", launchMe);
	return 0;
}
#endif
