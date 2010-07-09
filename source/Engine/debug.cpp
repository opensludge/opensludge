#include <stdarg.h>

#include "allfiles.h"
#include "debug.h"
#include "language.h"

void debugOut(char * a, ...) {
	if (gameSettings.debugMode) {
		va_list argptr;
		va_start(argptr, a);

		FILE * fp = fopen ("debuggy.txt", "at");
		if (fp) {
			vfprintf (fp, a, argptr);
			fclose (fp);
		}
	}
}
