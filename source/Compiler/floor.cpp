#include <stdio.h>
#include <string.h>

#include "moreio.h"
#include "Splitter.hpp"
#include "wintext.h"
#include "winterfa.h"
#include "messbox.h"

bool convertFloor (char * filename) {
	char * wholeFile = grabWholeFile (filename);
	if (! wholeFile) return false;

	filename[strlen (filename) - 1] = 'z';
	stringArray * knownCo = NULL;
	stringArray * splitBits;
	stringArray * sA;
	
	FILE * outFile = fopen (filename, "wb");
	if (! outFile) return addComment (ERRORTYPE_SYSTEMERROR, "Can't write compiled floor file", filename, NULL);

	sA = splitString (wholeFile, '*');
	
	destroyFirst (sA);

	fputc (countElements (sA), outFile);
	while (sA) {
		while (trimEnd (sA -> string, '\n') || trimEnd (sA -> string, '\r')) {;}
		splitBits = splitString (sA -> string, ';');
		fputc (countElements (splitBits), outFile);
		while (splitBits) {
			put2bytes (findOrAdd (knownCo, splitBits -> string), outFile);
			destroyFirst (splitBits);
		}
		destroyFirst (sA);
	}

	put2bytes (countElements (knownCo), outFile);
	int i;
	while (knownCo) {
		splitBits = splitString (knownCo -> string, ',');
		i = stringToInt (splitBits -> string, ERRORTYPE_PROJECTERROR);
		if (i < 0) {
			addComment (ERRORTYPE_PROJECTWARNING, "Error processing floor: X co-ordinate is not a positive integer. I'm pretending it's a zero.", knownCo -> string, filename);
			i = 0;
		}
		put2bytes (i, outFile);
		if (! destroyFirst (splitBits)) return addComment (ERRORTYPE_PROJECTERROR, "Error processing floor: No comma in co-ordinate pair", knownCo -> string, filename);
		i = stringToInt (splitBits -> string, ERRORTYPE_PROJECTERROR);
		if (i < 0) {
			addComment (ERRORTYPE_PROJECTWARNING, "Error processing floor: Y co-ordinate is not a positive integer. I'm pretending it's a zero.", knownCo -> string, filename);
			i = 0;
		}
		put2bytes (i, outFile);
		destroyFirst (splitBits);
		destroyFirst (knownCo);
	}
	fclose (outFile);
	return true;
}
