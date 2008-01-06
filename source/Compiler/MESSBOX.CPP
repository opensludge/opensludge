#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "winterfa.h"
#include "messbox.h"
#include "splitter.h"

extern HWND warningWindowH;
extern HWND compWin;

char * errorTypeStrings[ERRORTYPE_NUM] =
{
	"WARNING: ",
	"ERROR: ",
	"SYSTEM ERROR: ",
	"INTERNAL ERROR: "
};

//char * allComments = NULL;

struct errorLinkToFile
{
	int errorType;
	char * overview;
	char * filename;
	int lineNumber;
	errorLinkToFile * next;
};

errorLinkToFile * errorList = NULL;
int numErrors = 0;

void addComment (int errorType, const char * comment, const char * filename/*, int lineNumber*/) {

	if (filename && filename[0] == '\0')
		filename = NULL;

	errorLinkToFile * newLink = new errorLinkToFile;
	
	if (newLink)
	{
		newLink->errorType = errorType;
		newLink->overview = copyString (comment);
		newLink->filename = filename ? copyString (filename) : NULL;
		newLink->lineNumber = 0;
		newLink->next = errorList;
		errorList = newLink;

		char * after = filename ? joinStrings (" (in ", filename, ")") : copyString ("");
		char * wholeThing = joinStrings (errorTypeStrings[errorType], comment, after);
		SendDlgItemMessage(warningWindowH, ID_WARNINGLIST, LB_ADDSTRING, (WPARAM) 0, (LPARAM) wholeThing);
		SendDlgItemMessage(warningWindowH, ID_WARNINGLIST, LB_SELECTSTRING, (WPARAM) 0, (LPARAM) wholeThing);
		delete after;
		delete wholeThing;
		ShowWindow (warningWindowH, SW_SHOW);

		numErrors ++;
	}
}

void messageBox (const char * tx2, const char * tx1) {
	addComment (ERRORTYPE_PROJECTERROR, tx1, NULL);
//	MessageBox (NULL, tx1, tx2, MB_OK | MB_SETFOREGROUND);
}

BOOL errorBox (int errorType, const char * txt1, const char * txt2, const char * filename) {
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
	return FALSE;
}
/*
BOOL errorBox (const char * txt1, unsigned int i, const char * filename) {
	char * a = new char[strlen (txt1) + 30];
	if (a) {
		sprintf (a, "%s:\n%lu", txt1, i);
		addComment (a, filename);
		delete a;
	}
	return FALSE;
}*/

extern char * sourceDirectory;

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