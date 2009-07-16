/*
 *  Project.c
 *  Sludge Dev Kit
 *
 *  Created by Rikard Peterson on 2009-07-15.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Settings.h"
#include "MoreIO.h"
#include "Splitter.hpp"
#include "MessBox.h"
#include "Project.hpp"

char * loadedFile = NULL;
bool changed = false;

extern "C" void updateFileListing();
extern "C" void activateMenus (bool);

char *fileList[1000];
int fileListNum = 0;

void clearFileList() {
	int i = 0;
	while (i<fileListNum) {
		delete fileList[i];
		fileList[i] = NULL;
		i++;
	}
	fileListNum = 0;
	updateFileListing();
}

void addFileToList (char * file) {
	fileList[fileListNum] = new char [strlen (file)+1];
	if (! fileList[fileListNum]) return;
	strcpy (fileList[fileListNum], file);
	fileListNum++;
	updateFileListing();
}

char * getFileFromList (int index) {
	return fileList[index];
}

void updateTitle () {
	if (loadedFile) {
		char buff[500];
		sprintf (buff, "SLUDGE Project Manager: %s%s", changed ? "* " : "", loadedFile);
		//SetWindowText (mainWin, buff);
		activateMenus (true);
		//setWindowText (ID_LOADED_FILE, loadedFile);
	} else {
		//SetWindowText (mainWin, "SLUDGE Project Manager");
		activateMenus (false);
		//setWindowText (ID_LOADED_FILE, "No project loaded");
	}
}

void setChanged (bool newVal) {
	changed = newVal;
	updateTitle ();
}

void loadProject (const char * filename) {
	char * readLine;

	FILE * fp = fopen (filename, "rt");
	if (! fp) return;
	
	readSettings (fp);

	
	for (;;) {
		readLine = readText (fp);
		if (readLine == NULL) break;
		if (strcmp (readLine, "[FILES]") == 0) break;
		//messageBox ("Skipping", readLine);
		delete readLine;
	}
	
	if (readLine) delete readLine;
	
	clearFileList();
	for (;;) {
		readLine = readText (fp);
		if (readLine == NULL) break;
		fixPath (readLine, true);
		addFileToList (readLine);
		delete readLine;
	}
	
	fclose (fp);
	if (filename != loadedFile) {
		delete loadedFile;
		loadedFile = joinStrings (filename, "");
	}
	setChanged (false);
}

bool saveProject (const char * filename) {
	if (filename != loadedFile) {
		if (filename) {
			delete loadedFile;
			loadedFile = joinStrings (filename, "");
		}
	}
	FILE * fp = fopen (loadedFile, "wt");
	if (! fp) {
		errorBox ("Can't write to project file", loadedFile);
		return false;
	}
	
	writeSettings (fp);
	
	// Now write out the list of files...
	int i = 0;
	while (i<fileListNum) {
		fixPath (fileList[i], false);
		fprintf (fp, "%s\n", fileList[i]);
		fixPath (fileList[i], true);
		i++;
	}
	
	fclose (fp);
	
	setChanged (false);
	return true;
}

void closeProject () {
	blankSettings ();
	delete loadedFile;
	loadedFile = NULL;
	clearFileList();
	updateTitle ();
}

void doNewProject (const char * filename) {
	noSettings ();
	clearFileList();
	if (! saveProject (filename)) closeProject ();
}

void addFileToProject (char * wholeName) {
	int a = 0;
	char * newName, * temp;
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
		}
		a ++;
	}
	addFileToList (newName);
	delete newName;
}


