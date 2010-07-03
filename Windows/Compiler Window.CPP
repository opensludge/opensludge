#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "wincomp.h"
#include "winbox.h"
#include "winterfa.h"
#include "messbox.h"
#include "splitter.hpp"
#include "preproc.h"
#include "settings.h"
#include "moreio.h"
#include "Compiler.hpp"
#include "Interface.h"

#include "PercBar.h"

#define BOXX		13
#define BOXWIDTH	280
#define BOX1		86
#define BOX2		132
#define BOXHEIGHT	20

unsigned int percTotal[2] = {100, 100}, lastVal[2] = {0, 0};
extern HWND compWin;
HWND warningWindowH = NULL;
extern HWND mainWin;
extern HINSTANCE inst;

extern stringArray * functionNames;


extern char *fileList[];
extern int fileListNum;
extern char loadedFile[];

static void warningOpenSourceFile (char * fileName) {
	char * wholePath = new char[strlen (sourceDirectory) + strlen (fileName) + 2];
	if (wholePath) {
		sprintf (wholePath, "%s\\%s", sourceDirectory, fileName);
		if ((unsigned long) ShellExecute (warningWindowH, "open",
										  wholePath, NULL, NULL,
										  SW_SHOWNORMAL) <= 31) {
			errorBox ("Couldn't launch ", wholePath);
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
		if (MessageBox (warningWindowH, link->fullText, "SLUDGE Compiler warning", MB_YESNO | MB_APPLMODAL | MB_SETFOREGROUND) == IDYES)
		{
			warningOpenSourceFile(link->filename);
		}
	}
}

BOOL APIENTRY warningBoxFunc (HWND h, UINT m, WPARAM w, LPARAM l) {
	switch (m) {
		case WM_COMMAND:
            switch (LOWORD (w)) {
                case ID_WARNINGLIST:
                    if (HIWORD(w) == LBN_DBLCLK)
                    {
                        int n = SendMessage (GetDlgItem (h, ID_WARNINGLIST), LB_GETCURSEL, 0, 0);
                        if (n != LB_ERR)
                            userClickedErrorLine(n);
                    }
                    return 1;
               }

		case WM_INITDIALOG:
            warningWindowH = h;
            return 1;
	}

	return 0;
}

void compilerCommentsUpdated()
{
    if (! warningWindowH) {
        CreateDialog(inst, MAKEINTRESOURCE(WARNINGDIALOG), NULL, warningBoxFunc);
    }
    SendDlgItemMessage(warningWindowH, ID_WARNINGLIST, LB_RESETCONTENT, (WPARAM) 0, (LPARAM) 0);
    struct errorLinkToFile * index = errorList;
    if (! index) return;
    int i = numErrors - 1;
    while (i>=0) {
        SendDlgItemMessage(warningWindowH, ID_WARNINGLIST, LB_ADDSTRING, (WPARAM) 0, (LPARAM) index->fullText);
        if (! (index = index->next)) return;
        i--;
    }
}



void runCompiledGame () {
	char * wholePath = new char[strlen (sourceDirectory) + strlen (settings.finalFile) + 20];
	if (wholePath)
	{
		sprintf (wholePath, "%s\\%s%s", sourceDirectory, settings.finalFile, settings.forceSilent ? " (silent).slg" : ".slg");
		unsigned long reply = (unsigned long) ShellExecute (mainWin, "open", wholePath, NULL, NULL, SW_SHOWNORMAL);

		if (reply <= 31)
		{
			errorBox ("Compiled OK, but can't determine the location of the SLUDGE engine on this machine...",
                        "You HAVE installed the SLUDGE engine, haven't you?");
		}
		delete wholePath;
	}
}

void clearRect (int i, int whichBox) {
	percTotal[whichBox] = i ? i : 1;
	percRect (i ? 0 : 1, whichBox);
}

void percRect (unsigned int i, int whichBox) {
	HDC hdc = GetDC (compWin);
	char buffff[10];

	HPEN myPen = CreatePen (PS_SOLID, 0, RGB (0, 0, 0));
	HBRUSH myBrush = CreateSolidBrush (RGB (255, 255, 0));
	HPEN lastPen = (HPEN) SelectObject (hdc, myPen);
	HBRUSH lastBrush = (HBRUSH) SelectObject (hdc, myBrush);

	lastVal[whichBox] = i;

 	if (percTotal[whichBox]) {
        if (i > percTotal[whichBox]) i = percTotal[whichBox];
		sprintf (buffff, "%i%%", (i * 100) / percTotal[whichBox]);
		i = (i * BOXWIDTH) / percTotal[whichBox];
	} else {
		sprintf (buffff, "");
		i = 0;
	}

	if (whichBox) {
		if (i) {
			Rectangle (hdc, BOXX - 1, BOX2, BOXX + i, BOX2 + BOXHEIGHT);
		}
		SelectObject (hdc, lastBrush);
		DeleteObject (myBrush);

		myBrush = CreateSolidBrush (RGB (128, 0, 0));
		lastBrush = (HBRUSH) SelectObject (hdc, myBrush);
		Rectangle (hdc, BOXX - 1 + i, BOX2, BOXX + BOXWIDTH, BOX2 + BOXHEIGHT);
		SelectObject (hdc, lastBrush);
		DeleteObject (myBrush);
	} else {
		if (i) {
			Rectangle (hdc, BOXX - 1, BOX1, BOXX + i, BOX1 + BOXHEIGHT);
		}
		SelectObject (hdc, lastBrush);
		DeleteObject (myBrush);

		myBrush = CreateSolidBrush (RGB (0, 128, 0));
		lastBrush = (HBRUSH) SelectObject (hdc, myBrush);
		Rectangle (hdc, BOXX - 1 + i, BOX1, BOXX + BOXWIDTH, BOX1 + BOXHEIGHT);
		SelectObject (hdc, lastBrush);
		DeleteObject (myBrush);
	}

	SelectObject (hdc, lastPen);
	DeleteObject (myPen);
	ReleaseDC (compWin, hdc);
}

void setCompilerText (const compilerStatusText w, const char * theText)
{
    unsigned int id=0;
    switch (w) {
        case COMPILER_TXT_ACTION:
            id = COM_PROGTEXT;
            break;
        case COMPILER_TXT_FILENAME:
            id = COM_FILENAME;
            break;
        case COMPILER_TXT_ITEM:
            id = COM_ITEMTEXT;
            break;
    }
    if (id) SendDlgItemMessage(compWin, id, WM_SETTEXT, 0, (LPARAM)theText);
}

void setCompilerStats (int funcs, int objTypes, int files, int globals, int strings)
{
    char buf[10];
    sprintf(buf, "%d", funcs);
    SendDlgItemMessage(compWin, COM_NUM_FUNC, WM_SETTEXT, 0, (LPARAM)buf);
    sprintf(buf, "%d", objTypes);
    SendDlgItemMessage(compWin, COM_NUM_OBJ, WM_SETTEXT, 0, (LPARAM)buf);
    sprintf(buf, "%d", files);
    SendDlgItemMessage(compWin, COM_NUM_RES, WM_SETTEXT, 0, (LPARAM)buf);
    sprintf(buf, "%d", globals);
    SendDlgItemMessage(compWin, COM_NUM_GLOB, WM_SETTEXT, 0, (LPARAM)buf);
    sprintf(buf, "%d", strings);
    SendDlgItemMessage(compWin, COM_NUM_STRINGS, WM_SETTEXT, 0, (LPARAM)buf);
}


BOOL APIENTRY dialogComp(HWND h, UINT m, WPARAM w, LPARAM l) {
/*	static OPENFILENAME ofn;
	static char path[MAX_PATH];*/

	switch (m) {
		case WM_COMMAND:
		if (LOWORD (w) == IDCANCEL) {
			DestroyWindow (h);
			return 1;
		} else if (LOWORD (w) == IDOK) {
		    runCompiledGame();
			DestroyWindow (h);
			return 1;
		}
		break;

		case WM_APP:
            SendDlgItemMessage(compWin, COM_PROJTEXT, WM_SETTEXT, 0, (LPARAM)settings.finalFile);
            if (compileEverything(loadedFile, fileList, &fileListNum)) {
                EnableWindow (GetDlgItem (compWin, IDOK), true);
            }
            EnableWindow (GetDlgItem (compWin, IDCANCEL), true);
		break;

		case WM_INITDIALOG:
            compWin = h;
            /*
            GetCurrentDirectory (MAX_PATH, path);
            memset (&ofn, 0, sizeof (ofn));
            ofn.lStructSize = sizeof (ofn);
            ofn.hwndOwner = h;
            ofn.hInstance = inst;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrInitialDir = path;
            ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;
            */
            PostMessage (compWin, WM_APP, 0, 0);
            return 1;

		case WM_DESTROY:
            if (warningWindowH) DestroyWindow (warningWindowH);

            compWin = NULL;
            return 0;
	}
	return 0;
}
