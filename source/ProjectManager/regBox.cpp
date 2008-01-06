#if 0
#include <windows.h>
#include "winterfa.h"
#include "messbox.h"
#include "registry.h"

#define all32Chars "B7C2DFG8HJKT9V6WX01YLM4NA5PQR3SZ"

BOOL hacker = FALSE;

void gotoSite (HWND, char *);

void checkForHackers (char * name, char * codeIn) {
	unsigned int i, codeSoFar = 1793574635, exOrVal = 3192837465;
	for (i = 0; name[i]; i ++) {
		codeSoFar += name[i] ^ exOrVal;
		exOrVal <<= 1;
		exOrVal += name[i] & 1;
	}
	char requiredCode[9];
	for (i = 0; i < 8; i ++) {
		if (i == 3) {
			requiredCode[i] = '-';
		} else {
			requiredCode[i] = all32Chars[codeSoFar & 31];
			codeSoFar >>= 5;
		}
	}
	requiredCode[8] = NULL;
	hacker |= (strcmp (requiredCode, codeIn) == 0);
}

LRESULT CALLBACK regBoxFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_INITDIALOG:
		{
			char * t;
			t = getRegString ("regName");
		    SetWindowText (GetDlgItem (hDlg, ID_REG_NAME), t);
			delete t;
			t = getRegString ("regCode");
		    SetWindowText (GetDlgItem (hDlg, ID_REG_CODE), t);
			delete t;
		}
		break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case ID_REG_GET_CODE:
				gotoSite (hDlg, "http://www.hungrysoftware.com/store/");
				break;

				case IDOK:
				{
					int le = GetWindowTextLength (GetDlgItem (hDlg, ID_REG_NAME)) + 1;
					char * newName = new char[le];
					if (newName) GetWindowText (GetDlgItem (hDlg, ID_REG_NAME), newName, le);

					le = GetWindowTextLength (GetDlgItem (hDlg, ID_REG_CODE)) + 1;
					char * newCode = new char[le];
					if (newCode) GetWindowText (GetDlgItem (hDlg, ID_REG_CODE), newCode, le);
					
					if (newName && newCode) {
						setRegString ("regName", newName);
						setRegString ("regCode", newCode);
						checkForHackers (newName, newCode);
					}
				}

				case IDCANCEL:
				EndDialog(hDlg, TRUE);
				return (TRUE);
			}
			break;
	}

    return FALSE;
}
#endif