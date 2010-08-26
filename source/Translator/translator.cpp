/*
 *  Translator.cpp
 *  Sludge Dev Kit
 *
 *  Created by Rikard Peterson on 2009-08-06.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>

#include "MessBox.h"
#include "MOREIO.H"
#include "settings.h"
#include "SPLITTER.HPP"

#include "Translator.h"

#define HEADERLINE	"### SLUDGE Translation File ###"

enum theMode {MODE_ID, MODE_NAME, MODE_STRINGS, MODE_UNKNOWN};

stringArray * splitLangString (const char * inString, const char findCharIn, const splitMode howMany) {
	stringArray * newStringArray = NULL;
	int a, stringLen = strlen (inString), lastStart = 0;
	char findChar = findCharIn;
	
	for (a = 0; a < stringLen; a ++) {
		if (inString[a] == findChar) {
			addToStringArray (newStringArray, inString, lastStart, a, false);
			lastStart = a + 1;
			if (howMany == ONCE) findChar = NULL;
		}
	}
	addToStringArray (newStringArray, inString, lastStart, stringLen, false);
	
	return newStringArray;
}


void newFile (transLine ** firstTransLine) {
	transLine * selectedTransLine;
	while (*firstTransLine) {
		selectedTransLine = *firstTransLine;
		*firstTransLine = (*firstTransLine) -> next;
		
		delete selectedTransLine -> transFrom;
		delete selectedTransLine -> transTo;
		delete selectedTransLine;
	}
}


transLine * addLine (char * line, transLine * lastSoFar) {
	stringArray * pair = splitLangString (line, '\t', ONCE);
	
	transLine * nt = new transLine;
		
	trimEdgeSpace (pair->string);
	nt -> transFrom = copyString (pair->string);
	destroyFirst (pair);
	if (pair) {
		trimEdgeSpace (pair->string);
		if (strcmp (pair -> string, "*\t")) {
			nt -> transTo = copyString (pair -> string);
			nt -> type = TYPE_TRANS;
		} else {
			nt -> transTo = NULL;
			nt -> type = TYPE_NEW;
		}
		destroyFirst (pair);
	} else {
		nt -> transTo = NULL;
		nt -> type = TYPE_NONE;
	}
	
	if (lastSoFar) {
		lastSoFar -> next = nt;
	}
	
	nt -> next = NULL;		
	return nt;
}

bool loadTranslationFile (char * fileIn, transLine ** firstTransLine, char **langName, unsigned int *lanID) {
	char * file = copyString (fileIn);
	char * error = NULL;
	transLine * lastSoFar = NULL;
	newFile (firstTransLine);
	
	FILE * fp = fopen (file, "rt");
	if (fp == NULL) {
		error = "Can't open file for reading";
	} else {
		char * line = readText (fp);
		if (line == NULL) {
			error = "File is empty";
		} else if (strcmp (line, HEADERLINE)) {
			error = "Not a SLUDGE translation file (first line isn't right)";
		} else {
			theMode mode = MODE_UNKNOWN;
			for (;;) {
				delete line;
				line = readText (fp);
				if (line == NULL) break;
				switch (line[0]) {
					case NULL:
						break;
						
					case '[':
						if (strcmp (line, "[ID]") == 0)
							mode = MODE_ID;
						else if (strcmp (line, "[DATA]") == 0)
							mode = MODE_STRINGS;
						else if (strcmp (line, "[NAME]") == 0)
							mode = MODE_NAME;
						else
							mode = MODE_UNKNOWN;
						break;
						
					default:
						switch (mode) {
							case MODE_ID:
								long id = strtol(line, NULL, 10); //stringToInt (line);
								if (id > 0) *lanID = id;
								break;
								
							case MODE_NAME:
								if (strlen(line) && !(*langName)) {
									(*langName) =  copyString (line);
								}
								break;
								
							case MODE_STRINGS:
								lastSoFar = addLine (line, lastSoFar);
								if (!(*firstTransLine))
									*firstTransLine = lastSoFar;
								break;
						}
				}
			}
		}
		delete line;
	}
	fclose (fp);
	if (error) {
		errorBox (error, file);
	}
	delete file;
	return (error == 0);
}

bool saveTranslationFile (const char * filename, transLine * firstTransLine, char *langName, unsigned int lan) {
	
	FILE * fp = fopen (filename, "wt");
	
	if (! fp) {
		return false;
	}
	
	fprintf (fp, HEADERLINE"\n\n[ID]\n%i\n\n[NAME]\n%s\n\n[DATA]\n", lan, langName);
	transLine * eachLine = firstTransLine;
	
	while (eachLine) {
		switch (eachLine -> type) {
			case TYPE_NEW:
				fprintf (fp, "%s\t*\t\n", eachLine -> transFrom);
				break;
				
			case TYPE_TRANS:
				fprintf (fp, "%s\t%s\n", eachLine -> transFrom, eachLine -> transTo);
				break;
				
			default:
				fprintf (fp, "%s\n", eachLine -> transFrom);
				break;
		}
		eachLine = eachLine -> next;
	}
	
	fclose (fp);	
	return true;
}


int foundStringInFileDel (char * string, transLine ** firstTransLine) {
	transLine * hunt = *firstTransLine;
	transLine * lastOne = NULL;
	
	trimEdgeSpace (string);
	
	while (hunt) {
		if (strcmp (hunt -> transFrom, string) == 0) {
			hunt -> exists = 1;
			return 0;
		}
		lastOne = hunt;
		hunt = hunt -> next;
	}
	//	errorBox ("Found NEW string", string);
	hunt = addLine(string, lastOne);
	if (!(*firstTransLine))
		*firstTransLine = hunt;
	
	hunt->type = TYPE_NEW;
	hunt->exists = 1;
	delete string;
	return 1;
}


int foundStringInFileEscaped (char * string, transLine **firstTransLine) {
	stringArray * bits = splitLangString (string, '\t', REPEAT);
	char * rebuilt = copyString ("");
	while (bits) {
		char * temp = joinStrings (rebuilt, bits->string);
		delete rebuilt;
		rebuilt = temp;
		destroyFirst (bits);
	}
	return foundStringInFileDel(rebuilt, firstTransLine);
}



int updateFromSource (char * filename, transLine **firstTransLine) {
	int len = strlen (filename);
	if (len < 4) return 0;

	char * last4 = filename + len - 4;
	if (strcmp (last4, ".slu")) return 0;
	
	fixPath (filename, true);
	FILE * source = fopen (filename, "rt");
	if (source == NULL) {
		errorBox ("Can't open source file for reading", filename);
		return 0;
	}
	
	int numChanges = 0;
	
	for (;;) {
		char * wholeLine = readText(source);
		if (wholeLine == NULL) break;
		for (int a = 0; wholeLine[a]; a ++) {
			if (wholeLine[a] == '#') break;	// Comment? Skip it!
			if (wholeLine[a] == '\"') {
				while (wholeLine[a+1] == ' ') a ++;	// No spaces at start, please
				bool escape = false;
				for (int b = a+1; wholeLine[b]; b ++) {
					if (wholeLine[b] == '\\') {
						if (! escape) wholeLine[b] = '\t';		// So we can split the string up on tab later
						escape = ! escape;
					} else if (wholeLine[b] == '\"') {
						if (! escape) {
							if (b != a + 1) {
								wholeLine[b] = NULL;
								numChanges += foundStringInFileEscaped (wholeLine + a + 1, firstTransLine);
								wholeLine[b] = '\"';
							}
							a = b;
							break;
						}
						escape = false;
					} else {
						escape = false;
					}
				}
			}
		}
		delete wholeLine;
	}
	
	fclose (source);
	return numChanges;
}

bool updateFromProject (const char * filename, transLine **firstTransLine) {
	getSourceDirFromName (filename);
	gotoSourceDirectory ();	

	FILE * fp = fopen (filename, "rt");
	int totalNew = 0;
	int totalOld = 0;
	if (fp) {
		transLine * hunt = *firstTransLine;
		
		while (hunt) {
			hunt -> exists = 0;
			hunt = hunt -> next;
		}		
		
		char * theLine;
		for (;;) {
			theLine = readText (fp);
			if (theLine == NULL) break;
			if (strcmp (theLine, "[FILES]") == 0) break;
			stringArray * bits = splitLangString (theLine, '=', ONCE);
			if (bits -> next != NULL) {
				if (strcmp (bits -> string, "windowname") == 0 ||
					strcmp (bits -> string, "quitmessage") == 0) {
					totalNew += foundStringInFileDel (copyString (bits -> next -> string), firstTransLine);
				}
			}
			delete theLine;
		}
		if (theLine == NULL) {
			fclose (fp);
			errorBox ("Not a SLUDGE project file", filename);
		}
		while (theLine) {
			delete theLine;
			theLine = readText (fp);
			if (theLine) totalNew += updateFromSource (theLine, firstTransLine);
		}
		
		hunt = *firstTransLine;
		transLine *prev = NULL;
		while (hunt) {
			if (! hunt -> exists) {
				fprintf(stderr, "Removing string: %s\n", hunt->transFrom);
				if (prev) {
					prev->next = hunt->next;
					
					delete hunt -> transFrom;
					delete hunt -> transTo;
					delete hunt;
					hunt = prev;
				} else {
					*firstTransLine = (*firstTransLine) -> next;

					delete hunt -> transFrom;
					delete hunt -> transTo;
					delete hunt;
					hunt = *firstTransLine;					
				}
				totalOld++;
			}
			if (hunt) {
				prev = hunt;
				hunt = hunt -> next;
			}
		}		
		
	}
	if (totalOld) {
		errorBox ("Warning.", "I found unused strings in the translation file. The unused strings are removed.");
	}	else if (! totalNew) {
		errorBox ("Found no new strings in the project that I don't already know about.", "This translation file is up to date! Hooray!");
		return false;
	}
	return true;
}
