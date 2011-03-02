/*
 *  helpers.cpp
 *  Helper functions that don't depend on other source files.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "helpers.h"

char * sourceDirectory = NULL;

char * joinStrings (const char * a, const char * b) {
	char * nS = new char[strlen (a) + strlen (b) + 1];
	sprintf (nS, "%s%s", a, b);
	return nS;
}

char * joinStrings (const char * a, const char * b, const char * c) {
	char * nS = new char[strlen (a) + strlen (b) + strlen (c) + 1];
	sprintf (nS, "%s%s%s", a, b, c);
	return nS;
}

char * joinStrings (const char * a, const char * b, const char * c, const char * d) {
	char * nS = new char[strlen (a) + strlen (b) + strlen (c) + strlen (d) + 1];
	sprintf (nS, "%s%s%s%s", a, b, c, d);
	return nS;
}

char * joinQuote (char * a, char * b, char q1, char q2) {
	char * nS = new char[strlen (a) + strlen (b) + 3];
	sprintf (nS, "%s%c%s%c", a, q1, b, q2);
	return nS;
}


bool getSourceDirFromName (const char * name) {
	char * filename = joinStrings (name, "");

	int a, lastSlash = -1;
	for (a = 0; filename[a]; a ++) {
		if (filename[a] == '/' || filename[a] == '\\') {
			lastSlash = a;
		}
	}
	if (lastSlash != -1) {
		char slashChar = filename[lastSlash];
		filename[lastSlash] = 0;
		if (chdir (filename)) {
			fprintf (stderr, "Can't move to source directory %s", filename);
			return false;
		}
		filename[lastSlash] = slashChar;
	}
	char buff[1000];
	if (! getcwd (buff, 1000)) {
		fprintf (stderr, "I can't work out which directory I'm in...");
		return false;
	}
	sourceDirectory = joinStrings (buff, "");
	fixPath (sourceDirectory, true);
	return true;
}

bool gotoSourceDirectory () {
	bool r = chdir (sourceDirectory);
	if (r) {
		fprintf (stderr, "Can't move to source directory %s", sourceDirectory);
		return false;
	}
	return true;
}


// Fix pathnames, because Windows doesn't use the same path separator
// as the rest of the world
void fixPath (char *filename, bool makeGood) {
	if (! filename) return;
	char * ptr;

	if (makeGood) {
		while (ptr = strstr (filename, "\\")) {
			ptr[0] = '/';
		}
	} else {
		while (ptr = strstr (filename, "/")) {
			ptr[0] = '\\';
		}
	}
}

