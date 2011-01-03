/*
 *  helpers.cpp
 *  Helper functions that don't depend on other source files.
 */

#include <stdio.h>
#include <string.h>

#include "helpers.h"

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
