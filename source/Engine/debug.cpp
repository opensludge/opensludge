#include "allfiles.h"
#include "debug.h"

#if DEBUGGING
void debugOut(char * a) {
	FILE * fp = fopen ("debuggy.txt", "at");
	fprintf (fp, "%s\n", a);
	fclose (fp);
}
#endif