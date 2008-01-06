#include <windows.h>
#include <stdio.h>

int get2bytes (FILE * fp) {
	return fgetc (fp) * 256 + fgetc (fp);
}

void put2bytes (int numtoput, FILE * fp) {
	fputc ((char) (numtoput / 256), fp);
	fputc ((char) (numtoput % 256), fp);
}

void put2bytesR (int numtoput, FILE * fp) {
	fputc ((char) (numtoput % 256), fp);
	fputc ((char) (numtoput / 256), fp);
}
