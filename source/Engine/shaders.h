/*
 *  shaders.h
 *  Sludge Engine
 *
 *  Created by Rikard Peterson on 2009-12-29.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */


#if !defined(HAVE_GLES2)
#include <GL/glew.h>
#else
#include <GLES2/gl2.h>
#endif

char *shaderFileRead(const char *fn); 
int buildShaders (const char *vertexShader, const char *fragmentShader);

