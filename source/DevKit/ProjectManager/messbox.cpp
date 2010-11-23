/*
 *  MessBox.cpp
 *  Sludge Dev Kit
 *
 *  Created by Rikard Peterson on 2009-07-15.
 *  Copyright 2009 Hungry Software and contributors. All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h>

#include "messbox.h"
#include "moreio.h"
#include "splitter.hpp"
#include "compilerinfo.h"


extern stringArray * allSourceStrings;
extern stringArray * allFileHandles;

const char * errorTypeStrings[ERRORTYPE_NUM] =
{
	"WARNING: ",
	"ERROR: ",
	"SYSTEM ERROR: ",
	"INTERNAL ERROR: "
};


errorLinkToFile * errorList = NULL;
int numErrors = 0;

void clearComments () {
	while (errorLinkToFile * item = errorList) {
		errorList = errorList->next;
		delete item->overview;
		if (item->filename) delete item->filename;
		delete item->fullText;
		delete item;
	}
	numErrors = 0;
}


void addComment (int errorType, const char * comment, const char * filename/*, int lineNumber*/) {

	errorLinkToFile * newLink = new errorLinkToFile;
	if (! newLink) return;

	if (filename && filename[0] == '\0')
		filename = NULL;

	char * s = copyString(comment);
	char * s2, *s3;

	// Extract real strings...
	while (s2 = strstr(s, "_string")) {
		unsigned long i = 0;
		int ac = 0;

		s3 = s2 + 7;
		int looping = true;
		while (s3[ac] && looping) {
			if (s3[ac] >= '0' && s3[ac] <= '9') {
				i = (i * 10) + s3[ac] - '0';
				if (i >= 65535) {
					looping = false;
				}
			} else {
				looping = false;
			}
			if (looping) ac ++;
		}
		s3+=ac;
		s2[1] = 0;
		s2[0] = '"';
		s2 = s;
		s = joinStrings(s, returnElement (allSourceStrings, i), "\"", s3);
		delete s2;
	}

	// Extract resource file names ...
	while (s2 = strstr(s, "_file")) {
		unsigned long i = 0;
		int ac = 0;

		s3 = s2 + 7;
		int looping = true;
		while (s3[ac] && looping) {
			if (s3[ac] >= '0' && s3[ac] <= '9') {
				i = (i * 10) + s3[ac] - '0';
				if (i >= 65535) {
					looping = false;
				}
			} else {
				looping = false;
			}
			if (looping) ac ++;
		}
		s3+=ac;
		s2[1] = 0;
		s2[0] = '\'';
		s2 = s;
		s = joinStrings(s, returnElement (allFileHandles, i), "'", s3);
		delete s2;
	}

	newLink->errorType = errorType;
	newLink->overview = copyString (s);
	newLink->filename = filename ? copyString (filename) : NULL;
	newLink->lineNumber = 0;
	newLink->next = errorList;
	errorList = newLink;

	char * after = filename ? joinStrings (" (in ", filename, ")") : copyString ("");
	newLink->fullText = joinStrings (errorTypeStrings[errorType], s, after);

	fprintf (stderr, "addComment: %s\n", newLink->fullText);
	delete after;

	numErrors ++;

	compilerCommentsUpdated();
}

bool addComment (int errorType, const char * txt1, const char * txt2, const char * filename) {
	if (txt2)
	{
		char * a = joinStrings (txt1, ": ", txt2);
		if (a)
		{
			addComment (errorType, a, filename);
			delete a;
		}
	}
	else
	{
		addComment (errorType, txt1, filename);
	}
	return false;
}
