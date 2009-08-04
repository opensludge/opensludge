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

char * getFileFromList (char **fileList, int index) {
	return fileList[index];
}

void clearFileList(char **resourceList, int *numResources)
{
	int i = 0;
	while (i<*numResources) {
		delete resourceList[i];
		resourceList[i] = NULL;
		i++;
	}
	*numResources = 0;
}

void addFileToList (char * file, char **resourceList, int *numResources)
{
	for (int i = 0; i<*numResources; i++) {
		if (strcmp (file, resourceList[i]) == 0)
			return;
	}
	resourceList[*numResources] = new char [strlen (file)+1];
	if (! resourceList[*numResources]) return;
	strcpy (resourceList[*numResources], file);
	(*numResources)++;
}

void removeFileFromList (int index, char **resourceList, int *numResources)
{
	if (index>=*numResources) return;
	delete resourceList[index];
	int i = index + 1;
	while (i < *numResources) {
		resourceList[i-1] = resourceList[i];
		i++;
	}
	(*numResources)--;
}

char * getFullPath (const char * file) {
#ifdef _WIN32
	return joinStrings (sourceDirectory, "\\", file);
#else
	return joinStrings (sourceDirectory, "/", file);
#endif
}

void deleteString(char * s) {
	delete s;
}


bool loadProject (const char * filename, char **fileList, int *numFiles) {
	char * readLine;
	
	FILE * fp = fopen (filename, "rt");
	if (! fp) return false;
	
	readSettings (fp);

	
	for (;;) {
		readLine = readText (fp);
		if (readLine == NULL) break;
		if (strcmp (readLine, "[FILES]") == 0) break;
		delete readLine;
	}
	
	if (readLine) delete readLine;
	
	clearFileList(fileList, numFiles);
	for (;;) {
		readLine = readText (fp);
		if (readLine == NULL) break;
		fixPath (readLine, true);
		addFileToList (readLine, fileList, numFiles);
		delete readLine;
	}
	
	fclose (fp);
	return true;
}

bool saveProject (const char * filename, char **fileList, int *numFiles) {
	FILE * fp = fopen (filename, "wt");
	if (! fp) {
		errorBox ("Can't write to project file", filename);
		return false;
	}
	
	writeSettings (fp);
	
	// Now write out the list of files...
	int i = 0;
	while (i<*numFiles) {
		fixPath (fileList[i], false);
		fprintf (fp, "%s\n", fileList[i]);
		fixPath (fileList[i], true);
		i++;
	}
	
	fclose (fp);
	
	return true;
}

void closeProject (char **fileList, int *numFiles) {
	clearFileList(fileList, numFiles);
}

void doNewProject (const char * filename, char **fileList, int *numFiles) {
	noSettings ();
	clearFileList(fileList, numFiles);
	if (! saveProject (filename, fileList, numFiles)) closeProject (fileList, numFiles);
}


void addFileToProject (const char * wholeName, char * path, char **fileList, int *numFiles) {
	int a = 0;
	char * newName, * temp;
#ifdef _WIN32
	char sep = '\\';
#else
	char sep = '/';
#endif
	while (wholeName[a] == path[a]) {
		a ++;
	}
	
	if (! path[a] && wholeName[a] == sep) {
		newName = joinStrings ("", wholeName + a + 1);
	} else {
		for (;;) {
			if (a == 0)
				break;
			if (wholeName[a-1] == sep) 
				break;
			a --;
		}
	
		newName = joinStrings ("", wholeName + a);
		a--;
		while (path[a]) {
			if (path[a] == sep) {
#ifdef _WIN32
				temp = joinStrings ("..\\", newName);
#else
				temp = joinStrings ("../", newName);
#endif
				delete newName;
				newName = temp;
			}
			a ++;
		}
	}
	addFileToList (newName, fileList, numFiles);
	delete newName;
}
