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
	
void clearFileList(char **fileList, int *numFiles);
void addFileToList (char * file, char **fileList, int *numFiles);
void removeFileFromList (int index, char **resourceList, int *numResources);

bool loadProject (const char * filename, char **fileList, int *numFiles);
bool saveProject (const char * filename, char **fileList, int *numFiles);
void closeProject (char **fileList, int *numFiles);
void doNewProject (const char * filename, char **fileList, int *numFiles);

void addFileToProject (const char * wholeName, char * path, char **fileList, int *numFiles);

char * getFullPath (const char * file);
void deleteString(void *);

#ifdef __cplusplus
}
#endif
