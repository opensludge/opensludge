#ifdef WIN32

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "MessBox.h"
#include "Settings.h"
#include "SPLITTER.HPP"
#include "launch.h"
#include "WINTERFA.H"

extern HMENU myMenu;

extern char loadedFile[];

/*
typeOfFile getFileType (char * filename) {
	typeOfFile reply = FILETYPE_UNKNOWN;

	char * extension = joinStrings (filename + strlen(filename) - 4, "");
	CharLower (extension);

	if (strcmp (extension, ".tra") == 0) reply = FILETYPE_TRANS;
	if (strcmp (extension, ".slu") == 0) reply = FILETYPE_SCRIPT;
	if (strcmp (extension, ".sld") == 0) reply = FILETYPE_CONST;

	delete extension;
	return reply;
}*/

char * getError (unsigned long longVal) {
	switch (longVal) {
		case 0:
		return "Can't start program; out of memory";

		case ERROR_BAD_FORMAT:
		return "Can't start program; bad .EXE file";

		case ERROR_PATH_NOT_FOUND:
		return "Can't start program; path not found";

		case ERROR_FILE_NOT_FOUND:
		return "Can't start program; file not found";
	}
	return NULL;
}

void runProg (char * prog, char * flags, char * filename) {
    getSourceDirFromName(loadedFile);
    gotoSourceDirectory();
	char * comLin = new char [strlen (prog) + strlen (flags) + strlen (filename) + 10];
	sprintf (comLin, "\"%s\" %s%s", prog, flags, filename);

	char * theError = getError (WinExec (comLin, SW_SHOW));
	if (theError) {
		errorBox (theError, comLin);
	}
	delete comLin;
}

void runFile (char * me, HWND hDlg) {
	char * theError = getError ((unsigned long) ShellExecute (hDlg, "open",
															  me, NULL, NULL,
															  SW_SHOWNORMAL));

	if (theError) {
		errorBox (theError, me);
	}
}

void editFile (char * me, HWND hDlg) {
/*	switch (getFileType (me)) {
		case FILETYPE_SCRIPT:
		case FILETYPE_CONST:
		runProg (editor, "", me);
		break;

		default:*/
		runFile (me, hDlg);
	//}
}

void gotoSite (HWND hDlg, char * url) {
	if ((unsigned long) ShellExecute (hDlg, NULL, url, NULL, "C:\\", SW_SHOWNORMAL) <= 31) {
		errorBox ("Ooh 'eck, chief!", "Can't open browser window!");
	}
}

#define NUM_OTHER_PROGRAMS	5

enum whereEnum {WHERE_KIT, WHERE_ENGINE, WHERE_TRANS};

char * lookForFolderNameHereDir[] = {
	"Software\\Hungry Software\\SLUDGE Compiler",
	"Software\\Hungry Software\\SLUDGE",
	"Software\\Hungry Software\\SLUDGE Translation Editor"
};

char * lookForFolderNameHereKey[] = {
	"compilerInstallDir",
	"engineInstallDir",
	"editorInstallDir"
};

struct fileLocation {
	int ID;
	whereEnum where;
	char * runThis;
};

struct fileLocation fileStuff[] = {
	{ID_LAUNCH_SPRITE, WHERE_KIT, "SLUDGE sprite bank editor.exe"},
	{ID_LAUNCH_Z, WHERE_KIT, "SLUDGE z-buffer maker.exe"},
	{ID_LAUNCH_TRANS, WHERE_TRANS, "SLUDGE translation editor.exe"},
	{ID_LAUNCH_FLOOR, WHERE_KIT, "SLUDGE floor maker.exe"},
	{ID_LAUNCH_ENGINE, WHERE_ENGINE, "SLUDGE engine.exe"},
};
/*
void launchOtherKitProgram (int ID, HWND h) {
	for (int i = 0; i < NUM_OTHER_PROGRAMS; i ++) {
		if (fileStuff[i].ID == ID) {
			int a = fileStuff[i].where;
			char * runFolder = getFolderFromSpecialRegistryKey (lookForFolderNameHereDir[a], lookForFolderNameHereKey[a]);
//			messageBox ("Want to run something in this folder", runFolder);
			char * wholePath = joinStrings (runFolder, "\\");
			char * wholeName = joinStrings (wholePath, fileStuff[i].runThis);
//			messageBox ("Want to run this, in fact", wholeName);
			runFile (wholeName, h);
			delete wholeName;
			delete wholePath;
			delete runFolder;
		}
	}
}

void verifyOtherKitPrograms () {
	for (int i = 0; i < NUM_OTHER_PROGRAMS; i ++) {
		int a = fileStuff[i].where;
		char * runFolder = getFolderFromSpecialRegistryKey (lookForFolderNameHereDir[a], lookForFolderNameHereKey[a]);
		if (runFolder) {
			delete runFolder;
		} else {
			EnableMenuItem (myMenu, fileStuff[i].ID, MF_GRAYED);
		}
	}
}
*/

#endif
