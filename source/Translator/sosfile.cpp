#if 0
TODO

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "sosfile.h"
#include "moreio.h"
#include "messbox.h"
#include "splitter.h"
#include "wintext.h"
#include "winterfa.h"
#include "wincomp.h"

#define HEADERLINE	"### SLUDGE Translation File ###"

extern HWND				mainWin;
extern HINSTANCE		inst;
extern HMENU			myMenu;

transLine * firstTransLine = NULL;
transLine * selectedTransLine;

bool beenChanged = false, fileBeenChanged = false;
extern char * loader;
extern char * searchString;
extern bool searchSensitive;

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

void newFile () {
	while (firstTransLine) {
		selectedTransLine = firstTransLine;
		firstTransLine = firstTransLine -> next;
		
		delete selectedTransLine -> transFrom;
		delete selectedTransLine -> transTo;
		delete selectedTransLine;
	}
	autoSelectContent (TYPE_TRANS);
	setFileName (NULL, 0);
	delete searchString;
	searchString = false;
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

transLine * handleLine (char * line, transLine * lastSoFar) {
	stringArray * pair = splitString (line, '\t', ONCE);

	transLine * nt = new transLine;
	trimEdgeSpace (pair->string);
	nt -> transFrom = copyString (pair->string);
	destroyFirst (pair);
	if (pair) {
		trimEdgeSpace (pair->string);
		if (strcmp (pair -> string, "*\t")) {
			nt -> transTo = copyString (pair -> string);
			nt -> type = TYPE_TRANS;
		} else {
			nt -> transTo = NULL;
			nt -> type = TYPE_NEW;
		}
		destroyFirst (pair);
	} else {
		nt -> transTo = NULL;
		nt -> type = TYPE_NONE;
	}

	if (lastSoFar) {
		lastSoFar -> next = nt;
	} else {
		firstTransLine = nt;
	}

	nt -> next = NULL;		
	return nt;
}

enum theMode {MODE_ID, MODE_STRINGS, MODE_UNKNOWN};

void loadFile (char * fileIn) {
	char * file = copyString (fileIn);
	char * error = NULL;
	transLine * lastSoFar = NULL;
	unsigned int lanID = 0;
	newFile ();
	
	FILE * fp = fopen (file, "rt");
	if (fp == NULL) {
		error = "Can't open file for reading";
	} else {
		char * line = readText (fp);
		if (line == NULL) {
			error = "File is empty";
		} else if (strcmp (line, HEADERLINE)) {
			error = "Not a SLUDGE translation file (first line isn't right)";
		} else {
			theMode mode = MODE_UNKNOWN;
			for (;;) {
				delete line;
				line = readText (fp);
				if (line == NULL) break;
				switch (line[0]) {
					case NULL:
					break;
				
					case '[':
					if (strcmp (line, "[ID]") == 0)
						mode = MODE_ID;
					else if (strcmp (line, "[DATA]") == 0)
						mode = MODE_STRINGS;
					else
						mode = MODE_UNKNOWN;
					break;

					default:
					switch (mode) {
						case MODE_ID:
						lanID = stringToInt (line);
						break;
						
						case MODE_STRINGS:
						lastSoFar = handleLine (line, lastSoFar);
						break;
					}
				}
			}
		}
		delete line;
	}
	fclose (fp);
	if (error) {
		errorBox (error, file);
	} else {
		autoSelectContent (TYPE_TRANS);
		setFileName (file, lanID);
	}
	delete file;
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
	callMe = searchSensitive ? strstr : mystrstr;
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

int foundStringInFileDel (char * string) {
	transLine * hunt = firstTransLine;
	transLine * lastOne = NULL;
	
	trimEdgeSpace (string);
	
	while (hunt) {
		if (strcmp (hunt -> transFrom, string) == 0) return 0;
		lastOne = hunt;
		hunt = hunt -> next;
	}
//	errorBox ("Found NEW string", string);
	hunt = handleLine(string, lastOne);
	hunt->type = TYPE_NEW;
	delete string;
	return 1;
}

int foundStringInFileEscaped (char * string) {
	stringArray * bits = splitString (string, '\t', REPEAT);
	char * rebuilt = copyString ("");
	while (bits) {
		char * temp = joinStrings (rebuilt, bits->string);
		delete rebuilt;
		rebuilt = temp;
		destroyFirst (bits);
	}
//	if (strcmp (string, rebuilt)) errorBox (string, rebuilt);
	return foundStringInFileDel(rebuilt);
}

int updateFromSource (char * filename) {
	int len = strlen (filename);
	if (len < 4) return 0;
	char * last4 = filename + len - 4;
	last4[1] = toupper (last4[1]);
	last4[2] = toupper (last4[2]);
	last4[3] = toupper (last4[3]);
	if (strcmp (last4, ".SLU")) return 0;
	
	FILE * source = fopen (filename, "rt");
	if (source == NULL) {
		errorBox ("Can't open source file for reading", filename);
		return 0;
	}
	
	int numChanges = 0;
	
	for (;;) {
		char * wholeLine = readText(source);
		if (wholeLine == NULL) break;
		for (int a = 0; wholeLine[a]; a ++) {
			if (wholeLine[a] == '#') break;	// Comment? Skip it!
			if (wholeLine[a] == '\"') {
				while (wholeLine[a+1] == ' ') a ++;	// No spaces at start, please
				bool escape = false;
				for (int b = a+1; wholeLine[b]; b ++) {
					if (wholeLine[b] == '\\') {
						if (! escape) wholeLine[b] = '\t';		// So we can split the string up on tab later
						escape = ! escape;
					} else if (wholeLine[b] == '\"') {
						if (! escape) {
							if (b != a + 1) {
								wholeLine[b] = NULL;
								numChanges += foundStringInFileEscaped (wholeLine + a + 1);
								wholeLine[b] = '\"';
							}
							a = b;
							break;
						}
						escape = false;
					} else {
						escape = false;
					}
				}
			}
		}
		delete wholeLine;
	}
	
	fclose (source);
	return numChanges;
}

void updateFromProject (char * filename) {
	FILE * fp = fopen (filename, "rt");
	int totalNew = 0;
	if (fp) {
		char * theLine;
		for (;;) {
			theLine = readText (fp);
			if (theLine == NULL) break;
			if (strcmp (theLine, "[FILES]") == 0) break;
			stringArray * bits = splitString (theLine, '=', ONCE);
			if (bits -> next != NULL) {
				if (strcmp (bits -> string, "windowname") == 0 ||
					strcmp (bits -> string, "quitmessage") == 0) {
					totalNew += foundStringInFileDel (copyString (bits -> next -> string));
				}
			}
			delete theLine;
		}
		if (theLine == NULL) {
			fclose (fp);
			errorBox ("Not a SLUDGE project file", filename);
		}
		while (theLine) {
			delete theLine;
			theLine = readText (fp);
			if (theLine) totalNew += updateFromSource (theLine);
		}
	}
	if (totalNew) {
		autoSelectContent (TYPE_NEW);
		fixMenus (true);
	} else {
		errorBox ("Found no new strings in the project that I don't already know about. This translation file is up to date! Hooray!\n\nProject file scanned", filename);
	}
}

void saveToFile (char * filename, unsigned int lan) {

	FILE * fp = fopen (filename, "wt");

	if (! fp) {
		errorBox ("Couldn't write to file", filename);
		return;
	}
	
	fprintf (fp, HEADERLINE"\n\n[ID]\n%i\n\n[DATA]\n", lan);
	transLine * eachLine = firstTransLine;
	
	while (eachLine) {
		switch (eachLine -> type) {
			case TYPE_NEW:
			fprintf (fp, "%s\t*\t\n", eachLine -> transFrom);
			break;

			case TYPE_TRANS:
			fprintf (fp, "%s\t%s\n", eachLine -> transFrom, eachLine -> transTo);
			break;
			
			default:
			fprintf (fp, "%s\n", eachLine -> transFrom);
			break;
		}
		eachLine = eachLine -> next;
	}
	
	fclose (fp);	
	setFileName (filename, lan);
}

#endif