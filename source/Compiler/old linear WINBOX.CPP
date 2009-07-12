#ifdef WIN32

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "wincomp.h"
#include "winbox.h"
#include "winterfa.h"
#include "messbox.h"
#include "splitter.h"
#include "preproc.h"
#include "function.h"
#include "realproc.h"
#include "registry.h"
#include "settings.h"
#include "linker.h"
#include "objtype.h"
#include "moreio.h"
#include "dumpfiles.h"
#include "percbar.h"
#include "wintext.h"
#include "allknown.h"
#include "backdrop.h"
#include "translation.h"

HWND compWin=NULL;
HWND warningWindowH=NULL;
extern HINSTANCE inst;
extern char * windowName;
extern char * quitMessage;
extern char * customIcon;
extern bool runMe;
extern bool forceSilent;

extern stringArray * functionNames;
extern char * sourceDirectory;

extern stringArray * objectTypeNames;
char * theFile;
extern stringArray * allFileHandles = NULL;

extern int numStringsFound;
extern int numFilesFound;

bool registered;

extern bool silent;

bool compileEverything () {
	int a, numProcessed = 0;
	stringArray * allSourceStrings = NULL;
	stringArray * globalVarNames = NULL;
	compilationSpace globalSpace;
	FILE * projectFile;
	char * tx;

	setGlobPointer (& globalVarNames);

	clearRect (5, P_TOP);
	clearRect (10, P_BOTTOM);

	if (! getSourceDirFromName (theFile)) return errorBox ("Error initialising!");

	projectFile = fopen (theFile, "rt");
	if (! projectFile) return errorBox ("Can't read project file", theFile);
	if (! readSettings (projectFile)) return false;
	
	addToStringArray (allSourceStrings, "");
	addToStringArray (allSourceStrings, windowName);
	addToStringArray (allSourceStrings, quitMessage);
	
	setWindowText (COM_PROJTEXT, windowName);
	setWindowText (COM_PROGTEXT, "Parsing");
	
	for (;;) {
		tx = readText (projectFile);
		if (! tx) break;
		if (tx[0] && tx[0] != '[') {
			setWindowText (COM_FILENAME, tx);
			char * compareMe = tx + (strlen (tx) - 4);
			char * lowExt = compareMe;
			while (*lowExt) {
				*lowExt = tolower (*lowExt);
				lowExt ++;
			}
			if (strcmp (compareMe, ".sld") == 0) {
				doDefines (tx, allSourceStrings, allFileHandles);
			} else if (strcmp (compareMe, ".slu") == 0) {
				if (! preProcess (tx, numProcessed ++, allSourceStrings, allFileHandles)) return false;
			} else if (strcmp (compareMe, ".tra") == 0) {
				registerTranslationFile (tx);
			} else {
				return errorBox ("What on Earth is this file doing in a project?", tx);
			}
		}
		delete tx;
	}
	fclose (projectFile);

	numStringsFound = countElements (allSourceStrings);
	numFilesFound = countElements (allFileHandles);
/*
	{
		char buff[255];
		sprintf (buff, "Source files processed: %i\r\n", numProcessed);
		addComment (buff);
		sprintf (buff, "Unique strings found: %i\r\n", numStringsFound);
		addComment (buff);
		sprintf (buff, "Unique resources found: %i\r\n", numFilesFound);
		addComment (buff);
	}
*/	
	setWindowText (COM_PROGTEXT, "Compiling");
	percRect (1, P_TOP);
	clearRect (numProcessed, P_BOTTOM);

	if (! startFunction (protoFunction ("_globalInitFunction"), 0, globalSpace, "_globalInitFunction", true, false, "-"))
		return errorBox ("Couldn't create global variable initialisation code segment");

//	if (! startFunction (protoFunction ("_globalInitFunction"), 0, globalSpace, "_globalInitFunction", true, true, "-"))
//		return errorBox ("Couldn't create global variable initialisation code segment");

	for (a = 0; a < numProcessed; a ++) {
		if (! realWork (a, globalVarNames, globalSpace)) {
			return false;
		}
		percRect (a + 1, P_BOTTOM);
	}

	if (! registered) outputDoneCode (globalSpace, SLU_UNREGMESSAGE, 0);

	outputHalfCode (globalSpace, SLU_LOAD_GLOBAL, "init");
	outputDoneCode (globalSpace, SLU_CALLIT, 0);
	finishFunction (globalSpace, 0);

	{
		char buff[255];
		sprintf (buff, "Functions: %i", countElements (functionNames) - 1);
		addComment (buff);
		sprintf (buff, "Object types: %i", countElements (objectTypeNames));
		addComment (buff);
	}
	
	stringArray * silenceChecker = allFileHandles;
	while (silenceChecker) {
		if (audioFile (silenceChecker -> string)) silent = false;
		silenceChecker = silenceChecker -> next;
	}
	
//	errorBox ("Silent", silent);
	
	//--------------------------------------------------------
	// IMPORTANT! This is where we open the actual data file...
	//--------------------------------------------------------

	projectFile = openFinalFile (".sl~", "wb");
	if (! projectFile) return errorBox ("Can't open output file for writing");

	FILE * textFile = getRegSetting ("compilerWriteStrings") ?
		openFinalFile (" text.doc", "wt") : NULL;

	writeFinalData (projectFile, registered);

	// ADD ICON ------------------------------------
	if (customIcon[0]) {
		setWindowText (COM_PROGTEXT, "Adding custom icon");
		setWindowText (COM_FILENAME, customIcon);
		convertTGA (customIcon);
		setWindowText (COM_ITEMTEXT, "");
		fputc (1, projectFile);
		if (! dumpFileInto (projectFile, customIcon)) {
			fclose (projectFile);
			return errorBox ("Error adding custom icon (file not found or not a valid TGA file)");
		}
	} else {
		fputc (0, projectFile);
	}
	//----------------------------------------------

	put2bytes (countElements (globalVarNames), projectFile);

	if (! saveStrings (projectFile, textFile, allSourceStrings)) {
		fclose (projectFile);
		return errorBox ("Can't save string bank(s)");
	}

	// Linking (scripts)

	setWindowText (COM_PROGTEXT, "Linking (scripts)");
	percRect (2, P_TOP);
	clearRect (countElements (functionNames), P_BOTTOM);
	
	if (! gotoTempDirectory ()) {
		fclose (projectFile);
		return false;
	}
	FILE * tempIndex = fopen ("SLUDGE1.tmp", "wb");
	FILE * tempData = fopen ("SLUDGE2.tmp", "wb");
	
	stringArray * sA = functionNames;
	unsigned long iSize = countElements (functionNames) * 4 + ftell (projectFile) + 4;
	for (a = 0; a < countElements (functionNames); a ++) {
		if (! runLinker (tempData, tempIndex, a, globalVarNames, iSize, allSourceStrings)) return false;
		sA = sA -> next;
		percRect (a + 1, P_BOTTOM);
	}
	put4bytes (ftell (tempIndex) + ftell (tempData), projectFile);
	fclose (tempIndex);
	fclose (tempData);
	dumpFileInto (projectFile, "SLUDGE1.tmp");
	dumpFileInto (projectFile, "SLUDGE2.tmp");
	
	// Linking (objects)

	percRect (3, P_TOP);
	setWindowText (COM_PROGTEXT, "Linking (objects)");
	int tot = countElements (objectTypeNames);
	clearRect (tot, P_BOTTOM);

	tempIndex = fopen ("SLUDGE1.tmp", "wb");
	tempData = fopen ("SLUDGE2.tmp", "wb");

	iSize = tot * 4 + ftell (projectFile) + 4;
	for (a = 0; a < tot; a ++) {
		if (! linkObjectFile (tempData, tempIndex, a, iSize)) {
			fclose (projectFile);
			return false;
		}
		percRect (a + 1, P_BOTTOM);
	}

	put4bytes (ftell (tempIndex) + ftell (tempData), projectFile);
	fclose (tempIndex);
	fclose (tempData);
	dumpFileInto (projectFile, "SLUDGE1.tmp");
	dumpFileInto (projectFile, "SLUDGE2.tmp");
	unlink ("SLUDGE1.tmp");
	unlink ("SLUDGE2.tmp");

	// Add data

	percRect (4, P_TOP);
	if (! dumpFiles (projectFile, allFileHandles)) {
		fclose (projectFile);
		return false;
	}
	percRect (5, P_TOP);
	setWindowText (COM_PROGTEXT, "Done!");
	setWindowText (COM_FILENAME, "");
	setWindowText (COM_ITEMTEXT, "");

	fclose (projectFile);
	return true;
}

bool doneOnce = false;
extern char * finalFile;

void runCompiledGame (HWND h) {
	char * wholePath = new char[strlen (sourceDirectory) + strlen (finalFile) + 20];
	if (wholePath) {
		sprintf (wholePath, "%s\\%s%s", sourceDirectory, finalFile, forceSilent ? " (silent).slg" : ".slg");
		if ((unsigned long) ShellExecute (h, "open",
										  wholePath, NULL, NULL,
										  SW_SHOWNORMAL) <= 31) {
			errorBox ("Compiled OK, but can't determine the location of the SLUDGE engine on this machine...\n\nYou HAVE installed the SLUDGE engine, haven't you?\n\nIf not, please visit http://www.hungrysoftware.com/tools/sludge/ and download it. You need only do this once - as soon as you've got a copy, your games will run automatically. Thanks!");
		} else {
			DestroyWindow (h);
		}
		delete wholePath;
	}
}

bool APIENTRY warningBoxFunc (HWND h, UINT m, WPARAM w, LPARAM l) {
	switch (m) {
		case WM_COMMAND:
		switch (LOWORD (w)) {
			case IDCANCEL:
			ShowWindow (h, SW_HIDE);
			return 1;
		}
		
		case WM_INITDIALOG:
		ShowWindow (h, SW_HIDE);
		warningWindowH = h;
		break;
	}
	
	return 0;
}

bool APIENTRY dialogComp (HWND h, UINT m, WPARAM w, LPARAM l) {
//	static OPENFILENAME ofn;
//	static char path[MAX_PATH];

	switch (m) {
		case WM_COMMAND:
		switch (LOWORD (w)) {
			case IDCANCEL:
			DestroyWindow (h);
			return 1;
			
			case IDOK:
			runCompiledGame (h);
			return 1;
		}
		break;

		case WM_USER + 1:
		
		EnableWindow (GetDlgItem (h, IDCANCEL), true);
		if (! compileEverything ()) {		// SOMETHING WENT WRONG!

			// Report

			errorBox ("Compilation aborted!");

			// Sort out file stuff

			gotoSourceDirectory ();
			char * killName = joinStrings (finalFile, ".sl~");
			unlink (killName);
			delete killName;

		} else {							// SUCCESS! HOORAY!
			EnableWindow (GetDlgItem (h, IDOK), true);

			// Sort out file stuff

			gotoSourceDirectory ();
			char * fromName = joinStrings (finalFile, ".sl~");
			char * toName = joinStrings (finalFile, forceSilent ? " (silent).slg" : ".slg");
			unlink (toName);
			if (rename (fromName, toName)) {
				errorBox ("Couldn't rename the compiled game file... it's been left with the name", fromName);
			} else {

				// Run the compiled program (if we were told to)

				if (runMe) {
					runCompiledGame (h);
				}
			}
			delete fromName;
			delete toName;
		}
		return 1;

		case WM_INITDIALOG:
//		MessageBox (NULL, "Init dialog", er, MB_OK | MB_SETFOREGROUND);
		compWin = h;
		SetClassLong(h, GCL_HICON, (LONG) LoadIcon(inst, MAKEINTRESOURCE(IDI_ICON1)));
//		GetCurrentDirectory (MAX_PATH, path);
//		memset (&ofn, 0, sizeof (ofn));
//		ofn.lStructSize = sizeof (ofn);
//		ofn.hwndOwner = h;
//		ofn.hInstance = inst;
//		ofn.nMaxFile = MAX_PATH;
//		ofn.lpstrInitialDir = path;
//		ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;
		CreateDialog(inst, MAKEINTRESOURCE(WARNINGDIALOG), h, warningBoxFunc);
		SetForegroundWindow (h);	// handle of window
		return 1;

		case WM_CLOSE:
		DestroyWindow (h);
		return 1;

		case WM_PAINT:
//		repaintPercBars ();
		if (doneOnce == false) {
			doneOnce = true;
			PostMessage (compWin, WM_USER + 1, 526, 0);
		}
		break;

		case WM_DESTROY:
		PostQuitMessage (0);
		compWin = NULL;
		return 1;

//		default:
//		return (DefWindowProc(h, m, w, l));
	}
	return 0;
}

int findCharInString (const char ch, const char * st) {
	int i = 0;
	while (st[i] && ch != st[i]) i ++;
	return i;
}

#define all32Chars "6WXE1YLM4NA5PQR3SZB7C2DFG8HJKT9V"

bool realUnlockCheck (const char * name, const char * codeIn) {
	unsigned int i;
	unsigned int codeSoFar = 3556733886;
	unsigned int exOrVal = 1842677271;
	if (! name) return false;
	if (! codeIn) return false;
	for (i = 0; name[i]; i ++) {
		codeSoFar += name[i] ^ exOrVal;
		exOrVal <<= 1;
		exOrVal += name[i] & 1;
	}
	int numCodeIn = 0;
	for (i = 0; codeIn[i]; i ++) {
		if (i == 3) {
			numCodeIn = (codeIn[i] == '-') ? numCodeIn : 0;
		} else {
			int j = findCharInString (codeIn[i], all32Chars);
			numCodeIn <<= 5;
			numCodeIn += j;
//			errorBox ("Char index", j);
		}
	}
//	errorBox ("In", numCodeIn);
//	errorBox ("Should be", codeSoFar);

	return (i == 8) ? (numCodeIn == codeSoFar) : false;
}

char * myNameIs;

void compileWindow (char * file, char * name, char * code) {
	MSG msg;
	theFile = file;
	registered = realUnlockCheck (name, code);
	myNameIs = name;

	initBuiltInFunc (registered);

	if (! CreateDialog (inst, MAKEINTRESOURCE(DIALOG_1), NULL, dialogComp)) {
		MessageBox (NULL, "Can't create dialogue box... ack!", er, MB_OK | MB_SETFOREGROUND);
		return;
	}

	while (1) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
			if (!GetMessage(&msg, NULL, 0, 0))
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else
			WaitMessage();
	}
}

#endif