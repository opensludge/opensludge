#include <stdarg.h>

#include "allfiles.h"
#include "debug.h"

#if DEBUGGING
void debugOut(char * a, ...) {

	va_list argptr;
	va_start(argptr, a);

	FILE * fp = fopen ("debuggy.txt", "at");
	if (fp) {
		vfprintf (fp, a, argptr);
		fclose (fp);
	}
}
#endif