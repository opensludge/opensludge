#ifdef WIN32

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "WINTERFA.H"
#include "MessBox.h"
#include "SPLITTER.HPP"

extern HWND warningWindowH;
extern HWND compWin;



//char * allComments = NULL;



void messageBox (const char * tx2, const char * tx1) {
	addComment (ERRORTYPE_PROJECTERROR, tx1, NULL);
//	MessageBox (NULL, tx1, tx2, MB_OK | MB_SETFOREGROUND);
}

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