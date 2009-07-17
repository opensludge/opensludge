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
	
void clearFileList();
void addFileToList();
extern int fileListNum;
char * getFileFromList (int index);	
	
void setChanged (bool newVal);

void loadProject (const char * filename);
bool saveProject (const char * filename);
void closeProject ();
void doNewProject (const char * filename);

void addFileToProject (char * wholeName);


#ifdef __cplusplus
}
#endif
