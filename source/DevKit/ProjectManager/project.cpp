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

#include "settings.h"
#include "moreio.h"
#include "helpers.h"
#include "splitter.hpp"
#include "messbox.h"
#include "project.hpp"
#include "interface.h"

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

void removeFileFromList (int index, char **fileList, int *numFiles)
{
	if (index>=*numFiles) return;
	delete fileList[index];
	int i = index + 1;
	while (i < *numFiles) {
		fileList[i-1] = fileList[i];
		i++;
	}
	(*numFiles)--;
}

int isResource (const char * scriptName, char *resource) {
	FILE * fp;
	char t, lastOne;
	enum parseMode pM;
	char buffer[256];
	int numBuff;
	
	gotoSourceDirectory ();
	
	const char * extension = scriptName + strlen(scriptName) - 4;	
	if (strlen (scriptName) > 4 && strcmp (extension, ".slu") == 0) {
		fp = fopen (scriptName, "rt");
		if (fp) {
			pM = PM_NORMAL;
			t = ' ';
			for (;;) {
				lastOne = t;
				t = fgetc (fp);
				if (feof (fp)) break;
				switch (pM) {
					case PM_NORMAL:
						if (t == '\'') {
							pM = PM_FILENAME;
							numBuff = 0;
						}
						if (t == '\"') pM = PM_QUOTE;
						if (t == '#') pM = PM_COMMENT;
						break;
						
					case PM_COMMENT:
						if (t == '\n') pM = PM_NORMAL;
						break;
						
					case PM_QUOTE:
						if (t == '\"' && lastOne != '\\') pM = PM_NORMAL;
						break;
						
					case PM_FILENAME:
						if (t == '\'' && lastOne != '\\') {
							buffer[numBuff] = 0;
							pM = PM_NORMAL;
							if (strcmp(buffer, resource)) return true;
						} else {
							buffer[numBuff++] = t;
							if (numBuff == 250) {
								buffer[numBuff++] = 0;
								errorBox ("Resource filename too long!", buffer);
								numBuff = 0;
							}
						}
						break;
				}
			}
			fclose (fp);
		} else {
			errorBox ("Can't open script file to look for resources", scriptName);
		}
	}
	return false;
}

void populateResourceList (const char * scriptName, char **resourceList, int *numResources)
{
	FILE * fp;
	char t, lastOne;
	enum parseMode pM;
	char buffer[256];
	int numBuff;

	gotoSourceDirectory ();

	const char * extension = scriptName + strlen(scriptName) - 4;	
	if (strlen (scriptName) > 4 && strcmp (extension, ".slu") == 0) {
		fp = fopen (scriptName, "rt");
		if (fp) {
			pM = PM_NORMAL;
			t = ' ';
			for (;;) {
				lastOne = t;
				t = fgetc (fp);
				if (feof (fp)) break;
				switch (pM) {
					case PM_NORMAL:
						if (t == '\'') {
							pM = PM_FILENAME;
							numBuff = 0;
						}
						if (t == '\"') pM = PM_QUOTE;
						if (t == '#') pM = PM_COMMENT;
							break;
							
					case PM_COMMENT:
						if (t == '\n') pM = PM_NORMAL;
						break;
							
					case PM_QUOTE:
						if (t == '\"' && lastOne != '\\') pM = PM_NORMAL;
						break;
							
					case PM_FILENAME:
						if (t == '\'' && lastOne != '\\') {
							buffer[numBuff] = 0;
							pM = PM_NORMAL;
							addFileToList (buffer, resourceList, numResources);
						} else {
							buffer[numBuff++] = t;
							if (numBuff == 250) {
								buffer[numBuff++] = 0;
								errorBox ("Resource filename too long!", buffer);
								numBuff = 0;
							}
						}
						break;
				}
			}
			fclose (fp);
		} else {
			errorBox ("Can't open script file to look for resources", scriptName);
		}
	}
}

char * getFullPath (const char * file) {
	return joinStrings (sourceDirectory, "/", file);
}

bool loadProject (const char * filename, char **fileList, int *numFiles) {
	char * readLine;

	FILE * fp = fopen (filename, "rb");
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

	while (wholeName[a] == path[a]) {
		a ++;
	}

	if (! path[a] && wholeName[a] == '/') {
		newName = joinStrings ("", wholeName + a + 1);
	} else if (! path[a] && path[a-1] == '/' && wholeName[a-1] == '/') {
		newName = joinStrings ("", wholeName + a);
	} else {
		for (;;) {
			if (a == 0)
				break;
			if (wholeName[a-1] == '/')
				break;
			a --;
		}

		newName = joinStrings ("", wholeName + a);
		a--;
		while (path[a]) {
			if (path[a] == '/') {
				temp = joinStrings ("../", newName);
				delete newName;
				newName = temp;
			}
			a ++;
		}
	}
	addFileToList (newName, fileList, numFiles);
	delete newName;
}
