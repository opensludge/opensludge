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
#include "Splitter.h"
#include "MessBox.h"
#include "Project.hpp"

char * loadedFile = NULL;
bool changed = false;


void updateTitle () {
	if (loadedFile) {
		char buff[500];
		sprintf (buff, "SLUDGE Project Manager: %s%s", changed ? "* " : "", loadedFile);
		//SetWindowText (mainWin, buff);
		//setEverything (true);
		//setWindowText (ID_LOADED_FILE, loadedFile);
	} else {
		//SetWindowText (mainWin, "SLUDGE Project Manager");
		//setEverything (false);
		//setWindowText (ID_LOADED_FILE, "No project loaded");
	}
}

void setChanged (bool newVal) {
	changed = newVal;
	updateTitle ();
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
		//messageBox ("Skipping", readLine);
		delete readLine;
	}
	
	if (readLine) delete readLine;
	
	//MESS(ID_FILELIST, LB_RESETCONTENT, 0, 0);
	for (;;) {
		readLine = readText (fp);
		if (readLine == NULL) break;
		//if (readLine[0]) MESS(ID_FILELIST, LB_ADDSTRING, 0, readLine);
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
	/*
	int i, n = MESS (ID_FILELIST, LB_GETCOUNT, 0, 0);
	
	for (i = 0; i < n; i ++) {
		tx = getFileFromBox (i);
		fprintf (fp, "%s\n", tx);
		delete tx;
	}*/
	
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
	//MESS(ID_FILELIST, LB_RESETCONTENT, 0, 0);
	updateTitle ();
}

void doNewProject (char * filename) {
	noSettings ();
	//MESS(ID_FILELIST, LB_RESETCONTENT, 0, 0);
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
	//MESS(ID_FILELIST, LB_ADDSTRING, 0, newName);
	//MESS(ID_FILELIST, LB_SELECTSTRING, 0, newName);
	delete newName;
}