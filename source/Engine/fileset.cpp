#include <errno.h>
#include <stdint.h>
#include <string.h>

#include "stringy.h"

// For unicode conversion
#include <iconv.h>

#include "debug.h"

#include "allfiles.h"
#include "moreio.h"
#include "newfatal.h"
#include "version.h"


bool sliceBusy = true;
FILE * bigDataFile = NULL;
uint32_t startOfDataIndex, startOfTextIndex,
			  startOfSubIndex, startOfObjectIndex;

bool openSubSlice (int num) {
//	FILE * dbug = fopen ("debuggy.txt", "at");

//	fprintf (dbug, "\nTrying to open sub %i\n", num);

	if (sliceBusy) {
		fatal ("Can't read from data file", "I'm already reading something");
		return false;
	}

//	fprintf (dbug, "Going to position %li\n", startOfSubIndex + (num << 2));
	fseek (bigDataFile, startOfSubIndex + (num << 2), 0);
	fseek (bigDataFile, get4bytes (bigDataFile), 0);
//	fprintf (dbug, "Told to skip forward to %li\n", ftell (bigDataFile));
//	fclose (dbug);
	return sliceBusy = true;
}

bool openObjectSlice (int num) {
//	FILE * dbug = fopen ("debuggy.txt", "at");

//	fprintf (dbug, "\nTrying to open object %i\n", num);

	if (sliceBusy) {
		fatal ("Can't read from data file", "I'm already reading something");
		return false;
	}

//	fprintf (dbug, "Going to position %li\n", startOfObjectIndex + (num << 2));
	fseek (bigDataFile, startOfObjectIndex + (num << 2), 0);
	fseek (bigDataFile, get4bytes (bigDataFile), 0);
//	fprintf (dbug, "Told to skip forward to %li\n", ftell (bigDataFile));
//	fclose (dbug);
	return sliceBusy = true;
}

unsigned int openFileFromNum (int num) {
//	FILE * dbug = fopen ("debuggy.txt", "at");

	if (sliceBusy) {
		fatal ("Can't read from data file", "I'm already reading something");
		return 0;
	}

//	fprintf (dbug, "\nTrying to open file %i\n", num);
//	fprintf (dbug, "Jumping to %li (for index) \n", startOfDataIndex + (num << 2));
	fseek (bigDataFile, startOfDataIndex + (num << 2), 0);
	fseek (bigDataFile, get4bytes (bigDataFile), 1);
//	fprintf (dbug, "Jumping to %li (for data) \n", ftell (bigDataFile));
	sliceBusy = true;
//	fclose (dbug);

	return get4bytes (bigDataFile);
}


// Converts a string from Windows CP-1252 to UTF-8. 
// This is needed for old games.
char * convertString(char * s) {
	static char *buf = NULL;
	
	if (! buf) {
		buf = new char [65536];	
		if (! checkNew (buf)) return NULL;
	}

	char **tmp1 = (char **) &s;
	char **tmp2 = (char **) &buf;
	char * sOrig = s;
	char * bufOrig = buf;
	
	iconv_t convert = iconv_open ("UTF-8", "CP1250");

	if (convert == (iconv_t)-1) {
			switch (errno) {
			case EINVAL:
				fprintf (stderr, "Error: Encoding not supported by iconv.\n");
				break;
			default:
				fprintf (stderr, "Error: Could not open iconv conversion descriptor.\n");
		}	
	}

	size_t len1 = strlen(s)+1;
	size_t len2 = 65535;
	size_t iconv_value =
#ifdef _WIN32
	iconv (convert,(const char **) tmp1, &len1, tmp2, &len2);
#else
	iconv (convert,(char **) tmp1, &len1, tmp2, &len2);
#endif
	
    if (iconv_value == (size_t) -1) {
		switch (errno) {
				/* See "man 3 iconv" for an explanation. */
			case EILSEQ:
				fprintf (stderr, "Invalid multibyte sequence.\n");
				break;
			case EINVAL:
				fprintf (stderr, "Incomplete multibyte sequence.\n");
				break;
			case E2BIG:
				fprintf (stderr, "No more room.\n");
				break;
			default:
				fprintf (stderr, "Error: %s.\n", strerror (errno));
		}
		fatal("Conversion to Unicode failed. This can be fixed by recompiling the game in a current version of the SLUDGE Development Kit, but this error should never happen. Congratulations, you've found a bug in the SLUDGE engine! Please let us know about it.");
    }
	iconv_close (convert);
	
	
	delete [] sOrig;
	return copyString(buf = bufOrig);
}

char * getNumberedString (int value) {

	if (sliceBusy) {
		fatal ("Can't read from data file", "I'm already reading something");
		return NULL;
	}

	fseek (bigDataFile, (value << 2) + startOfTextIndex, 0);
	value = get4bytes (bigDataFile);
	fseek (bigDataFile, value, 0);

	char * s = readString (bigDataFile);
	
	if (gameVersion < VERSION(2,2)) {
		// This is an older game - We need to convert the string to UTF-8
		s = convertString(s);
	}
	
	return s;
}

bool startAccess () {
	int wasBusy = sliceBusy;
	sliceBusy = true;
	return wasBusy;
}
void finishAccess () {
	sliceBusy = false;
}

int32_t startIndex;

void setFileIndices (FILE * fp, int numLanguages, unsigned int skipBefore) {
	if (fp) {
		// Keep hold of the file handle, and let things get at it
		bigDataFile = fp;
		startIndex = ftell (fp);
	} else {
		// No file pointer - this means that we reuse the bigDataFile
		fp = bigDataFile;
		fseek (fp, startIndex, 0);
	}
	sliceBusy = false;

	if (skipBefore > numLanguages) {
		warning ("Not a valid language ID! Using default instead.");
		skipBefore = 0;
	}

	// STRINGS
	int skipAfter = numLanguages - skipBefore;
	while (skipBefore) {
		fseek (fp, get4bytes (fp), 0);
		skipBefore --;
	}
	startOfTextIndex = ftell (fp) + 4;

	fseek (fp, get4bytes (fp), 0);

	while (skipAfter) {
		fseek (fp, get4bytes (fp), 0);
		skipAfter --;
	}

	startOfSubIndex = ftell (fp) + 4;
	fseek (fp, get4bytes (fp), 1);

	startOfObjectIndex = ftell (fp) + 4;
	fseek (fp, get4bytes (fp), 1);

	// Remember that the data section starts here
	startOfDataIndex = ftell (fp);
}
