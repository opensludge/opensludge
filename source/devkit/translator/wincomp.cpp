#ifdef WIN32

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "WINTERFA.H"
#include "MOREIO.H"
#include "MessBox.h"
#include "wincomp.h"
#include "wintext.h"
#include "SPLITTER.HPP"
#include "sosfile.h"

HWND mainWin=NULL;
HINSTANCE inst;
HMENU myMenu;

char * loader;

#define FILEFILTER	"SLUDGE translation files (*.TRA)\0*.tra\0\0"
#define MESS(id,m,w,l) SendDlgItemMessage(mainWin,id,m,(WPARAM)w,(LPARAM)l)

char * registryGetString (char * settingName) {
	HKEY gotcha;
	int r;
	char * buff = NULL; 
	unsigned long si = 8;

	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Software\\Hungry Software\\SLUDGE Translation Editor", 0, KEY_READ, & gotcha) != ERROR_SUCCESS) return NULL;
	r = RegQueryValueEx (gotcha, settingName, NULL, NULL, NULL, & si);
	if (r == ERROR_SUCCESS) {
		buff = new char[si + 1];
		if (buff) {
			r = RegQueryValueEx (gotcha, settingName, NULL, NULL, (unsigned char *) buff, & si);
			if (r != ERROR_SUCCESS) {
				delete buff;
				buff = NULL;
			} else if (buff[0]) {
				int j = strlen (buff);
				if (buff[j - 1] == '\\' || buff[j - 1] == '/')
					buff[j - 1] = NULL;
			}
		}			
	}
	RegCloseKey (gotcha);
	return buff;
}

char * searchString = NULL;

LRESULT CALLBACK searchBoxFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
        case WM_INITDIALOG:
       	if (programSettings.searchSensitive) CheckDlgButton (hDlg, ID_CASESENSITIVE, BST_CHECKED);
		return (true);

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
				int le = GetWindowTextLength (GetDlgItem (hDlg, ID_SEARCH)) + 1;
				if (le == 1) {
					messageBox ("Find", "Please enter something to look for... What am I, a mindreader?");
					return 1;
				}
				searchString = new char[le];
				programSettings.searchSensitive = IsDlgButtonChecked (hDlg, ID_CASESENSITIVE) == BST_CHECKED;

//				putRegSetting ("searchSensitive", searchSensitive);

				if (searchString) {
					GetWindowText (GetDlgItem (hDlg, ID_SEARCH), searchString, le);
				} else {
					messageBox ("Find", "No memory for specified string.");
				}
				// No break; here
				if (! programSettings.searchSensitive) CharUpper (searchString);
				autoSelectContent (TYPE_SEARCH);

				case IDCANCEL:
				EndDialog(hDlg, true);
				return (true);
			}
			break;
	}

    return false;
}

bool APIENTRY dialogproc (HWND h, UINT m, WPARAM w, LPARAM l) {

	static OPENFILENAME ofn;
	static char path[MAX_PATH];

	switch (m) {
		case WM_COMMAND:
		switch (LOWORD(w)) {
			case ID_ENTER_TRANSLATION:
			if (HIWORD(w) == 1024) setChanged (true);
			break;

			case IDCANCEL:				// X button
			if (trashProgress ("quit without saving")) PostQuitMessage (0);
			break;
			
			case ID_PROJECT_REVERT:
			if (trashProgress ("revert to how this file was the last time it was saved")) {
				loadFile (loader);
			}
			break;
						
			case ID_CATEGORY:
			if (HIWORD(w) == CBN_SELCHANGE) updateStringList ();
			break;

			case IDOK:
			commitChanges ();
			break;			
			
			case ID_STRINGS:
			if (HIWORD(w) != CBN_SELCHANGE) break;
			// No break;
			
			case ID_UNDO:
			{
				int n = MESS (ID_STRINGS, LB_GETCURSEL, 0, 0);
				char * thisIsMe = new char[MESS (ID_STRINGS, LB_GETTEXTLEN, n, 0) + 1];
				if (thisIsMe) {
					MESS (ID_STRINGS, LB_GETTEXT, n, thisIsMe);
					selectedString (thisIsMe);
					delete thisIsMe;
					setChanged (false);
				}
			}
			break;
			
			case ID_PROJECT_NEW:
			if (trashProgress ("finish with the current file and start a new one")) {
				newFile ();
			}
			break;
			
			case ID_PROJECT_LOAD:
			if (trashProgress ("finish with the current file and load a new one")) {
				char file[MAX_PATH]="";
				file[0] = 0;
				ofn.lpstrFilter=FILEFILTER;
				ofn.lpstrFile=file;
				ofn.lpstrTitle = "Choose a SLUDGE translation file to load...";
				ofn.lpstrDefExt = NULL;
				ofn.Flags=OFN_EXPLORER | OFN_FILEMUSTEXIST;
				if (GetOpenFileName(&ofn)) {
					memcpy(path,file,ofn.nFileOffset);
					path[ofn.nFileOffset-1]=0;
					loadFile (file);
				}
			}
			break;
			
			case ID_GOT_TRANSLATION:
			{
				if (IsDlgButtonChecked (h, ID_GOT_TRANSLATION) == BST_CHECKED) {
					enableTranslationBox (0, NULL);
				} else {
					enableTranslationBox (5, NULL);
				}
				setChanged (true);
			}
			break;
			
			case ID_SEARCH:
			DialogBox(inst, MAKEINTRESOURCE(666), h, (DLGPROC)searchBoxFunc);
			break;
			
			case ID_HELP:
			{
				char * folder = registryGetString ("editorInstallDir");
				if (folder) {
					char * wholeString = joinStrings (folder, "\\help.html");
					delete folder;
					if (wholeString) {
						if ((int) ShellExecute (h, "open", wholeString, NULL, NULL,
																	  SW_SHOWNORMAL) >= 32) {
							delete wholeString;
							break;
						}
						errorBox ("Couldn't launch help file:\n"
						  "Please open the file HELP.HTML from the translation editor folder yourself, or choose \"SLUDGE translation editor documentation\" from your Windows Start menu.\n\nFilename",
						  wholeString);

						delete wholeString;
						break;
					}
				}
				errorBox ("Couldn't launch help file",
						  "Please open the file HELP.HTML from the translation editor folder yourself, or choose \"SLUDGE translation editor documentation\" from your Windows Start menu.");
			}
			break;

			case ID_PROJECT_UPDATE:
			{
				char file[MAX_PATH]="";
				file[0] = 0;
				ofn.lpstrFilter="SLUDGE projects (*.SLP)\0*.slp\0\0";
				ofn.lpstrFile=file;
				ofn.lpstrTitle = "Choose a SLUDGE project to open...";
				ofn.lpstrDefExt = NULL;
				ofn.Flags=OFN_EXPLORER | OFN_FILEMUSTEXIST;
				if (GetOpenFileName(&ofn)) {
					memcpy(path,file,ofn.nFileOffset);
					path[ofn.nFileOffset-1]=0;
					updateFromProject (file);
				}
			}
			break;

			case ID_PROJECT_SAVEAS:
			case ID_PROJECT_SAVE:
			{
				bool didItWork = false;
				unsigned int languageID = GetDlgItemInt (mainWin, ID_ID, &didItWork, false);
				if (didItWork && languageID > 0 && languageID <= 65535) {
					char * trySavingMe = NULL;
	
					if (LOWORD(w) == ID_PROJECT_SAVE && loader[0]) {
						trySavingMe = copyString (loader);
					} else {
						char file[MAX_PATH]="";
						if (loader && loader[0]) {
							strcpy (file, loader);
						} else {
							strcpy (file, "ENTER LANGUAGE NAME HERE.tra");
						}
						ofn.lpstrFilter=FILEFILTER;
						ofn.lpstrFile=file;
						ofn.lpstrTitle = "Save this translation data into a file...";
						ofn.lpstrDefExt = "tra";
						ofn.Flags=OFN_HIDEREADONLY | OFN_EXPLORER | OFN_PATHMUSTEXIST;
		
						if (GetSaveFileName(&ofn)) {
							memcpy(path,file,ofn.nFileOffset);
							path[ofn.nFileOffset-1]=0;
							trySavingMe = copyString (file);
						} else {
							break;
						}
					}
					
					if (trySavingMe) {
						saveToFile (trySavingMe, languageID);
						delete trySavingMe;
					}
				} else {
					errorBox ("Invalid ID number", "The \"ID number\" value (in the top left of your window) must be between 1 and 65535. Change the value in the box and try again, please...");
				}
			}
		}
		break;

		case WM_INITDIALOG:
//		errorBox ("WM_INITDIALOG...", "");
		myMenu = LoadMenu (inst, MAKEINTRESOURCE(1));
		SetMenu (h, myMenu);
		mainWin=h;
		SetClassLong(h, GCL_HICON, (LONG) LoadIcon(inst, MAKEINTRESOURCE(2)));
		SetClassLong(h, GCL_HICONSM, (LONG) LoadIcon(inst, MAKEINTRESOURCE(3)));
		GetCurrentDirectory(MAX_PATH,path);
		memset(&ofn,0,sizeof(ofn));
		ofn.lStructSize=sizeof(ofn);
		ofn.hwndOwner=h;
		ofn.hInstance=inst;
		ofn.nMaxFile=MAX_PATH;
		ofn.lpstrInitialDir=path;

		MESS (ID_CATEGORY, CB_ADDSTRING, 0, "NEW STRINGS NOT YET TRANSLATED");
		MESS (ID_CATEGORY, CB_ADDSTRING, 0, "STRINGS NOT REQUIRING TRANSLATIONS");
		MESS (ID_CATEGORY, CB_ADDSTRING, 0, "STRINGS WITH TRANSLATIONS");
		MESS (ID_CATEGORY, CB_ADDSTRING, 0, "SEARCH RESULTS");
		MESS (ID_CATEGORY, CB_SETCURSEL, 0, 0);
		EnableWindow (GetDlgItem (mainWin, ID_ORIGINAL_TEXT), false);

//		errorBox ("I want to load the following file", loader);
		if (loader[0])
			loadFile (loader);
		else
			newFile ();
			
		return 1;
	}
	#pragma unused(l)
	return 0;
}

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	inst=hInstance;
	loader = copyString (lpCmdLine);
	if (loader[0] == '\"') {
		stringArray * bits = splitString (loader + 1, '\"', ONCE);
//		errorBox ("Changed", loader);
		delete loader;
		loader = copyString (bits -> string);
		while (destroyFirst (bits)) {;}
//		errorBox ("into", loader);
	}

	if (CreateDialog(hInstance, MAKEINTRESOURCE(500), NULL, dialogproc)) {
		for (;;) {
			if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
				if (!GetMessage(&msg, NULL, 0, 0))
					break;
				if (! IsDialogMessage (mainWin, &msg)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			} else
				WaitMessage();
		}
	}	
	return 0;
}


#endif