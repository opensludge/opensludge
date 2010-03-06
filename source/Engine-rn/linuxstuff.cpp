#ifdef __linux__
#include "ALLFILES.H"
#include <iostream>
#include <fstream>

#include "Language.h" // for settings

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

#endif
