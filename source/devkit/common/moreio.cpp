#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>


#include "moreio.h"

#define MOVETEXT 1

char * readString (FILE * fp) {
	int n = get2bytes (fp), a;
	char * grabber = new char[n + 1];

//	checkNew (grabber);

	for (a = 0; a < n; a ++) {
		grabber[a] = (char) (fgetc (fp) - MOVETEXT);
	}
	grabber[n] = NULL;

	return grabber;
}

void put2bytes (unsigned int numtoput, FILE * fp) {
	fputc ((unsigned char) (numtoput / 256), fp);
	fputc ((unsigned char) (numtoput % 256), fp);
}

void put2bytesR (int numtoput, FILE * fp) {
	fputc ((char) (numtoput % 256), fp);
	fputc ((char) (numtoput / 256), fp);
}

void put4bytes (long int i, FILE * fp) {
	fwrite (&i, sizeof (long int), 1, fp);
}


unsigned int get2bytes (FILE * fp) {
	return fgetc (fp) * 256 + fgetc (fp);
}

char * copyString (const char * c) {
	char * r = new char[strlen (c) + 1];
	if (! r) return NULL;
	strcpy (r, c);
	return r;
}

long get4bytes (FILE * fp) {
	long f;
	fread (& f, sizeof (long), 1, fp);
	return f;
}

void writeString (const char * txt, FILE * fp) {
	int a, n = strlen (txt);

	put2bytes (n, fp);

	for (a = 0; a < n; a ++) {
		fputc (txt[a] + MOVETEXT, fp);
	}
}

char * readText (FILE * fp) {
	fpos_t startPos;
	int stringSize = 0;
	bool keepGoing = true;
	char gotChar;
	char * reply;

	fgetpos (fp, & startPos);
	while (keepGoing) {
		gotChar = (char) fgetc (fp);
		if ((gotChar == '\n') || feof (fp)) {
			keepGoing = false;
		} else {
			stringSize ++;
		}
	}

	if ((stringSize == 0) && (feof (fp))) {
		reply = NULL;
	} else {
		fseek (fp, startPos, 0);
		reply = new char[stringSize + 1];
//		checkNew (reply);
		fread (reply, stringSize, 1, fp);
		if (reply[stringSize-1] == '\r') reply[stringSize-1] = 0;
		fgetc (fp); // Skip the newline character
		reply[stringSize] = NULL;
	}

	return reply;
}

char * grabWholeFile (char * theName) {
	FILE * inputFile;
	char * allText;
	fpos_t size;

	inputFile = fopen (theName, "rb");
	if (! inputFile) return NULL;//fatal ("Can't read file", theName);

	fseek (inputFile, 0, 2);		// Jump to the end
	fgetpos (inputFile, & size);	// Get the position
	fseek (inputFile, 0, 0);		// Back to the start

//	if (size >= MAXINT - 2) fatal ("File too big to read into memory", theName);

	// Allocate memory... then read and close the file

	allText = new char[(int) size + 1];
//	checkNew (allText);
	fread (allText, (int) size, 1, inputFile);
   allText[(int) size] = NULL;
	fclose (inputFile);

	return allText;
}


bool newerFile (char * newFileN, char * oldFileN) {

	struct stat oldFileStat, newFileStat;
	if (stat (oldFileN, & oldFileStat)) return true;
	if (stat (newFileN, & newFileStat)) return true;
	return difftime (oldFileStat.st_mtime, newFileStat.st_mtime) <= 0;

}



void putFloat (float f, FILE * fp) {
	fwrite (& f, sizeof (float), 1, fp);
}


short getSigned (FILE * fp) {
	short f;
	fread (& f, sizeof (short), 1, fp);
	return f;
}

void putSigned (short f, FILE * fp) {
	fwrite (& f, sizeof (short), 1, fp);
}

