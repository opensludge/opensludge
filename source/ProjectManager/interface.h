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

const char * getTempDir ();
bool askAQuestion (const char * head, const char * msg);
bool errorBox (const char * head, const char * msg);

#ifdef __cplusplus
}
#endif
