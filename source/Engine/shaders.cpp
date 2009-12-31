/*
 *  shaders.cpp
 *  Sludge Engine
 *
 *  Created by Rikard Peterson on 2009-12-29.
 *
 */

#include <stdio.h>

#include "shaders.h"


int buildShaders (const GLchar *vertexShader, const GLchar *fragmentShader) 
{
	GLuint VS, FS, prog;
	GLint vertCompiled, fragCompiled;
	GLint linked;
	
	// Create Shader Objects
	VS = glCreateShader(GL_VERTEX_SHADER);
	FS = glCreateShader(GL_FRAGMENT_SHADER);
	
	// Load source code strings into shaders
	glShaderSource(VS, 1, &vertexShader, NULL);
	glShaderSource(FS, 1, &fragmentShader, NULL);
	
	// Compile vertex shader and print log
	glCompileShader(VS);
	//printOpenGLError();
	glGetShaderiv(VS, GL_COMPILE_STATUS, &vertCompiled);
	//printShaderInfoLog (VS);
	
	// Compile fragment shader and print log
	glCompileShader(FS);
	//printOpenGLError();
	glGetShaderiv(FS, GL_COMPILE_STATUS, &fragCompiled);
	//printShaderInfoLog (FS);
	
	if (!vertCompiled || !fragCompiled)
		return 0;
	
	fprintf (stderr, "Shaders compiled. \n");
	
	
	// Create a program object and attach the two compiled shaders
	prog = glCreateProgram();
	glAttachShader(prog, VS);
	glAttachShader(prog, FS);
	
	// Clean up
	glDeleteShader (VS);
	glDeleteShader (FS);
	
	// Link the program and print log
	glLinkProgram(prog);
	//printOpenGLError();
	glGetProgramiv(prog, GL_LINK_STATUS, &linked);
	//printProgramInfoLog(prog);
	
	if (!linked) 
		return 0;
	
	fprintf (stderr, "Shader program linked. \n");
		
	return prog;
}


