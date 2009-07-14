
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "winterfa.h"
#include "moreio.h"
#include "messbox.h"
#include "wincomp.h"
#include "wintext.h"
#include "registry.h"
#include "settings.h"
#include "splitter.h"
#include "launch.h"

enum parseMode {PM_NORMAL, PM_QUOTE, PM_COMMENT, PM_FILENAME};

bool keepImages, keepStrings, verboseMode;
bool changed = false;

extern char * quitMessage;
extern char * customIcon;
char * runtimeDataFolder = NULL;
unsigned int screenWidth = 0, screenHeight = 0, frameSpeed = 0;
extern int winMouseImage;
extern bool runFullScreen;
extern bool forceSilent;
bool ditherImages = false;
bool searchSensitive = false;
extern chrRenderingSettingsStruct chrRenderingSettings;

bool startupShowLogo = true;
bool startupShowLoading = false;
extern bool startupInvisible;


#ifdef WIN32
#include <windows.h>

#include "dialogueBoxInOut.h"


//TODO? 

HWND mainWin=NULL;
HINSTANCE inst;
HMENU myMenu;

char * loader;
char * editor;
char * compiler;
char * helpFile;

char * loadedFile = NULL;

char * searchString = NULL;

/* Messaging macros */
#define MESS(id,m,w,l) SendDlgItemMessage(mainWin,id,m,(WPARAM)w,(LPARAM)l)
#define MESSP(id,m,w,l) SendDlgItemMessage(hDlg,id,m,(WPARAM)w,(LPARAM)l)

void setChanged (bool newVal);

static bool advancedLoadSaveSetup (HWND hDlg, dlgOperation operation)
{
	return (dialogueBoxTransferValue_Checkbox (hDlg, ID_STARTUP_INVISIBLE, 			& startupInvisible,   operation) &&
			dialogueBoxTransferValue_Checkbox (hDlg, ID_STARTUP_DISPLAY_SLUDGE, 	& startupShowLogo,    operation) &&
			dialogueBoxTransferValue_Checkbox (hDlg, ID_STARTUP_DISPLAY_LOADING,	& startupShowLoading, operation));
}

LRESULT CALLBACK projectAdvancedFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
/*
	switch (message) {
        case WM_INITDIALOG:
        {
        	if (startupInvisible) CheckDlgButton (hDlg, ID_STARTUP_INVISIBLE, BST_CHECKED);
        	if (startupShowLogo) CheckDlgButton (hDlg, ID_STARTUP_DISPLAY_SLUDGE, BST_CHECKED);
        	if (startupShowLoading) CheckDlgButton (hDlg, ID_STARTUP_DISPLAY_LOADING, BST_CHECKED);
			return (true);
		}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
				{
					startupInvisible = isChecked (hDlg, ID_STARTUP_INVISIBLE);
					startupShowLogo = isChecked (hDlg, ID_STARTUP_DISPLAY_SLUDGE);
					startupShowLoading = isChecked (hDlg, ID_STARTUP_DISPLAY_LOADING);
					setChanged (true);
				}

				case IDCANCEL:
				EndDialog(hDlg, true);
				return (true);
			}
			break;
	}

    return false;
 */
	dialogueBoxHandler (hDlg, message, wParam, lParam, advancedLoadSaveSetup);
}

static bool renderingLoadSaveSetup (HWND hDlg, dlgOperation operation)
{
	return (dialogueBoxTransferValue_Checkbox (hDlg, ID_RENDERING_AA_DEF_ENABLED, & chrRenderingSettings.defEnabled,   operation) &&
			dialogueBoxTransferValue_Checkbox (hDlg, ID_RENDERING_AA_MAX_ENABLED, & chrRenderingSettings.maxEnabled,   operation) &&
			dialogueBoxTransferValue_Checkbox (hDlg, ID_RENDERING_AA_MAX_READINI, & chrRenderingSettings.maxReadIni,   operation) &&
			dialogueBoxTransferValue_Int	  (hDlg, ID_RENDERING_AA_DEF_SOFT_X,  & chrRenderingSettings.defSoftnessX, operation, "Default softness (horizontal)") &&
			dialogueBoxTransferValue_Int	  (hDlg, ID_RENDERING_AA_DEF_SOFT_Y,  & chrRenderingSettings.defSoftnessY, operation, "Default softness (vertical)") &&
			dialogueBoxTransferValue_Int	  (hDlg, ID_RENDERING_AA_MAX_SOFT_X,  & chrRenderingSettings.maxSoftnessX, operation, "Maximum softness (horizontal)") &&
			dialogueBoxTransferValue_Int	  (hDlg, ID_RENDERING_AA_MAX_SOFT_Y,  & chrRenderingSettings.maxSoftnessY, operation, "Maximum softness (vertical)"));
}

LRESULT CALLBACK projectRenderingFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	dialogueBoxHandler (hDlg, message, wParam, lParam, renderingLoadSaveSetup);
}

LRESULT CALLBACK prefsBoxFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	static OPENFILENAME ofn;
	static char path[MAX_PATH];
	switch (message) {
        case WM_INITDIALOG:
        {
        	SetDlgItemInt (hDlg, ID_EDIT_WINWIDTH, screenWidth, 0);
        	SetDlgItemInt (hDlg, ID_EDIT_WINHEIGHT, screenHeight, 0);
        	SetDlgItemInt (hDlg, ID_EDIT_SPEED, frameSpeed, 0);
        	SetDlgItemText (hDlg, ID_EDIT_QUITMESSAGE, quitMessage);
        	SetDlgItemText (hDlg, ID_EDIT_CUSTOMICON, customIcon);
        	SetDlgItemText (hDlg, ID_EDIT_DATAFOLDER, runtimeDataFolder);

		MESSP (ID_MOUSE_IMAGE, CB_ADDSTRING, 0, "Standard arrow");
		MESSP (ID_MOUSE_IMAGE, CB_ADDSTRING, 0, "XOR crosshairs");
		MESSP (ID_MOUSE_IMAGE, CB_ADDSTRING, 0, "Invisible");
		MESSP (ID_MOUSE_IMAGE, CB_ADDSTRING, 0, "Big arrow");
		MESSP (ID_MOUSE_IMAGE, CB_SETCURSEL, winMouseImage, 0);

        	if (runFullScreen) CheckDlgButton (hDlg, ID_FULLSCREEN, BST_CHECKED);
        	if (forceSilent) CheckDlgButton (hDlg, ID_MAKESILENT, BST_CHECKED);
        	if (ditherImages) CheckDlgButton (hDlg, ID_DITHERIMAGES, BST_CHECKED);
			return (true);
		}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case ID_EDIT_SPEED:
				if (HIWORD(wParam) == 1024) {
					bool success = true;
					unsigned int newFrame = GetDlgItemInt (hDlg, ID_EDIT_SPEED, &success, 0);
					
					if (! success || newFrame > 1000 || newFrame < 1) {
			        	SetDlgItemText (hDlg, ID_EDIT_SPEED2, "???");
					} else {
			        	SetDlgItemInt (hDlg, ID_EDIT_SPEED2, 1000/newFrame, 0);
			        }
				}
				break;

				case IDOK:
				{
					char * newText;
					int le;
	
					bool success = true;
	
					unsigned int newHeight, newWidth = GetDlgItemInt (hDlg, ID_EDIT_WINWIDTH, & success, 0);
					if (success) {
						newHeight = GetDlgItemInt (hDlg, ID_EDIT_WINHEIGHT, & success, 0);
					}
					if (! success) {
						messageBox (er, "Screen width and height should be positive integer values.");
						return true;
					}
					
					unsigned int newFrame = GetDlgItemInt (hDlg, ID_EDIT_SPEED, &success, 0);
					if (! success || newFrame > 250 || newFrame < 1) {
						messageBox (er, "Milliseconds per frame (ms/frame) should be an integer between 1 and 250 inclusive.");
						return true;
					}
	
					frameSpeed = newFrame;
					screenWidth = newWidth;
					screenHeight = newHeight;
	
					// GET QUIT MESSAGE
					le = GetWindowTextLength (GetDlgItem (hDlg, ID_EDIT_QUITMESSAGE)) + 1;
					newText = new char[le];
					if (newText) {
						GetWindowText (GetDlgItem (hDlg, ID_EDIT_QUITMESSAGE), newText, le);
						delete quitMessage;
						quitMessage = newText;
					} else {
						errorBox ("Out of memory...", "Can't update quit message.");
					}

					// GET ICON FILENAME
					le = GetWindowTextLength (GetDlgItem (hDlg, ID_EDIT_CUSTOMICON)) + 1;
					newText = new char[le];
					if (newText) {
						GetWindowText (GetDlgItem (hDlg, ID_EDIT_CUSTOMICON), newText, le);
						delete customIcon;
						customIcon = newText;
					} else {
						errorBox ("Out of memory...", "Can't update custom icon filename.");
					}
					
					// GET DATA FOLDER NAME
					le = GetWindowTextLength (GetDlgItem (hDlg, ID_EDIT_DATAFOLDER)) + 1;
					newText = new char[le];
					if (newText) {
						GetWindowText (GetDlgItem (hDlg, ID_EDIT_DATAFOLDER), newText, le);
						delete runtimeDataFolder;
						runtimeDataFolder = newText;
					} else {
						errorBox ("Out of memory...", "Can't update run-time data folder name.");
					}
					
					// GET MORE STUFF
					runFullScreen = isChecked (hDlg, ID_FULLSCREEN);
					forceSilent = isChecked (hDlg, ID_MAKESILENT);
					ditherImages = isChecked (hDlg, ID_DITHERIMAGES);
					winMouseImage = MESSP (ID_MOUSE_IMAGE, CB_GETCURSEL, 0, 0);
//					hideWinMouse = isChecked (hDlg, ID_HIDEMOUSE);
					setChanged (true);
				}

				case IDCANCEL:
				EndDialog(hDlg, true);
				return (true);
				
				case ID_MOREOPTIONS:
				DialogBox(inst, MAKEINTRESOURCE(DLG_PROJECT_ADVANCED), hDlg, (DLGPROC)projectAdvancedFunc);
				break;

				case ID_RENDERING:
				DialogBox(inst, MAKEINTRESOURCE(DLG_PROJECT_RENDERING), hDlg, (DLGPROC)projectRenderingFunc);
				break;
			}
			break;
	}

    return false;
}

void upperString (char * s) {
	while (* s) {
		(* s) = toupper (* s);
		s ++;
	}
}

LRESULT CALLBACK resourcesBoxFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
        case WM_INITDIALOG:
        {
			int i, n = MESS (ID_FILELIST, LB_GETCOUNT, 0, 0);
       		char * tx;

			gotoSourceRoot ();
			
        	if (searchString) {
	        	SetDlgItemText (hDlg, IDOK, "Edit");
				SetWindowText (hDlg, "Search results");
				if (! searchSensitive) upperString (searchString);
        		for (i = 0; i < n; i ++) {
					tx = getFileFromBox (i);
					if (tx) {
						bool gotcha = false;
						char * entireFile = grabWholeFile (tx);
						
						if (entireFile) {
							if (! searchSensitive) upperString (entireFile);
							
							// Look for searchString...
							for (int i = 0; entireFile[i]; i ++) {
								int j;
								for (j = 0; searchString[j]; j ++) {
									if (entireFile[i + j] != searchString[j])
										break;
								}
								if (! searchString[j]) {
									gotcha = true;
									break;
								}
							}
							
							// If we found it, add the name of the file
							if (gotcha) {
		    	    			if (LB_ERR == SendDlgItemMessage (hDlg, ID_FILELIST, LB_FINDSTRINGEXACT, (WPARAM) -1, (LPARAM) tx)) {
									SendDlgItemMessage (hDlg, ID_FILELIST, LB_ADDSTRING, (WPARAM) 0, (LPARAM) tx);
								}
							}
							delete entireFile;
						}
						delete tx;
					}
				}
				delete searchString;
				searchString = NULL;
        	} else {
				FILE * fp;
				char t, lastOne;
				parseMode pM;
				char buffer[256];
				int numBuff;

				for (i = 0; i < n; i ++) {
					if (SendDlgItemMessage (mainWin, ID_FILELIST, LB_GETSEL, (WPARAM) i, (LPARAM) 0)) {
						tx = getFileFromBox (i);
						char * extension = tx + strlen(tx) - 4;						
//						errorBox ("Extension", extension);
						if (strlen (tx) > 4 && strcmp (extension, ".slu") == 0) {
							fp = fopen (tx, "rt");
							if (fp) {
//								errorBox ("Searching through", tx);
								pM = PM_NORMAL;
								t = ' ';
								for (;;) {
									lastOne = t;
									t = fgetc (fp);
									if (feof (fp)) break;
									switch (pM) {
										case PM_NORMAL:
										if (t == '\'') {
											pM = PM_FILENAME;
											numBuff = 0;
										}
										if (t == '\"') pM = PM_QUOTE;
										if (t == '#') pM = PM_COMMENT;
										break;
			
										case PM_COMMENT:
										if (t == '\n') pM = PM_NORMAL;
										break;
			
										case PM_QUOTE:
										if (t == '\"' && lastOne != '\\') pM = PM_NORMAL;
										break;
			
										case PM_FILENAME:
										if (t == '\'' && lastOne != '\\') {
											buffer[numBuff] = 0;
											pM = PM_NORMAL;
//											errorBox ("Just found resource file", buffer);
											if (LB_ERR == SendDlgItemMessage (hDlg, ID_FILELIST, LB_FINDSTRINGEXACT, (WPARAM) -1, (LPARAM) buffer)) {
												SendDlgItemMessage (hDlg, ID_FILELIST, LB_ADDSTRING, (WPARAM) 0, (LPARAM) buffer);
											}
										} else {
											buffer[numBuff++] = t;
											if (numBuff == 250) {
												buffer[numBuff++] = 0;
												errorBox ("Resource filename too long!", buffer);
												numBuff = 0;
											}
										}
										break;
									}
								}
								fclose (fp);
							} else {
								errorBox ("Can't open and hunt through", tx);
							}
						}
						else
						{
//							errorBox ("Skipping because not a .slu file", tx);
						}
						delete tx;
					}
				}
			}
			return (true);
		}

		case WM_COMMAND:
		switch (LOWORD(wParam)) {
			case IDOK:
			int n = SendDlgItemMessage (hDlg, ID_FILELIST, LB_GETCOUNT, (WPARAM) 0, (LPARAM) 0);
			for (int i = 0; i < n; i ++) {
				if (SendDlgItemMessage (hDlg, ID_FILELIST, LB_GETSEL, (WPARAM) i, (LPARAM) 0)) {
					char * me = new char[SendDlgItemMessage (hDlg, ID_FILELIST, LB_GETTEXTLEN, (WPARAM) i, (LPARAM) 0) + 1];
					SendDlgItemMessage (hDlg, ID_FILELIST, LB_GETTEXT, (WPARAM) i, (LPARAM) me);
					editFile (me, hDlg);
					delete me;
				}
			}
			return true;
			
			case IDCANCEL:
			EndDialog(hDlg, true);
			return true;
		}
		break;
	}

    return false;
}

LRESULT CALLBACK searchBoxFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
        case WM_INITDIALOG:
       	if (searchSensitive) CheckDlgButton (hDlg, ID_CASESENSITIVE, BST_CHECKED);
		return (true);

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
				int le = GetWindowTextLength (GetDlgItem (hDlg, ID_TEXT_EDITOR)) + 1;
				if (le == 1) {
					messageBox ("Find", "Please enter something to look for... What am I, a mindreader?");
					return 1;
				}
				searchString = new char[le];
				searchSensitive = isChecked (hDlg, ID_CASESENSITIVE);

				putRegSetting ("searchSensitive", searchSensitive);

				if (searchString) {
					GetWindowText (GetDlgItem (hDlg, ID_TEXT_EDITOR), searchString, le);
					DialogBox(inst, MAKEINTRESOURCE(999), mainWin, (DLGPROC)resourcesBoxFunc);
				} else {
					messageBox ("Find", "No memory for specified string.");
				}
				// No break; here
								
				case IDCANCEL:
				EndDialog(hDlg, true);
				return (true);
			}
			break;
	}

    return false;
}

HBITMAP hbitmap = NULL;

void drawLogo (HWND hDlg, int x, int y) {
	PAINTSTRUCT ps;
	HDC hDestDC = BeginPaint (hDlg, &ps);

	if (hbitmap) {
		HDC hSrcDC = CreateCompatibleDC (NULL);
		if (hSrcDC) {
			if (SelectObject (hSrcDC, hbitmap)) {
				BITMAP bmp;          		    // structure for bitmap info
				if (GetObject (hbitmap, sizeof (BITMAP), & bmp)) {
					BitBlt (hDestDC, x, y, bmp.bmWidth, bmp.bmHeight, hSrcDC, 0, 0, SRCCOPY);
				}
			}
			DeleteDC (hSrcDC);
		}
	}
	
	EndPaint (hDlg, &ps);
}

LRESULT CALLBACK aboutBoxFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
//        case WM_INITDIALOG:
//      	return true;
        
        case WM_PAINT:
		drawLogo (hDlg, 149, 12);
		return false;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
				gotoSite (hDlg, "http://www.hungrysoftware.com/");
				break;

				case IDCANCEL:
				EndDialog(hDlg, true);
				return (true);
			}
			break;
	}

    return false;
}

LRESULT CALLBACK settingsBoxFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	static OPENFILENAME ofn;
	static char path[MAX_PATH];
	switch (message) {
        case WM_INITDIALOG:
        {
		    SetWindowText (GetDlgItem (hDlg, ID_TEXT_EDITOR), editor);
			GetCurrentDirectory(MAX_PATH,path);
			memset(&ofn,0,sizeof(ofn));
			ofn.lStructSize=sizeof(ofn);
			ofn.hwndOwner=hDlg;
			ofn.hInstance=inst;
			ofn.nMaxFile=MAX_PATH;
			ofn.lpstrInitialDir=path;
			ofn.Flags=OFN_HIDEREADONLY|OFN_EXPLORER;
//			ShowWindow (hDlg, SW_SHOW);
			return (true);
		}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
				{
					int le = GetWindowTextLength (GetDlgItem (hDlg, ID_TEXT_EDITOR)) + 1;
					char * newEditor = new char[le];
					if (newEditor) {
						GetWindowText (GetDlgItem (hDlg, ID_TEXT_EDITOR), newEditor, le);
						delete editor;
						editor = newEditor;
						setRegString ("compilerEditor", editor);
					} else {
						errorBox ("Out of memory...", "Can't update editor location");
					}
				}

				case IDCANCEL:
				EndDialog(hDlg, true);
				return (true);
				
				case ID_TEXT_BROWSE:
				{
					char file[MAX_PATH]="";
					ofn.lpstrFilter="Executables (*.EXE)\0*.exe\0\0";
					ofn.lpstrFile=file;
					ofn.lpstrTitle = "Choose your text editor...";
					if (GetOpenFileName(&ofn)) {
						memcpy(path,file,ofn.nFileOffset);
						path[ofn.nFileOffset-1]=0;
						SetWindowText (GetDlgItem(hDlg, ID_TEXT_EDITOR), file);
					}
				}
				break;
			}
			break;
	}

    return false;
}

LRESULT CALLBACK sizeBoxFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
        case WM_INITDIALOG:
        {
        	SetDlgItemInt (hDlg, ID_EDIT_WINWIDTH, getRegInt ("utilityWidth", 640), 0);
        	SetDlgItemInt (hDlg, ID_EDIT_WINHEIGHT, getRegInt ("utilityHeight", 480), 0);
			if (! loadedFile) EnableWindow (GetDlgItem (hDlg, ID_USE_PROJECT_SIZE), false);
			return (true);
		}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case ID_USE_PROJECT_SIZE:
    	    	SetDlgItemInt (hDlg, ID_EDIT_WINWIDTH, screenWidth, 0);
	        	SetDlgItemInt (hDlg, ID_EDIT_WINHEIGHT, screenHeight, 0);
				return (true);
				
				case IDOK:
				{
					bool success;
					unsigned int newHeight, newWidth = GetDlgItemInt (hDlg, ID_EDIT_WINWIDTH, & success, 0);
					if (success) {
						newHeight = GetDlgItemInt (hDlg, ID_EDIT_WINHEIGHT, & success, 0);
					}
					if (! success || newHeight < 100 || newWidth < 100) {
						messageBox (er, "Width and height should both be positive integer values bigger than 99");
						return true;
					}
					if (! putRegInt ("utilityWidth", newWidth))		return true;
					if (! putRegInt ("utilityHeight", newHeight))	return true;
				}

				case IDCANCEL:
				EndDialog(hDlg, true);
				return (true);
			}
			break;
	}

    return false;
}

void setTick (int what, bool yesNo) {
	CheckMenuItem (myMenu, what, yesNo ? MF_CHECKED : MF_UNCHECKED);
}

void setEverything (bool on) {
	EnableWindow (GetDlgItem (mainWin, ID_FILELIST), on);
	EnableWindow (GetDlgItem (mainWin, ID_PROJECT_COMPILE), on);
	EnableWindow (GetDlgItem (mainWin, ID_PROJECT_RUN), on);
	EnableWindow (GetDlgItem (mainWin, ID_DISPLAY), on);
	EnableWindow (GetDlgItem (mainWin, ID_PROJECT_SAVE), on && changed);
	EnableWindow (GetDlgItem (mainWin, ID_CONTENTS_ADD_FILE), on);
	EnableWindow (GetDlgItem (mainWin, ID_CONTENTS_FIND_IN_FILE), on);
	EnableWindow (GetDlgItem (mainWin, ID_CONTENTS_RESOURCES), on);
	EnableWindow (GetDlgItem (mainWin, ID_CONTENTS_EDIT_FILE), on);
	EnableWindow (GetDlgItem (mainWin, ID_CONTENTS_DELETE_FILE), on);
	EnableWindow (GetDlgItem (mainWin, ID_EDIT_NAME), on);
	EnableWindow (GetDlgItem (mainWin, ID_EDIT_OUTPUTFILE), on);
//	EnableWindow (GetDlgItem (mainWin, ID_EDIT_QUITMESSAGE), on);
//	EnableWindow (GetDlgItem (mainWin, ID_EDIT_WINWIDTH), on);
//	EnableWindow (GetDlgItem (mainWin, ID_EDIT_WINHEIGHT), on);
	
	EnableMenuItem (myMenu, ID_PROJECT_COMPILE, on ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem (myMenu, ID_PROJECT_RUN, on ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem (myMenu, ID_PROJECT_SAVEAS, on ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem (myMenu, ID_PROJECT_CLOSE, on ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem (myMenu, ID_CONTENTS_ADD_FILE, on ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem (myMenu, ID_CONTENTS_FIND_IN_FILE, on ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem (myMenu, ID_CONTENTS_RESOURCES, on ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem (myMenu, ID_DISPLAY, on ? MF_ENABLED : MF_GRAYED);

	EnableMenuItem (myMenu, ID_PROJECT_SAVE, (on && changed) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem (myMenu, ID_PROJECT_REVERT, (on && changed) ? MF_ENABLED : MF_GRAYED);
}

void updateTitle () {
	if (loadedFile) {
		char buff[500];
		sprintf (buff, "SLUDGE Project Manager: %s%s", changed ? "* " : "", loadedFile);
		SetWindowText (mainWin, buff);
		setEverything (true);
		setWindowText (ID_LOADED_FILE, loadedFile);
	} else {
		SetWindowText (mainWin, "SLUDGE Project Manager");
		setEverything (false);
		setWindowText (ID_LOADED_FILE, "No project loaded");
	}
}

void setChanged (bool newVal) {
	changed = newVal;
	updateTitle ();
}

char * getFileFromBox (int i) {
	char * me = new char[MESS (ID_FILELIST, LB_GETTEXTLEN, i, 0) + 1];
	MESS (ID_FILELIST, LB_GETTEXT, i, me);
	return me;
}

void loadProject (char * filename) {
	char * readLine;
	FILE * fp = fopen (filename, "rt");
	if (! fp) return;

	readSettings (fp);
	for (;;) {
		readLine = readText (fp);
		if (readLine == NULL) break;
		if (strcmp (readLine, "[FILES]") == 0) break;
		messageBox ("Skipping", readLine);
		delete readLine;
	}

	if (readLine) delete readLine;

	MESS(ID_FILELIST, LB_RESETCONTENT, 0, 0);
	for (;;) {
		readLine = readText (fp);
		if (readLine == NULL) break;
		if (readLine[0]) MESS(ID_FILELIST, LB_ADDSTRING, 0, readLine);
		delete readLine;
	}

	fclose (fp);
	if (filename != loadedFile) {
		delete loadedFile;
		loadedFile = joinStrings (filename, "");
	}
	setChanged (false);
}

bool saveProject (char * filename) {
	FILE * fp = fopen (filename, "wt");
	if (! fp) {
		errorBox ("Can't write to project file", filename);
		return false;
	}

	writeSettings (fp);
	
	// Now write out the list of files...
	
	char * tx;
	int i, n = MESS (ID_FILELIST, LB_GETCOUNT, 0, 0);
	
	for (i = 0; i < n; i ++) {
		tx = getFileFromBox (i);
		fprintf (fp, "%s\n", tx);
		delete tx;
	}
	
	fclose (fp);

	if (filename != loadedFile) {
		delete loadedFile;
		loadedFile = joinStrings (filename, "");
	}
	setChanged (false);
	return true;
}

void closeProject () {
	blankSettings ();
	delete loadedFile;
	loadedFile = NULL;
	MESS(ID_FILELIST, LB_RESETCONTENT, 0, 0);
	updateTitle ();
}

void newProject (char * filename) {
	noSettings ();
	MESS(ID_FILELIST, LB_RESETCONTENT, 0, 0);
	if (! saveProject (filename)) closeProject ();
}

void addFileToProject (char * wholeName) {
	int a = 0;
	char * newName, * temp;
//	messageBox (wholeName, loadedFile);
	while (wholeName[a] == loadedFile[a]) {
		a ++;
	}
	
	for (;;) {
		if (a == 0) break;
		if (wholeName[a - 1] == '\\') break;
		a --;
	}
	
	newName = joinStrings ("", wholeName + a);
	while (loadedFile[a]) {
		if (loadedFile[a] == '\\') {
			temp = joinStrings ("..\\", newName);
			delete newName;
			newName = temp;
//			messageBox (newName, loadedFile + a);
		}
		a ++;
	}
//	messageBox ("Final: ", newName);
	MESS(ID_FILELIST, LB_ADDSTRING, 0, newName);
	MESS(ID_FILELIST, LB_SELECTSTRING, 0, newName);
	delete newName;
}

bool checkOverwrite (char * fn) {
	FILE * fp = fopen (fn, "rb");
	if (fp) {
		fclose (fp);
		char * buff = joinStrings ("That file already exists! Are you sure you want to overwrite the following file?\n\n", fn);
		if (MessageBox (mainWin, buff, "Overwrite?", MB_TASKMODAL | MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND) == IDYES) {
			delete buff;
			return true;
		} else {
			delete buff;
			return false;
		}
	}
	return true;
}

void fixExtension (char * buff, char * ext) {
	int a = strlen (buff);
	if (buff[a - 4] != '.' ||
		(tolower (buff[a - 3]) != ext[0]) ||
		(tolower (buff[a - 2]) != ext[1]) ||
		(tolower (buff[a - 1]) != ext[2])) {
		buff[a] = '.';
		buff[a + 1] = ext[0];
		buff[a + 2] = ext[1];
		buff[a + 3] = ext[2];
		buff[a + 4] = NULL;
	}
}

bool APIENTRY dialogproc (HWND h, UINT m, WPARAM w, LPARAM l) {

	static OPENFILENAME ofn;
	static char path[MAX_PATH];

	switch (m) {
//		case WM_TIMER:
//			{
//				sprintf(text,"%.1f",BASS_GetCPU());
//				MESS(ID_CPU,WM_SETTEXT,0,text);
//			}
//			break;

		case WM_COMMAND:
		switch (LOWORD(w)) {
			case ID_EDIT_NAME:
			case ID_EDIT_OUTPUTFILE:
			if (HIWORD(w) == 1024) setChanged (true);
			break;
			
			case ID_SETTINGSBOX:
			DialogBox(inst, MAKEINTRESOURCE(DLG_FILE_LOCATIONS), h, (DLGPROC)settingsBoxFunc);
			break;

			case ID_SIZEBOX:
			DialogBox(inst, MAKEINTRESOURCE(DLG_WINDOW_SIZES), h, (DLGPROC)sizeBoxFunc);
			break;

			case ID_DISPLAY:
			DialogBox(inst, MAKEINTRESOURCE(DLG_PROJECT_PROPERTIES), h, (DLGPROC)prefsBoxFunc);
			break;

//			case ID_REGISTER:
//			DialogBox(inst, MAKEINTRESOURCE(DLG_REGISTRATION_INFO), h, (DLGPROC)regBoxFunc);
//			break;

			case ID_CONTENTS_ADD_FILE:
			{
				char file[MAX_PATH]="";
				ofn.lpstrFilter="SLUDGE scripts (*.SLU)\0*.slu\0SLUDGE definition files (*.SLD)\0*.sld\0SLUDGE translation files (*.TRA)\0*.tra\0";
				ofn.lpstrFile=file;
				ofn.lpstrTitle = "Add a file to the current project...";
				ofn.Flags=OFN_HIDEREADONLY|OFN_EXPLORER;
				if (GetOpenFileName(&ofn)) {
					memcpy(path,file,ofn.nFileOffset);
					path[ofn.nFileOffset-1]=0;
					switch (ofn.nFilterIndex) {
						case 1:
						fixExtension (file, "slu");
						break;
						
						case 2:
						fixExtension (file, "sld");
						break;
						
						case 3:
						fixExtension (file, "tra");
						break;
					}
					FILE * tester = fopen (file, "rb");

					bool ok = true;
					if (tester) {
						fclose (tester);
					} else {
						char * q1 = joinStrings ("The file \"", file);
						char * q2 = joinStrings (q1, "\" doesn't exist at the moment. Do you want to create it now?");
						delete q1;
						ok = MessageBox (mainWin, q2, "Non-existant file!", MB_YESNO | MB_SETFOREGROUND) == IDYES;
						delete q2;

						if (ok) {
							tester = fopen (file, "wb");
							fclose (tester);
						}
					}

					if (ok) {
						addFileToProject (file);
						setChanged (true);
					}
				}
			}
			break;
			
			case ID_PROJECT_SAVE:
			saveProject (loadedFile);
			break;

			case ID_PROJECT_REVERT:
			loadProject (loadedFile);
			break;

			case ID_PROJECT_CLOSE:		// My menu thingy
			closeProject ();
			break;
			
			case IDCANCEL:				// X button
			case ID_EXIT:
			PostQuitMessage (0);
			break;
			
			case ID_UPGRADES:
			checkForUpgrades (h);
			break;
			
			case ID_LAUNCH_FLOOR:
			case ID_LAUNCH_Z:
			case ID_LAUNCH_ENGINE:
			case ID_LAUNCH_TRANS:
			case ID_LAUNCH_SPRITE:
			launchOtherKitProgram (LOWORD(w), h);
			break;

			case ID_PROJECT_SAVEAS:
			{
				char file[MAX_PATH]="";
				messageBox ("Important! If you save this project into a different directory, the project manager and compiler will not be able to find the SLUDGE scripts (as all locations are relative to the location of the project file).", "To move an entire project to a new location, please use Windows to copy all the required files, or clear the list of files and add them again.");
				sprintf (file, loadedFile);
				ofn.lpstrFilter="SLUDGE projects (*.SLP)\0*.slp\0\0";
				ofn.lpstrFile=file;
				ofn.lpstrTitle = "Save this SLUDGE project as...";
				ofn.Flags=OFN_HIDEREADONLY|OFN_EXPLORER;
				if (GetSaveFileName(&ofn)) {
					memcpy(path,file,ofn.nFileOffset);
					path[ofn.nFileOffset-1]=0;
					fixExtension (file, "slp");
					if (checkOverwrite (file)) {
						saveProject (file);
					}
				}
			}
			break;
			
			case ID_PROJECT_NEW:
			{
				char file[MAX_PATH]="New.slp";
//				sprintf (file, loadedFile);
				ofn.lpstrFilter="SLUDGE projects (*.SLP)\0*.slp\0\0";
				ofn.lpstrTitle = "Create a new SLUDGE project...";
				ofn.lpstrFile=file;
				ofn.Flags=OFN_HIDEREADONLY|OFN_EXPLORER;
				if (GetSaveFileName(&ofn)) {
					memcpy(path,file,ofn.nFileOffset);
					path[ofn.nFileOffset-1]=0;
					fixExtension (file, "slp");
					if (checkOverwrite (file)) {
						newProject (file);
					}
				}
			}
			break;
			
			case ID_CONTENTS_FIND_IN_FILE:
			DialogBox(inst, MAKEINTRESOURCE(666), h, (DLGPROC)searchBoxFunc);
			break;

			case ID_CONTENTS_RESOURCES:
			DialogBox(inst, MAKEINTRESOURCE(999), h, (DLGPROC)resourcesBoxFunc);
			break;

			case ID_ABOUT:
			DialogBox(inst, MAKEINTRESOURCE(1234), h, (DLGPROC)aboutBoxFunc);
			break;

			case ID_SLUDGE_HELP:
//			WinHelp (mainWin, helpFile, HELP_FINDER, 0);
			editFile (helpFile, mainWin);
			break;
			
/*			case ID_CONTENTS_NEW_PROJECT:
			MESS(ID_FILELIST, LB_RESETCONTENT, 0, 0);
			break;
*/
			case ID_PROJECT_COMPILE:
			saveProject (loadedFile);
			runProg (compiler, "", loadedFile);
			break;

			case ID_PROJECT_RUN:
			saveProject (loadedFile);
			runProg (compiler, "-r ", loadedFile);
			break;

			case ID_CONTENTS_DELETE_FILE:
			{
				int i, n = MESS (ID_FILELIST, LB_GETCOUNT, 0, 0);
				
				for (i = 0; i < n; i ++) {
					if (MESS (ID_FILELIST, LB_GETSEL, i, 0)) {
						MESS (ID_FILELIST, LB_DELETESTRING, i, 0);
						setChanged (true);
						i --;
						n --;
					}
				}
			}
			break;

			case ID_CONTENTS_EDIT_FILE:
			{
				int i, n = MESS (ID_FILELIST, LB_GETCOUNT, 0, 0);
				
				for (i = 0; i < n; i ++) {
					if (MESS (ID_FILELIST, LB_GETSEL, i, 0)) {
						char * fn = getFileFromBox (i);
						editFile (fn, h);
//						runProg (editor, "", fn);
						delete fn;
					}
				}
			}
			break;

			case ID_KEEP_IMAGES:
			if (putRegSetting ("compilerKillImages", keepImages)) {
				keepImages = ! keepImages;
				setTick (ID_KEEP_IMAGES, keepImages);
			}
			break;

			case ID_KEEP_STRINGS:
			if (putRegSetting ("compilerWriteStrings", ! keepStrings)) {
				keepStrings = ! keepStrings;
				setTick (ID_KEEP_STRINGS, keepStrings);
			}
			break;
						
			case ID_VERBOSE:
			if (putRegSetting ("compilerVerbose", ! verboseMode)) {
				verboseMode = ! verboseMode;
				setTick (ID_VERBOSE, verboseMode);
			}
			break;

			case ID_PROJECT_LOAD:
			{
				char file[MAX_PATH]="";
				ofn.lpstrFilter = "SLUDGE projects (*.SLP)\0*.slp\0\0";
				ofn.lpstrFile = file;
				ofn.lpstrTitle = "Load a SLUDGE project...";
				ofn.Flags=OFN_HIDEREADONLY|OFN_EXPLORER;
				if (GetOpenFileName(&ofn)) {
					memcpy(path,file,ofn.nFileOffset);
					path[ofn.nFileOffset-1]=0;
					loadProject (file);
				}
			}
			break;
		}
		break;

		case WM_INITDIALOG:

		// Set up the menu
		myMenu = LoadMenu (inst, MAKEINTRESOURCE(1));
		SetMenu (h, myMenu);
		verifyOtherKitPrograms ();
		
		mainWin=h;
		SetClassLong(h, GCL_HICON, (LONG) LoadIcon(inst, MAKEINTRESOURCE(3)));
		GetCurrentDirectory(MAX_PATH,path);
		memset(&ofn,0,sizeof(ofn));
		ofn.lStructSize=sizeof(ofn);
		ofn.hwndOwner=h;
		ofn.hInstance=inst;
		ofn.nMaxFile=MAX_PATH;
		ofn.lpstrInitialDir=path;
//      noSettings ();

		keepImages = ! getRegSetting ("compilerKillImages");
		setTick (ID_KEEP_IMAGES, keepImages);

		keepStrings = getRegSetting ("compilerWriteStrings");
		setTick (ID_KEEP_STRINGS, keepStrings);

		verboseMode = getRegSetting ("compilerVerbose");
		setTick (ID_VERBOSE, verboseMode);

		searchSensitive = getRegSetting ("searchSensitive");
		editor = getRegString ("compilerInstallDir");
		if (editor) {
			compiler = joinStrings (editor, "\\SLUDGE Compiler.exe");
			helpFile = joinStrings (editor, "\\Help\\SLUDGE.chm");
			delete editor;
		} else {
			compiler = joinStrings ("", "");
			helpFile = joinStrings (".", "\\Help\\SLUDGE.chm");
		}
		editor = getRegString ("compilerEditor");
		if (! editor) editor = joinStrings ("notepad.exe", "");
		if (loader[0]) {
//			messageBox ("Trying to load...", loader);
			loadProject (loader);
		} else {
			updateTitle ();
		}
		return 1;

        case WM_PAINT:
		drawLogo (h, 301, 257);
		return false;

		case WM_DESTROY:
		WinHelp (mainWin, helpFile, HELP_QUIT, 0);
		PostQuitMessage(0);
		return 1;
	}
	#pragma unused(l)
	return 0;
}


int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	inst=hInstance;
	loader = lpCmdLine;
	if (loader[0] == '\"') {
		stringArray * bits = splitString (loader + 1, '\"', ONCE);
		loader = joinStrings (bits -> string, "");
		while (destroyFirst (bits)) {;}
	}

	hbitmap = (HBITMAP) LoadImage (hInstance, MAKEINTRESOURCE (1235),
                                    IMAGE_BITMAP, 0, 0,
                                    LR_CREATEDIBSECTION);

	if (!CreateDialog(hInstance, MAKEINTRESOURCE(DLG_MAIN), NULL, dialogproc)) {
		return 0;
	}
	
	while (1) {
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

	cleanUpFiles ();
	return 0;
}

#endif