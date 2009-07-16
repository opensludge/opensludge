#ifdef WIN32

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "winterfa.h"
#include "messbox.h"
#include "Splitter.hpp"

extern HWND warningWindowH;
extern HWND compWin;



//char * allComments = NULL;



void messageBox (const char * tx2, const char * tx1) {
	addComment (ERRORTYPE_PROJECTERROR, tx1, NULL);
//	MessageBox (NULL, tx1, tx2, MB_OK | MB_SETFOREGROUND);
}

bool errorBox (int errorType, const char * txt1, const char * txt2, const char * filename) {
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
/*
bool errorBox (const char * txt1, unsigned int i, const char * filename) {
	char * a = new char[strlen (txt1) + 30];
	if (a) {
		sprintf (a, "%s:\n%lu", txt1, i);
		addComment (a, filename);
		delete a;
	}
	return false;
}*/


static void warningOpenSourceFile (char * fileName) {
	char * wholePath = new char[strlen (sourceDirectory) + strlen (fileName) + 2];
	if (wholePath) {
		sprintf (wholePath, "%s\\%s", sourceDirectory, fileName);
		if ((unsigned long) ShellExecute (warningWindowH, "open",
										  wholePath, NULL, NULL,
										  SW_SHOWNORMAL) <= 31) {
			errorBox (ERRORTYPE_SYSTEMERROR, "Couldn't launch ", wholePath, NULL);
		}
		delete wholePath;
	}
}

void userClickedErrorLine (int whichEntry)
{
	int n = (numErrors - 1 ) - whichEntry;
	errorLinkToFile * link = errorList;
	
	while (link && n > 0)
	{
		n --;
		link = link->next;
	}
		
	if (link && link->filename)
	{
		
		char * warningLine, *details;
		int type = MB_YESNO;
		
			warningLine = joinStrings (errorTypeStrings[link->errorType], "In file ", link->filename);
			details = joinStrings ("\n\nDETAILS: ", link->overview, "\n\nOpen source file now?");
			type = MB_YESNO;
			
		char * wholeThing = joinStrings (warningLine, details);
		delete details;
		delete warningLine;
		if (MessageBox (warningWindowH, wholeThing, "SLUDGE Compiler warning", type | MB_APPLMODAL | MB_SETFOREGROUND) == IDYES)
		{
			warningOpenSourceFile(link->filename);
		}
		delete wholeThing;
	}
}

#endif