/*
 *  MessBox.cpp
 *  Sludge Dev Kit
 *
 *  Created by Rikard Peterson on 2009-07-15.
 *  Copyright 2009 Hungry Software and contributors. All rights reserved.
 *
 */

#include <stdio.h>

#include "MessBox.h"
#include "MoreIO.h"
#include "Splitter.hpp"


char * errorTypeStrings[ERRORTYPE_NUM] =
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
	
	newLink->errorType = errorType;
	newLink->overview = copyString (comment);
	newLink->filename = filename ? copyString (filename) : NULL;
	newLink->lineNumber = 0;
	newLink->next = errorList;
	errorList = newLink;
	
	char * after = filename ? joinStrings (" (in ", filename, ")") : copyString ("");
	newLink->fullText = joinStrings (errorTypeStrings[errorType], comment, after);
	
	compilerCommentsUpdated();
	
//	fprintf (stderr, "addComment: %s\n", newLink->fullText);
	delete after;
	
	numErrors ++;
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
