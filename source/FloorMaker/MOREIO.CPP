#include <windows.h>
#include <stdio.h>

int get2bytes (FILE * fp) {
	return fgetc (fp) * 256 + fgetc (fp);
}

void put2bytes (int numtoput, FILE * fp) {
	fputc ((char) (numtoput / 256), fp);
	fputc ((char) (numtoput % 256), fp);
}

char * copyString (const char * c) {
	char * r = new char[strlen (c) + 1];
	if (! r) return NULL;
	strcpy (r, c);
	return r;
}
