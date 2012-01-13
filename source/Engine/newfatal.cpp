#include <SDL/SDL.h>

#include "platform-dependent.h"
#include "allfiles.h"
#include "version.h"

#include <string.h>
#include <stdlib.h>

#include "sound.h"

#include "stringy.h"
#include "errors.h"
#include "graphics.h"

const char emergencyMemoryMessage[]	= "Out of memory displaying error message!";

static char * fatalMessage = NULL;
static char * fatalInfo = joinStrings ("Initialisation error! Something went wrong before we even got started!", "");

extern int numResourceNames /* = 0*/;
extern char * * allResourceNames /*= NULL*/;

int resourceForFatal = -1;

const char * resourceNameFromNum (int i) {
	if (i == -1) return NULL;
	if (numResourceNames == 0) return "RESOURCE";
	if (i < numResourceNames) return allResourceNames[i];
	return "Unknown resource";
}

bool hasFatal () {
	if (fatalMessage) return true;
	return false;
}

void displayFatal () {
	if (fatalMessage) {
		msgBox ("SLUDGE v"TEXT_VERSION" fatal error!", fatalMessage);
	}
}

void warning (const char * l) {
	setGraphicsWindow(false);
	msgBox ("SLUDGE v"TEXT_VERSION" non-fatal indigestion report", l);
}

void registerWindowForFatal () {
	delete fatalInfo;
	fatalInfo = joinStrings ("There's an error with this SLUDGE game! If you're designing this game, please turn on verbose error messages in the project manager and recompile. If not, please contact the author saying where and how this problem occured.", "");
}

extern SDL_Event quit_event;

int inFatal (const char * str) {

	FILE * fatFile = fopen ("fatal.txt", "wt");
	if (fatFile) {
		fprintf (fatFile, "FATAL:\n%s\n", str);
		fclose (fatFile);
	}

	fatalMessage = copyString (str);
	if (fatalMessage == NULL) fatalMessage = copyString ("Out of memory");

	killSoundStuff ();
#if defined(HAVE_GLES2)
	EGL_Close();
#endif
	SDL_Quit();
		
	atexit (displayFatal);
	exit (1);
}

int checkNew (const void * mem) {
	if (mem == NULL) {
		inFatal (ERROR_OUT_OF_MEMORY);
		return 0;
	}
	return 1;
}

void setFatalInfo (const char * userFunc, const char * BIF) {
	delete fatalInfo;
	fatalInfo = new char [strlen (userFunc) + strlen (BIF) + 38];
	if (fatalInfo) sprintf (fatalInfo, "Currently in this sub: %s\nCalling: %s", userFunc, BIF);
}

void setResourceForFatal (int n) {
	resourceForFatal = n;
}

int fatal (const char * str1) {
	if (numResourceNames && resourceForFatal != -1) {
		const char * r = resourceNameFromNum(resourceForFatal);
		char * newStr = new char[strlen (str1) + strlen(r) + strlen (fatalInfo) + 14];
		if (checkNew (newStr)) {
			sprintf (newStr, "%s\nResource: %s\n\n%s", fatalInfo, r, str1);
			inFatal (newStr);
		} else fatal (emergencyMemoryMessage);
	} else {
		char * newStr = new char[strlen (str1) + strlen (fatalInfo) + 3];
		if (checkNew (newStr)) {
			sprintf (newStr, "%s\n\n%s", fatalInfo, str1);
			inFatal (newStr);
		} else fatal (emergencyMemoryMessage);
	}
	return 0;
}

int fatal (const char * str1, const char * str2) {
	char * newStr = new char[strlen (str1) + strlen (str2) + 2];
	if (checkNew (newStr)) {
		sprintf (newStr, "%s %s", str1, str2);
		fatal (newStr);
	} else fatal (emergencyMemoryMessage);
	return 0;
}
