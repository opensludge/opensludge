#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "wincomp.h"
#include "winbox.h"
#include "winterfa.h"
#include "messbox.h"
#include "splitter.h"
#include "preproc.h"
#include "function.h"
#include "realproc.h"
#include "backdrop.h"
#include "settings.h"
#include "linker.h"
#include "objtype.h"
#include "moreio.h"

#define BOXWIDTH 300

HWND compWin=NULL;
extern HWND mainWin;
extern HINSTANCE inst;

int percTotal[2] = {100, 100};
extern stringArray * functionNames;

void clearRect (int i, whichPerc whichBox) {
	HDC hdc = GetDC (compWin);

	SelectObject (hdc, CreatePen (PS_SOLID, 0, RGB (0, 0, 0)));
	SelectObject (hdc, CreateSolidBrush (RGB (128, 0, 0)));
	percTotal[whichBox] = i;
	char bu[100];
	sprintf (bu, "Space for %i bits in perc. box", i);
	messageBox ("Hello!", bu);
	if (whichBox) {
		Rectangle (hdc, 9, 79, 9 + BOXWIDTH, 100);
	} else {
		Rectangle (hdc, 9, 32, 9 + BOXWIDTH, 53);
	}

	ReleaseDC (hdc, compWin);
}

void percRect (int i, whichPerc whichBox) {
	HDC hdc = GetDC (compWin);

	SelectObject (hdc, CreateSolidBrush (RGB (255, 255, 0)));
	if (percTotal[whichBox]) {
		i = (i * BOXWIDTH) / percTotal[whichBox];
	} else {
		i = 0;
	}
	if (whichBox) {
		Rectangle (hdc, 9, 79, 9 + i, 100);
	} else {
		Rectangle (hdc, 9, 32, 9 + i, 53);
	}

	ReleaseDC (hdc, compWin);
}

BOOL dumpFiles (stringArray * & theSA) {
	int i = 0, ch;
	char outname[20];
	FILE * inFile, * outFile;

	while (theSA) {
//		showTask (i, theSA -> string);
		gotoSourceDirectory ();
		if (strcmp (theSA -> string + (strlen (theSA -> string) - 4), ".tga") == 0) {
			convertTGA (theSA -> string);
		}
		inFile = fopen (theSA -> string, "rb");
		if (inFile == NULL) return errorBox ("Can't read file", theSA -> string);
		sprintf (outname, "d%i.dat", i);
		gotoOutputDirectory ();
		outFile = fopen (outname, "wb");
		if (inFile == NULL) return errorBox ("Can't write file", outname);
		for (;;) {
			ch = fgetc (inFile);
			if (feof (inFile)) break;
			fputc (ch, outFile);
		}
		destroyFirst (theSA);
		i ++;
		fclose (inFile);
		fclose (outFile);
	}
	return TRUE;
}

BOOL dumpFileInto (FILE * writer, char * thisFile) {
	int a;
	FILE * reader = fopen (thisFile, "rb");

	if (! reader) return FALSE;
	for (;;) {
		a = fgetc (reader);
		if (a == EOF) break;
		fputc (a, writer);
	}
	fclose (reader);
	return TRUE;
}

BOOL saveAndDiscard (stringArray * & theSA, char * oName) {
	FILE * projectFile, * indexFile, * textFile;
	int indexSize = countElements (theSA) * 4;

//	addComment ("Number of unique strings: ", countElements (theSA));
	gotoTempDirectory ();
	projectFile = fopen ("txtdata.tmp", "wb");
	indexFile = fopen ("txtindex.tmp", "wb");
	textFile = fopen ("alltext.txt", "wt");
	if (! (projectFile && indexFile && textFile)) return FALSE;

	while (theSA) {
//		printf ("Writing string %s at position %i\n", theSA -> string, (int) (ftell (projectFile) + indexSize));
		put4bytes ((int) (ftell (projectFile) + indexSize), indexFile);
		writeString (theSA -> string, projectFile);
		fprintf (textFile, "%s\n", theSA -> string);
		destroyFirst (theSA);
	}
//	addComment ("Size of string file: ", ftell (projectFile) + indexSize);
	fclose (projectFile);
	fclose (indexFile);
	fclose (textFile);

	gotoOutputDirectory ();
	projectFile = fopen (oName, "wb");

	gotoTempDirectory ();
	if (dumpFileInto (projectFile, "txtindex.tmp") && dumpFileInto (projectFile, "txtdata.tmp")) {
		fclose (projectFile);
		return TRUE;
	} else {
		fclose (projectFile);
		return FALSE;
	}
}

extern int numStringsFound;
extern int numFilesFound;

BOOL compileEverything () {
	int a, numProcessed = 0;
	stringArray * allSourceStrings = NULL;
	stringArray * allFileHandles = NULL;
	stringArray * globalVarNames = NULL;
	int num = MESS(ID_FILELIST, LB_GETCOUNT, 0, 0);
	BOOL OK;
	compilationSpace globalSpace;
	FILE * projectFile;

	if (num == LB_ERR) {
		messageBox ("ERROR!", "Can't count how many things are in ID_FILELIST");
		return FALSE;
	}
	char * tx;

	COMPMESS (COM_PROGTEXT, WM_SETTEXT, 0, "Parsing");
	clearRect (10, P_TOP);
	percRect (1, P_TOP);

	clearRect (num, P_BOTTOM);
	for (a = 0; a < num; a ++) {
		tx = getFileFromBox (a);
		COMPMESS (COM_FILENAME, WM_SETTEXT, 0, tx);
		OK = preProcess (tx, numProcessed ++, allSourceStrings, allFileHandles);
		delete tx;
		if (! OK) return FALSE;
		percRect (a + 1, P_BOTTOM);
	}

	numStringsFound = countElements (allSourceStrings);
	numFilesFound = countElements (allFileHandles);
	if (! saveAndDiscard (allSourceStrings, "alltext.dat")) {
		messageBox (er, "Can't save string bank");
		return FALSE;
	}

	COMPMESS (COM_PROGTEXT, WM_SETTEXT, 0, "Compiling");
	percRect (2, P_TOP);
	startFunction (protoFunction ("_globalInitFunction"), 0, globalSpace, "_globalInitFunction");
	clearRect (numProcessed, P_BOTTOM);
	for (a = 0; a < numProcessed; a ++) {
		realWork (a, globalVarNames, globalSpace);
		percRect (a + 1, P_BOTTOM);
	}
	outputHalfCode (globalSpace, SLU_LOAD_FUNC, "init");
	outputDoneCode (globalSpace, SLU_CALLIT, 0);
	finishFunction (globalSpace, 0);

	COMPMESS (COM_PROGTEXT, WM_SETTEXT, 0, "Linking");
	percRect (3, P_TOP);
	stringArray * sA = functionNames;
	clearRect (countElements (functionNames), P_BOTTOM);
	for (a = 0; a < countElements (functionNames); a ++) {
		COMPMESS (COM_FILENAME, WM_SETTEXT, 0, sA -> string);
		if (! runLinker (a, globalVarNames)) return FALSE;
		sA = sA -> next;
		percRect (a + 1, P_BOTTOM);
	}
//	finishTask ();

//	showTask (7, "Handling external data files");
//	newTask ("Moving files", countElements (allFileHandles));
	percRect (3, P_TOP);
	if (dumpFiles (allFileHandles)) {
//		finishTask ();

//	showTask (8, "Creating final data files");
		linkObjects ();
		gotoOutputDirectory ();
		projectFile = fopen ("settings.dat", "wb");

		put2bytes (countElements (globalVarNames), projectFile);
		fclose (projectFile);

		return TRUE;
	}
	return FALSE;
}

/* Messaging macros */

BOOL APIENTRY dialogComp (h, m, w, l) HWND h; UINT m; WPARAM w; LPARAM l; {
	static OPENFILENAME ofn;
	static char path[MAX_PATH];

	switch (m) {
		case WM_COMMAND:
		if (LOWORD (w) == IDC_PUSHBUTTON1) {
			DestroyWindow (h);
			return 1;
		}
		break;

		case WM_USER:
		if (! compileEverything ()) messageBox (er, "Compilation aborted!");
		DestroyWindow (h);
		break;

		case WM_INITDIALOG:
		compWin = h;
		GetCurrentDirectory (MAX_PATH, path);
		memset (&ofn, 0, sizeof (ofn));
		ofn.lStructSize = sizeof (ofn);
		ofn.hwndOwner = h;
		ofn.hInstance = inst;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrInitialDir = path;
		ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;
		PostMessage (compWin, WM_USER, 0, 0);
		return 1;

		case WM_DESTROY:
		PostQuitMessage (0);
		compWin = NULL;
		return 1;
	}
	l;
	return 0;
}

void compileWindow () {
	MSG msg;

	if (!CreateDialog(inst, MAKEINTRESOURCE(1), mainWin, dialogComp)) {
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
