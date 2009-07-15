/*
 *  Project.h
 *  Sludge Dev Kit
 *
 *  Created by Rikard Peterson on 2009-07-15.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#ifdef __cplusplus
extern "C" {
#endif
	
void updateTitle ();
void setChanged (bool newVal);

void loadProject (char * filename);
bool saveProject (char * filename);
void closeProject ();
void doNewProject (char * filename);

void addFileToProject (char * wholeName);

#ifdef __cplusplus
}
#endif
