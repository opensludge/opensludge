#include "debug.h"


#include "allfiles.h"
#include "moreio.h"
#include "newfatal.h"
//#include "debug.h"

//char * outputDir;

bool sliceBusy = true;
FILE * bigDataFile = NULL;
unsigned long startOfDataIndex, startOfTextIndex,
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
		return NULL;
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

char * getNumberedString (int value) {

	//debugOut ("FILSET: getNumberedString - Reading: %i (%i) %i\n", value, (value << 2) + startOfTextIndex, startOfTextIndex);

	if (sliceBusy) {
		fatal ("Can't read from data file", "I'm already reading something");
		return NULL;
	}
	
	fseek (bigDataFile, (value << 2) + startOfTextIndex, 0);
	value = get4bytes (bigDataFile);
	//debugOut ("FILSET: getNumberedString - Seeking: %i\n", value);
	fseek (bigDataFile, value, 0);

//	char * re = readString (bigDataFile);
//	fprintf (dbug, "Read [%s]\n", re);
//	return re;
	return readString (bigDataFile);
}

void finishAccess () {
	sliceBusy = false;
}

void setFileIndices (FILE * fp, int numLanguages, unsigned int skipBefore) {
	// Keep hold of the file handle, and let things get at it
	bigDataFile = fp;
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
