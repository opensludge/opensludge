#include <stdarg.h>

#include "allfiles.h"
#include "debug.h"
#include "language.h"

void debugOut(const char * a, ...) {
	if (! gameSettings.debugMode) return;

	va_list argptr;
	va_start(argptr, a);

#if defined __unix__ && !(defined __APPLE__)
	vfprintf(stderr, a, argptr);
#else
	FILE * fp = fopen ("debuggy.txt", "at");
	if (fp) {
		vfprintf (fp, a, argptr);
		fclose (fp);
	}
#endif
}

void debugHeader()
{
    debugOut( "*** Engine compiled " __DATE__ " at " __TIME__ ".\n");
}
