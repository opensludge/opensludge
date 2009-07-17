#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "typedef.h"
#include "Splitter.hpp"
#include "moreio.h"
#include "messbox.h"
#include "wintext.h"
#include "settings.h"
#include "dumpfiles.h"

int numberOfValidTranslations = 0;

struct translationReg {
	char * filename;
	int ID;
	translationReg * next;
};

translationReg * allTranslations = NULL;

enum mode {TM_COMMENTS, TM_ID, TM_DATA};

bool addNewTraReg (char * filename, int ID) {
//	errorBox ("Adding", filename);
	translationReg * newReg = new translationReg;
	if (newReg) {
		newReg -> filename = copyString (filename);
		if (newReg -> filename) {
			newReg -> ID = ID;
			newReg -> next = allTranslations;
			allTranslations = newReg;
			numberOfValidTranslations ++;
			return true;
		}
		delete newReg;
	}
	return errorBox (ERRORTYPE_INTERNALERROR, "Out of memory adding translation data", filename, NULL);
}

void registerTranslationFile (char * filename) {
	FILE * fp = fopen (filename, "rt");
	
	if (fp == NULL) {
		errorBox (ERRORTYPE_PROJECTERROR, "Can't open translation file for reading", filename, NULL);
		return;
	}
	
	char * theLine = readText (fp);
	if (strcmp (theLine, "### SLUDGE Translation File ###")) {
		fclose (fp);
		errorBox (ERRORTYPE_PROJECTERROR, "Not a valid SLUDGE translation file", filename, NULL);
		return;
	}
	
	mode theMode = TM_COMMENTS;
	int ID = -1;
	
	do {
		delete theLine;
		theLine = readText (fp);
		
		if (theLine && theLine[0]) {
			if (theLine[0] == '[' && theLine[strlen (theLine) - 1] == ']') {
				if (strcmp (theLine, "[DATA]") == 0) {
					theMode = TM_DATA;
				} else if (strcmp (theLine, "[ID]") == 0) {
					theMode = TM_ID;
				} else {
					errorBox (ERRORTYPE_PROJECTERROR, "Found a block type that I don't recognise in a translation file", theLine, NULL);
				}
			} else {
				if (theMode == TM_ID) {
					ID = stringToInt (theLine, ERRORTYPE_PROJECTERROR);
				}
			}
		}
	} while (theLine && theMode != TM_DATA);
	
	fclose (fp);
	
	if (theMode != TM_DATA) {
		errorBox (ERRORTYPE_PROJECTERROR, "This translation file doesn't seem to contain any translation data", filename, NULL);
		return;
	}
	
	if (ID < 0 || ID > 0xFFFF) {
		errorBox (ERRORTYPE_PROJECTERROR, "This translation file doesn't have a valid ID (either no ID is specified or the ID given is too high a number, negative or non-numerical)", filename, NULL);
		return;
	}
	
	addNewTraReg (filename, ID);
//		errorBox ("Translation file registered OK", filename);
}

stringArray * transFrom = NULL;
stringArray * transTo = NULL;

bool cacheTranslationData (char * f) {
	if (! gotoSourceDirectory ()) return false;
	FILE * fp = fopen (f, "rt");
	if (! fp) return errorBox (ERRORTYPE_PROJECTERROR, "Translation file has suddenly gone missing", f, NULL);
	
	bool unfinished = false;

	char * theLine = NULL;
	do {
		delete theLine;
		theLine = readText (fp);
	} while (theLine && strcmp (theLine, "[DATA]"));
	
	if (! theLine) return errorBox (ERRORTYPE_PROJECTERROR, "No [DATA] all of a sudden in file", f, NULL);
	delete theLine;

	do {
		theLine = readText (fp);
		if (theLine && theLine[0] && theLine[0] != '\t') {
			stringArray * pair = splitString (theLine, '\t', ONCE, false);
			addToStringArray (transFrom, pair->string, 0, -1, true);
			if (pair->next == NULL) {
				// No translation
			} else if (strcmp (pair->next->string, "*\t") == 0) {
				// Unfinished file
				if (unfinished == false) {
					errorBox (ERRORTYPE_PROJECTWARNING, "This translation file isn't finished - there are still strings in the \"YET TO BE TRANSLATED\" category", f, NULL);
					unfinished = true;
				}
			} else {
				// Translation
				destroyFirst (pair);
			}

			// The only thing we DO want to trim is excess tabbage
			trimStart (pair->string, '\t');

			addToStringArray (transTo, pair->string, 0, -1, true);
			while (destroyFirst (pair)) {;}
		}
	} while (theLine);

	fclose (fp);
	
	return true;
}

char * translateMe (char * originalIn) {
	char * original = copyString (originalIn);

	if (! original[0]) return original;

	int spacesAtStart = 0, spacesAtEnd = 0;
	while (trimStart (original, ' ')) spacesAtStart ++;
	while (trimEnd (original, ' ')) spacesAtEnd ++;
//	addComment (original);
	
	char * trans = NULL;
	int locInArray = findElement (transFrom, original);

	if (locInArray >= 0) {
		trans = copyString (returnElement (transTo, locInArray));
		delete original;
	} else {
		if (original[0]) errorBox (ERRORTYPE_PROJECTWARNING, "No translation for", original, NULL);
		trans = original;
	}
	
	for (int i = 0; i < spacesAtStart; i ++) {
		original = joinStrings (" ", trans);
		delete trans;
		trans = original;
	}

	for (int i = 0; i < spacesAtEnd; i ++) {
		original = joinStrings (trans, " ");
		delete trans;
		trans = original;
	}

	return trans;
}

bool addTranslationData (translationReg * trans, stringArray * theSA, FILE * mainFile) {
	if (! cacheTranslationData (trans->filename)) return false;
	
	FILE * projectFile, * indexFile;
	
	int indexSize = countElements (theSA) * 4 + ftell (mainFile) + 4;

//	errorBox ("Number of unique strings", countElements (theSA));

	if (! gotoTempDirectory ()) return false;
	projectFile = fopen ("tdata.tmp", "wb");
	indexFile = fopen ("tindex.tmp", "wb");

	if (! (projectFile && indexFile)) return errorBox (ERRORTYPE_SYSTEMERROR, "Can't write to temporary file", NULL, NULL);

	while (theSA) {
		put4bytes ((ftell (projectFile) + indexSize), indexFile);
		char * translation = translateMe (theSA -> string);
		writeString (translation, projectFile);
		delete translation;
		theSA = theSA -> next;
	}

	destroyAll (transFrom);
	destroyAll (transTo);

	put4bytes (ftell (projectFile) + indexSize, mainFile);

//	addComment ("Size of string file: ", ftell (projectFile) + indexSize);
	fclose (projectFile);
	fclose (indexFile);

	if (! gotoTempDirectory ()) return false;
	if (dumpFileInto (mainFile, "tindex.tmp") && dumpFileInto (mainFile, "tdata.tmp")) {
		unlink ("tindex.tmp");
		unlink ("tdata.tmp");
		return true;
	} else {
		return false;
	}	
}

bool addAllTranslationData (stringArray * theSA, FILE * mainFile) {
	translationReg * temp = allTranslations;

	while (temp) {
		if (! addTranslationData (temp, theSA, mainFile)) return false;
		temp = temp -> next;
	}

	return true;
}

void addTranslationIDTable (FILE * mainFile) {
	translationReg * temp = allTranslations;

	fputc (numberOfValidTranslations, mainFile);

	while (temp) {
		put2bytes (temp -> ID, mainFile);
		temp = temp -> next;
	}	
}

