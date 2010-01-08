#ifdef __linux__
#include "ALLFILES.H"
#include <iostream>
#include <fstream>

char * grabFileName () {
	return NULL;
}

int showSetupWindow() {
	return true;
}

void msgBox (const char * head, const char * msg) {
	fprintf(stderr, "%s\n%s\n", head, msg);
}

int msgBoxQuestion (const char * head, const char * msg) {
	return true;
}

#endif
