/*
 *  Project.c
 *  Sludge Dev Kit
 *
 *  Created by Rikard Peterson on 2009-07-15.
 *  Copyright 2009 Hungry Software and contributors. All rights reserved.
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
#include "Interface.h"

bool changed = false;

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

// Feed this with relative paths for best cross-platform (and cross-computer!) results.
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


void setChanged (bool newVal) {
	changed = newVal;
	updateTitle ();
}

bool loadProject (const char * filename) {
	char * readLine;
	
	FILE * fp = fopen (filename, "rt");
	if (! fp) return false;
	
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
	setChanged (false);
	return true;
}

bool saveProject (const char * filename) {
	FILE * fp = fopen (filename, "wt");
	if (! fp) {
		errorBox ("Can't write to project file", filename);
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
	clearFileList();
	updateTitle ();
}

void doNewProject (const char * filename) {
	noSettings ();
	clearFileList();
	if (! saveProject (filename)) closeProject ();
}


void addFileToProject (char * wholeName, char * path) {
	int a = 0;
	char * newName, * temp;
	while (wholeName[a] == path[a]) {
		a ++;
	}
	
	for (;;) {
		if (a == 0) break;
		if (wholeName[a - 1] == '\\') break;
		a --;
	}
	
	newName = joinStrings ("", wholeName + a);
	while (path[a]) {
		if (path[a] == '\\') {
			temp = joinStrings ("..\\", newName);
			delete newName;
			newName = temp;
		}
		a ++;
	}
	addFileToList (newName);
	delete newName;
}
