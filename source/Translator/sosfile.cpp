#if 0
TODO

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "sosfile.h"
#include "moreio.h"
#include "splitter.hpp"
#include "wintext.h"
#include "winterfa.h"
#include "wincomp.h"


extern HWND				mainWin;
extern HINSTANCE		inst;
extern HMENU			myMenu;

transLine * firstTransLine = NULL;
transLine * selectedTransLine;

bool beenChanged = false, fileBeenChanged = false;
extern char * loader;
extern char * searchString;

#define enableMenu(a,b)		EnableMenuItem (myMenu, a, b ? MF_ENABLED : MF_GRAYED)

void fixMenus (bool f) {
	fileBeenChanged = f;
	enableMenu (ID_PROJECT_REVERT, fileBeenChanged && loader[0]);	
	enableMenu (ID_PROJECT_SAVE,   fileBeenChanged && loader[0]);	
	enableMenu (ID_PROJECT_SAVEAS, fileBeenChanged || loader[0]);
}

bool trashProgress (char * doWhat) {
	if (fileBeenChanged) {
		
		char * question = joinStrings ("Are you sure you want to ", doWhat, "? You'll lose all your unsaved data...");
		bool answer = ask (question);
		delete question;
		return answer;
	} else {
		return true;
	}
}

void setFileName (char * fn, unsigned int languageID) {
	if (loader) delete loader;

	if (fn) {
		loader = copyString (fn);
		int a = 0, sta = 0;
		while (fn[a]) {
			if (fn[a] == '\\') sta = a + 1;
			a++;
		}
		SendDlgItemMessage (mainWin, ID_FILENAME, WM_SETTEXT, (WPARAM) 0, (LPARAM) (fn + sta));
	} else {
		loader = copyString ("");
		SendDlgItemMessage (mainWin, ID_FILENAME, WM_SETTEXT, (WPARAM) 0, (LPARAM) "Untitled");
	}
	SetDlgItemInt (mainWin, ID_ID, languageID, false);
	fixMenus (false);
}


unsigned int stringToInt (const char * textNumber) {
	unsigned long i = 0;
	int ac = 0;

	while (textNumber[ac]) {
		if (textNumber[ac] >= '0' && textNumber[ac] <= '9') {
			i = (i * 10) + textNumber[ac] - '0';
			if (i >= 0xFFFF) {
				errorBox ("Warning - ID too large", textNumber);
				return -1;
			}
		} else {
			errorBox ("Not a valid ID", textNumber);
			return -1;
		}
		ac ++;			
	}
	
	return (unsigned int) i;
}



void enableTranslationBox (int level, char * s) {
	if (s) SendDlgItemMessage (mainWin, ID_ORIGINAL_TEXT, WM_SETTEXT, (WPARAM) 0, (LPARAM) s);
	SendDlgItemMessage (mainWin, ID_ENTER_TRANSLATION, WM_SETTEXT, (WPARAM) 0, (LPARAM) "");
	CheckDlgButton (mainWin, ID_GOT_TRANSLATION, level ? BST_UNCHECKED : BST_CHECKED);
	EnableWindow (GetDlgItem (mainWin, ID_GOT_TRANSLATION), level < 10);
	EnableWindow (GetDlgItem (mainWin, ID_ENTER_TRANSLATION), level < 5);
}

void setChanged (bool bc) {
	beenChanged = bc;
	EnableWindow (GetDlgItem (mainWin, IDOK), bc);
	EnableWindow (GetDlgItem (mainWin, ID_UNDO), bc);
}

void selectedString (char * s) {
	selectedTransLine = firstTransLine;
	while (selectedTransLine) {
		if (strcmp (s, selectedTransLine -> transFrom) == 0) break;
		selectedTransLine = selectedTransLine -> next;
	}
	if (selectedTransLine) {	// Just in case things go wrong...
		switch (selectedTransLine -> type) {
			case TYPE_TRANS:
			enableTranslationBox (0, s);
			SendDlgItemMessage (mainWin, ID_ENTER_TRANSLATION, WM_SETTEXT, (WPARAM) 0, (LPARAM) selectedTransLine -> transTo);
			break;
			
			case TYPE_NONE:
			enableTranslationBox (5, s);
			break;
			
			default:	// New etc.
			enableTranslationBox (0, s);
			SendDlgItemMessage (mainWin, ID_ENTER_TRANSLATION, WM_SETTEXT, (WPARAM) 0, (LPARAM) "Please enter a translation here, or uncheck the box above...");
			break;
		}
	}	
}

void commitChanges () {
	if (selectedTransLine) {
		delete selectedTransLine -> transTo;
		if (IsDlgButtonChecked (mainWin, ID_GOT_TRANSLATION) == BST_CHECKED) {
			int sizeOfString = 1 + MESS (ID_ENTER_TRANSLATION, WM_GETTEXTLENGTH, 0, 0);
			selectedTransLine -> transTo = new char[sizeOfString];
			MESS (ID_ENTER_TRANSLATION, WM_GETTEXT, sizeOfString, selectedTransLine -> transTo);
			selectedTransLine -> type = TYPE_TRANS;
		} else {
			selectedTransLine -> transTo = NULL;
			selectedTransLine -> type = TYPE_NONE;			
		}
	}
	updateStringList ();
	fixMenus (true);
}

char * mystrstr (const char * biggy, const char * littley) {
	char * bigUp = copyString (biggy);
	CharUpper (bigUp);
	char * reply = strstr (bigUp, littley);
//	if (reply) messageBox ("Found in", bigUp);
	delete bigUp;
	return reply;
}

bool doesSearchStringMatchLine (transLine * thisLine) {
	char * (* callMe) (const char *, const char *);
	callMe = programSettings.searchSensitive ? strstr : mystrstr;
	if (searchString) {
		if (callMe (thisLine -> transFrom, searchString))
			return true;

		if (thisLine -> transTo && callMe (thisLine -> transTo, searchString))
			return true;
	}
	return false;
}

void updateStringList () {
	int showtype = MESS (ID_CATEGORY, CB_GETCURSEL, 0, 0);

	MESS (ID_STRINGS, LB_RESETCONTENT, 0, 0);
	
	transLine * eachLine = firstTransLine;
	
	while (eachLine) {
		bool addMe = false;

		if (showtype == TYPE_SEARCH) {
			addMe = doesSearchStringMatchLine (eachLine);
		} else {
			addMe = eachLine -> type == showtype;
		}
			
		if (addMe)
			MESS (ID_STRINGS, LB_ADDSTRING, 0, eachLine -> transFrom);
			
		eachLine = eachLine -> next;
	}
	
	if (MESS (ID_STRINGS, LB_GETCOUNT, 0, 0)) {
		EnableWindow (GetDlgItem (mainWin, ID_STRINGS), true);
	} else {
		MESS (ID_STRINGS, LB_ADDSTRING, 0, "(Empty)");
		EnableWindow (GetDlgItem (mainWin, ID_STRINGS), false);
	}
	selectedTransLine = NULL;
	enableTranslationBox (10, "");
	setChanged (false);
}

void autoSelectContent (lineType type) {
	MESS (ID_CATEGORY, CB_SETCURSEL, type, 0);
	updateStringList ();
}



#endif
