#include <windows.h>
#include <stdio.h>
#include <string.h>

//#include "fatal.h"

#define MOVETEXT 1

unsigned int get2bytes (FILE * fp) {
	unsigned int reply = (unsigned int) (fgetc (fp)) * 256 + fgetc (fp);
	return reply;
}

void put2bytes (unsigned int numtoput, FILE * fp) {
	fputc ((unsigned char) (numtoput / 256), fp);
	fputc ((unsigned char) (numtoput % 256), fp);
}

void put4bytes (long int i, FILE * fp) {
	fwrite (&i, sizeof (long int), 1, fp);
}

char * readString (FILE * fp) {
	int n = get2bytes (fp), a;

	// Special cases when we want to return NULL...
	if (n == 0) return NULL;
	if (feof (fp)) return NULL;
	
	// Read the string!
	char * grabber = new char[n];
	n --;

	for (a = 0; a < n; a ++) {
		char c1 = fgetc (fp);
		char c2 = a & 255;
		grabber[a] = c1 ^ c2;
	}
	grabber[n] = NULL;

	return grabber;
}

void writeString (const char * txt, FILE * fp) {
	if (txt) {
		int a, n = strlen (txt);
	
		put2bytes (n + 1, fp);
	
		for (a = 0; a < n; a ++) {
			char c2 = a & 255;
			fputc (txt[a] ^ c2, fp);
		}
	} else {
		put2bytes (0, fp);
	}
}

char * readText (FILE * fp) {
	unsigned long startPos;
	int stringSize = 0;
	BOOL keepGoing = TRUE;
	char gotChar;
	char * reply;

	fgetpos (fp, & startPos);
	while (keepGoing) {
		gotChar = (char) fgetc (fp);
		if ((gotChar == '\n') || (feof (fp))) {
			keepGoing = FALSE;
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
		fgetc (fp); // Skip the newline character
		reply[stringSize] = NULL;
	}

	return reply;
}

char * grabWholeFile (char * theName) {
	FILE * inputFile;
	char * allText;
	unsigned long size;

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
