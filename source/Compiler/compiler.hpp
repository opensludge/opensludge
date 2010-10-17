/*
 *  Compiler.hpp
 *  Sludge Dev Kit
 *
 *  Created by Rikard Peterson on 2009-07-16.
 *  Copyright 2009 Hungry Software and contributors. All rights reserved.
 *
 */

#include "compilerinfo.h"

#ifdef __cplusplus
extern "C" {
#endif
		
	int compileEverything (char * project, char **fileList, int *numFiles, void (*infoReceiver)(compilerInfo *));
	
	
#ifdef __cplusplus
}
#endif
