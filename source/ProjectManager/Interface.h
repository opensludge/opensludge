/*
 *  Interface.h
 *  Sludge Dev Kit
 *
 *  OS Dependent functions
 *
 *  Created by Rikard Peterson on 2009-07-17.
 *  Copyright 2009 Hungry Software and contributors. All rights reserved.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

void updateFileListing();

// Update title of project window, and (de)activate menus
void updateTitle ();


const char * getTempDir ();

	
#ifdef __cplusplus
}
#endif
