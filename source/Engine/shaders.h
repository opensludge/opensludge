/*
 *  shaders.h
 *  Sludge Engine
 *
 *  Created by Rikard Peterson on 2009-12-29.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */


#include "GLee.h"

char *shaderFileRead(const char *fn); 
int buildShaders (const GLchar *vertexShader, const GLchar *fragmentShader);

int printOglError (const char *file, int         line);
#define printOpenGLError() printOglError(__FILE__, __LINE__)

