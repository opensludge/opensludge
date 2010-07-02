#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "WINTERFA.H"
#include "MessBox.h"
#include "SPLITTER.HPP"

extern HWND mainWin;



//char * allComments = NULL;

void compilerCommentsUpdated()
{
}
/*
static void warningOpenSourceFile (char * fileName) {
    /*
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
*/


void messageBox (const char * tx2, const char * tx1) {
		MessageBox (mainWin, tx1, tx2, MB_OK | MB_SETFOREGROUND);
}

void messageBox (const char * tx2, int f, int f2) {
	char buff[40];
	sprintf (buff, "%d - %d", f, f2);
	MessageBox (mainWin, buff, tx2, MB_OK | MB_SETFOREGROUND);
}

bool errorBox (const char * txt1, const char * txt2) {
	char * a = new char[strlen (txt1) + strlen (txt2) + 10];
	if (a) {
		sprintf (a, "%s:\n%s", txt1, txt2);
		messageBox (er, a);
		delete a;
	} else {
		messageBox (er, "Out of memory displaying error message");
	}
	return false;
}

bool errorBox (const char * txt1, unsigned int number) {
	char * a = new char[strlen (txt1) + 30];
	if (a) {
		sprintf (a, "%s:\n%d", txt1, number);
		messageBox (er, a);
		delete a;
	} else {
		messageBox (er, "Out of memory displaying error message");
	}
	return false;
}

bool ask (const char * txt) {
	return MessageBox (mainWin, txt, "SLUDGE Development Kit", MB_YESNO | MB_SETFOREGROUND) == IDYES;
}
