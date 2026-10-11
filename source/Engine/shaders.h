/*
 *  shaders.h
 *  Sludge Engine
 *
 *  Created by Rikard Peterson on 2009-12-29.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */


#include "sludge_gl.h"

char *shaderFileRead(const char *fn); 
int buildShaders (const char *vertexShader, const char *fragmentShader);

