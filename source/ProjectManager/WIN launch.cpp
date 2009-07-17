#ifdef WIN32

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "messbox.h"
#include "Splitter.hpp"
#include "launch.h"
#include "winterfa.h"
#include "registry.h"

extern char * loadedFile;
extern char * editor;
extern HMENU myMenu;

typeOfFile getFileType (char * filename) {
	typeOfFile reply = FILETYPE_UNKNOWN;
	
	char * extension = joinStrings (filename + strlen(filename) - 4, "");
	CharLower (extension);
//	errorBox ("Extension", extension);
	
	if (strcmp (extension, ".tra") == 0) reply = FILETYPE_TRANS;
	if (strcmp (extension, ".slu") == 0) reply = FILETYPE_SCRIPT;
	if (strcmp (extension, ".sld") == 0) reply = FILETYPE_CONST;
	
	delete extension;
	return reply;
}

void gotoSourceRoot () {
	char * comLin = joinStrings (loadedFile, "");
	int lastSlash = -1, a = 0;
	while (comLin[a]) {
		if (comLin[a] == '\\') lastSlash = a;
		a ++;
	}
	if (lastSlash != -1) comLin[lastSlash] = NULL;
	chdir (comLin);
	delete comLin;
}

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
	gotoSourceRoot ();
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
	switch (getFileType (me)) {
		case FILETYPE_SCRIPT:
		case FILETYPE_CONST:
		runProg (editor, "", me);
		break;
						
		default:
		runFile (me, hDlg);
	}
}

void gotoSite (HWND hDlg, char * url) {
//	errorBox ("I want to launch", url);
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

inline char * getVersion (int here) {
	return getFolderFromSpecialRegistryKey (
				lookForFolderNameHereDir[here], "currentVersion");
}

char * scripty = NULL;

#define COLUMN1A	"FFFFDD"
#define COLUMN2A	"EEFFEE"
#define COLUMN3A	"FFEEEE"

#define COLUMN1		"FFFFBB"
#define COLUMN2		"DDFFDD"
#define COLUMN3		"FFDDDD"

void addTableRow (FILE * fp, int here, char * name, char * code) {
	char * gotThisVersion = getVersion (here);

	// Print a line of the table
	fprintf (fp, "   <TR><TD BGCOLOR=#" COLUMN1A "><B>%s</B><BR>\n", name);
	fprintf (fp, "    <TD BGCOLOR=#" COLUMN2A "><B>%s</B><BR>\n", gotThisVersion ? gotThisVersion : "<I>Unknown or not installed</I>");
	fprintf (fp, "    <TD BGCOLOR=#" COLUMN3A "><BR>\n");

	// Add to scripty thing
	char * temp = joinStrings (scripty, code);
	delete scripty;
	scripty = joinStrings (temp, gotThisVersion ? gotThisVersion : "no");
	
	// Free up memory used for temporary strings
	if (gotThisVersion) delete gotThisVersion;
	delete temp;
}


char * tempHTML = NULL;
	
void checkForUpgrades (HWND h) {
	char * tempDirectory = getTempDir();
	if (! tempDirectory) return;
	
	if (tempHTML) delete tempHTML;
	tempHTML = joinStrings (tempDirectory, "\\SLUDGE online check-up.html");
	delete tempDirectory;
	
	if (! tempHTML) return;
	
	FILE * fp = fopen (tempHTML, "wt");
	if (fp) {
	
		scripty = joinStrings ("", "");
	
		fprintf (fp,

"<HTML>\n"
" <HEAD>\n"
"  <TITLE>Update your SLUDGE setup online</TITLE>\n"
" </HEAD>\n"
" <BODY BGCOLOR=#FFFFFF TEXT=#000000 LINK=#DD3333 VLINK=#992222 ALINK=#FF8877>\n"
"  <FONT FACE=Ariel>\n"
"  <H2>Update your SLUDGE setup online</H3>\n"
"  <TABLE WIDTH=100% BORDER=1 CELLPADDING=5 CELLSPACING=0>\n"
"   <TR><TD BGCOLOR=#" COLUMN1 "><BIG><B>PROGRAM NAME</B></BIG><BR>\n"
"    <TD BGCOLOR=#" COLUMN2 "><BIG><B>YOUR VERSION</B></BIG><BR>\n"
"    <TD BGCOLOR=#" COLUMN3 "><BIG><B>LATEST VERSION</B></BIG><BR>\n");

		addTableRow (fp, WHERE_KIT, "SLUDGE development kit", "devkit=");
		addTableRow (fp, WHERE_ENGINE, "SLUDGE engine", "&engine=");
		addTableRow (fp, WHERE_TRANS, "SLUDGE translation editor", "&trans=");

		fprintf (fp,

"  </TABLE><P>\n"
"  Well, that's your current set-up... now you probably want to\n"
"  <B><A HREF=\"http://www.hungrysoftware.com/tools/sludge/checkup.cgi?%s\" TARGET=_top>check for newer versions online</A></B>.<P>\n"
"  </FONT>\n"
" </BODY>\n"
"</HTML>\n", scripty);

		fclose (fp);
		runFile (tempHTML, h);
	}
}

void cleanUpFiles () {
	if (tempHTML) unlink (tempHTML);
}

#endif