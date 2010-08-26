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

enum  compilerStatusText{
	COMPILER_TXT_ACTION,
	COMPILER_TXT_FILENAME,
	COMPILER_TXT_ITEM
} ;

void setCompilerText (const compilerStatusText, const char * theText);
void setCompilerStats (int funcs, int objTypes, int files, int globals, int strings);

const char * getTempDir ();


#ifdef __cplusplus
}
#endif
