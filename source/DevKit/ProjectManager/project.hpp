/*
 *  Project.h
 *  Sludge Dev Kit
 *
 *  Created by Rikard Peterson on 2009-07-15.
 *  Copyright 2009 Hungry Software and contributors. All rights reserved.
 *
 */
#ifdef __cplusplus
extern "C" {
#endif

enum parseMode {PM_NORMAL, PM_QUOTE, PM_COMMENT, PM_FILENAME};
	
void clearFileList(char **fileList, int *numFiles);
void addFileToList (char * file, char **fileList, int *numFiles);
void removeFileFromList (int index, char **fileList, int *numFiles);

void populateResourceList (const char * scriptName, char **resourceList, int *numResources);
int isResource (const char * scriptName, char *resource);

bool loadProject (const char * filename, char **fileList, int *numFiles);
bool saveProject (const char * filename, char **fileList, int *numFiles);
void closeProject (char **fileList, int *numFiles);
void doNewProject (const char * filename, char **fileList, int *numFiles);

void addFileToProject (const char * wholeName, char * path, char **fileList, int *numFiles);

char * getFullPath (const char * file);

#ifdef __cplusplus
}
#endif
