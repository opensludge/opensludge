#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>

#include "allfiles.h"

#include "winstuff.h"
#include "platform-dependent.h"
#include "newfatal.h"
#include "sprites.h"
#include "sprbanks.h"
#include "fonttext.h"
#include "backdrop.h"
#include "sludger.h"
#include "cursors.h"
#include "objtypes.h"
#include "region.h"
#include "people.h"
#include "talk.h"
#include "direct.h"
#include "sound.h"
#include "colours.h"
#include "moreio.h"
#include "stringy.h"

#include <shellapi.h>
#include <shlobj.h> // For SHGetFolderPath

#include "..\..\images\resource.h"

HINSTANCE hInst;  				// Handle of the main instance
extern HWND hMainWindow;

extern variableStack * noStack;

extern settingsStruct gameSettings;

// The platform-specific functions - Windows edition.

char * grabFileName () {
	OPENFILENAME ofn;
	char path[MAX_PATH];
	char file[MAX_PATH]="";

	hInst = GetModuleHandle(NULL);

	memset (& ofn, 0, sizeof (ofn));
	ofn.lStructSize = sizeof (ofn);
	ofn.hwndOwner = NULL;
	ofn.hInstance = hInst;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrInitialDir = path;
	ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;
	ofn.lpstrFilter = "SLUDGE games (*.SLG)\0*.slg\0\0";
	ofn.lpstrFile = file;

	if (GetOpenFileName (& ofn)) {
		return copyString (file);
	} else {
		return NULL;
	}
}

extern char ** languageName;
extern int * languageTable;

BOOL CALLBACK setupDlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_INITDIALOG:
        if (gameSettings.userFullScreen)
            CheckDlgButton (hDlg, 1000, BST_CHECKED);
        else
            CheckDlgButton (hDlg, 1000, BST_UNCHECKED);

        if (gameSettings.numLanguages) {
          char text[20];
          for (unsigned int i = 0; i<=gameSettings.numLanguages; i++) {
                if (languageName[i])
                    SendDlgItemMessage(hDlg, 1001, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>((LPCTSTR)languageName[i]));
                else {
                    sprintf(text, "Language %d", i);
                    SendDlgItemMessage(hDlg, 1001, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>((LPCTSTR)text));
                }
          }
          SendDlgItemMessage(hDlg, 1001, CB_SETCURSEL, getLanguageForFileB(), 0);
       } else {
            const char * text = "No translations available";
            SendDlgItemMessage(hDlg, 1001, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>((LPCTSTR)text));
            SendDlgItemMessage(hDlg, 1001, CB_SETCURSEL, 0, 0);
            EnableWindow(GetDlgItem(hDlg, 1001), false);
        }
        return true;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
            case IDOK:

            gameSettings.userFullScreen = (IsDlgButtonChecked(hDlg, 1000) == BST_CHECKED);
            if (gameSettings.numLanguages) {
                gameSettings.languageID = SendDlgItemMessage(hDlg, 1001, CB_GETCURSEL, 0, 0);
                if (gameSettings.languageID < 0) gameSettings.languageID = 0;
                gameSettings.languageID = languageTable[gameSettings.languageID];
            }
            EndDialog (hDlg, true);
            return TRUE;

            case IDCANCEL:
            EndDialog (hDlg, false);
            return TRUE;
        }
        break;
    }
    return false;
}

int showSetupWindow() {

 	hInst = GetModuleHandle(NULL);

    if (! hInst) fprintf (stderr, "ERROR: No hInst!\n");

    if (DialogBox (hInst, "SETUPWINDOW", NULL, setupDlgProc)) return true;
    return false;

}

void msgBox (const char * head, const char * msg) {
	MessageBox (NULL, msg, head, MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL | MB_SETFOREGROUND);
}

int msgBoxQuestion (const char * head, const char * msg) {
	if (MessageBox (NULL, msg, head, MB_YESNO | MB_SETFOREGROUND | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
		return false;
	return true;
}

void changeToUserDir () {
	TCHAR szAppData[MAX_PATH];
	/*hr = */SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szAppData);
	_chdir(szAppData);
}

uint32_t launch(char * f) {
	return (uint32_t) ShellExecute (hMainWindow, "open", f, NULL, "C:\\", SW_SHOWNORMAL);
}
#endif
