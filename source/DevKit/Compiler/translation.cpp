#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "typedef.h"
#include "splitter.hpp"
#include "moreio.h"
#include "helpers.h"
#include "messbox.h"
#include "interface.h"
#include "settings.h"
#include "dumpfiles.h"
#include "utf8.h"

int numberOfValidTranslations = 0;

struct translationReg {
	char * filename;
	char * name;
	int ID;
	translationReg * next;
};

translationReg * allTranslations = NULL;

enum mode {TM_COMMENTS, TM_ID, TM_DATA, TM_NAME};

bool addNewTraReg (char * filename, int ID, char * name) {
	translationReg * newReg = new translationReg;
	if (newReg) {
		newReg -> filename = copyString (filename);
		if (newReg -> filename) {
			newReg -> ID = ID;
			newReg -> name = name;
			newReg -> next = allTranslations;
			allTranslations = newReg;
			numberOfValidTranslations ++;
			return true;
		}
		delete newReg;
	}
	return addComment (ERRORTYPE_INTERNALERROR, "Out of memory adding translation data", NULL, filename,0);
}

void clearTranslations ()
{
	translationReg * oldReg;
	while (oldReg = allTranslations) {
		allTranslations = allTranslations->next;
		delete oldReg->filename;
		delete oldReg->name;
		delete oldReg;
	}
	numberOfValidTranslations = 0;
}

void registerTranslationFile (char * filename) {
	char * name = NULL;

	FILE * fp = fopen (filename, "rb");

	if (fp == NULL) {
		addComment (ERRORTYPE_PROJECTERROR, "Can't open translation file for reading", NULL, filename,0);
		return;
	}

	char * theLine = readText (fp);
	if (strcmp (theLine, "### SLUDGE Translation File ###")) {
		fclose (fp);
		addComment (ERRORTYPE_PROJECTERROR, "Not a valid SLUDGE translation file", NULL, filename,0);
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
				} else if (strcmp (theLine, "[NAME]") == 0) {
					theMode = TM_NAME;
				} else {
					addComment (ERRORTYPE_PROJECTERROR, "Found a block type that I don't recognise in a translation file", theLine, filename,0);
				}
			} else {
				if (theMode == TM_ID) {
					ID = stringToInt (theLine, ERRORTYPE_PROJECTERROR);
				} else if (theMode == TM_NAME) {
					name = copyString (theLine);
				}

			}
		}
	} while (theLine && theMode != TM_DATA);

	fclose (fp);

	if (theMode != TM_DATA) {
		addComment (ERRORTYPE_PROJECTERROR, "This translation file doesn't seem to contain any translation data", NULL, filename,0);
		return;
	}

	if (ID < 0 || ID > 0xFFFF) {
		addComment (ERRORTYPE_PROJECTERROR, "This translation file doesn't have a valid ID (either no ID is specified or the ID given is too high a number, negative or non-numerical)", NULL, filename,0);
		return;
	}

	addNewTraReg (filename, ID, name);
}

stringArray * transFrom = NULL;
stringArray * transTo = NULL;

bool cacheTranslationData (char * f) {
	if (! gotoSourceDirectory ()) return false;
	FILE * fp = fopen (f, "rb");
	if (! fp) return addComment (ERRORTYPE_PROJECTERROR, "Translation file has suddenly gone missing", NULL, f,0);

	bool unfinished = false;

	char * theLine = NULL;
	do {
		delete theLine;
		theLine = readText (fp);
	} while (theLine && strcmp (theLine, "[DATA]"));

	if (! theLine) return addComment (ERRORTYPE_PROJECTERROR, "No [DATA] all of a sudden in file", NULL, f,0);
	delete theLine;

	do {
		theLine = readText (fp);
		if (theLine && theLine[0] && theLine[0] != '\t') {
			stringArray * pair = splitString (theLine, '\t', ONCE, false);
			if (! u8_isvalid(pair->string)) {
				return addComment (ERRORTYPE_PROJECTERROR, "Invalid string found. (It is not UTF-8 encoded.)", NULL, f, 0);
			}
			
			addToStringArray (transFrom, pair->string, 0, -1, true);
			if (pair->next == NULL) {
				// No translation
			} else if (strcmp (pair->next->string, "*\t") == 0) {
				// Unfinished file
				if (unfinished == false) {
					addComment (ERRORTYPE_PROJECTWARNING, "This translation file isn't finished - there are still strings in the \"YET TO BE TRANSLATED\" category", NULL, f,0);
					unfinished = true;
				}
			} else {
				// Translation
				destroyFirst (pair);
			}

			// The only thing we DO want to trim is excess tabbage
			trimStart (pair->string, '\t');

			if (! u8_isvalid(pair->string)) {
				return addComment (ERRORTYPE_PROJECTERROR, "Invalid string found. (It is not UTF-8 encoded.)", NULL, f, 0);
			}
			addToStringArray (transTo, pair->string, 0, -1, true);
			while (destroyFirst (pair)) {;}
		}
	} while (theLine);

	fclose (fp);

	return true;
}

char * translateMe (char * originalIn, char * filename) {
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
		if (original[0]) addComment (ERRORTYPE_PROJECTWARNING, "No translation for", original, filename,0);
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

	if (! gotoTempDirectory ()) return false;
	projectFile = fopen ("tdata.tmp", "wb");
	indexFile = fopen ("tindex.tmp", "wb");

	if (! (projectFile && indexFile)) {
		addComment (ERRORTYPE_SYSTEMERROR, "Can't write to temporary file", NULL);
		return false;
	}

	while (theSA) {
		put4bytes ((ftell (projectFile) + indexSize), indexFile);
		char * translation = translateMe (theSA -> string, trans->filename);
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

void addTranslationIDTable (FILE * mainFile, char * name) {
	translationReg * temp = allTranslations;

	fputc (numberOfValidTranslations, mainFile);
	if (numberOfValidTranslations) {
		if (name && name[0]) {
			writeString (name, mainFile);
		} else {
			writeString ("No translation", mainFile);
		}
	}

	while (temp) {
		put2bytes (temp -> ID, mainFile);
		if (temp->name)
			writeString (temp->name, mainFile);
		else
			writeString ("Unnamed translation", mainFile);
		temp = temp -> next;
	}
}

