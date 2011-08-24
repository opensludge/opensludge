/*
 *  helpers.cpp
 *  Helper functions that don't depend on other source files.
 */

#include <stdio.h>
#include "helpers.h"

bool fileExists(const char * file) {
	FILE * tester;
	bool retval = false;
	tester = fopen (file, "rb");
	if (tester) {
		retval = true;
		fclose (tester);
	}
	return retval;
}
