/*
 *  shaders.cpp
 *  Sludge Engine
 *
 *  Created by Rikard Peterson on 2009-12-29.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "stringy.h"
#include "shaders.h"
#include "graphics.h"

#include "shaders_embedded.h"

// Return the source of one of the shaders, which we build into the
// executable, as a string allocated with malloc.
char *shaderFileRead(const char *name)
{
	for (const embeddedShader * shader = embeddedShaders; shader->name; shader ++) {
		if (strcmp (shader->name, name) == 0) {
			size_t length = strlen (shader->source);
			char * content = (char *) malloc (length + 1);
			if (content) memcpy (content, shader->source, length + 1);
			return content;
		}
	}
	return NULL;
}

static void
printShaderInfoLog (GLuint shader)
{
	GLint     infologLength = 0;
	GLint     charsWritten  = 0;
	char *infoLog;

	printOpenGLError ();  // Check for OpenGL errors
	glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &infologLength);
	printOpenGLError ();  // Check for OpenGL errors

	if (infologLength > 0)
    {
		infoLog = new char [infologLength];
		if (infoLog == NULL)
        {
			debugOut("ERROR: Could not allocate InfoLog buffer");
			return;
        }
		glGetShaderInfoLog (shader, infologLength, &charsWritten, infoLog);
		debugOut("Shader InfoLog:\n%s\n\n", infoLog);
		delete[] infoLog;
    }
	printOpenGLError();  // Check for OpenGL errors
}

/* Print out the information log for a program object */
static void
printProgramInfoLog (GLuint program)
{
	GLint     infologLength = 0;
	GLint     charsWritten  = 0;
	char *infoLog;

	printOpenGLError ();  // Check for OpenGL errors
	glGetProgramiv (program, GL_INFO_LOG_LENGTH, &infologLength);
	printOpenGLError ();  // Check for OpenGL errors

	if (infologLength > 0)
    {
		infoLog = new char [infologLength];
		if (infoLog == NULL)
        {
			debugOut( "ERROR: Could not allocate InfoLog buffer");
			return;
        }
		glGetProgramInfoLog (program, infologLength, &charsWritten, infoLog);
		debugOut( "Program InfoLog:\n%s\n\n", infoLog);
		delete[] infoLog;
    }
	printOpenGLError ();  // Check for OpenGL errors
}

int buildShaders (const char *vertexShader, const char *fragmentShader)
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

	debugOut("Compiling vertex shader... \n");

	// Compile vertex shader and print log
	glCompileShader(VS);
	printOpenGLError();
	glGetShaderiv(VS, GL_COMPILE_STATUS, &vertCompiled);
	printShaderInfoLog (VS);

	debugOut("\nCompiling fragment shader... \n");

	// Compile fragment shader and print log
	glCompileShader(FS);
	printOpenGLError();
	glGetShaderiv(FS, GL_COMPILE_STATUS, &fragCompiled);
	printShaderInfoLog (FS);

	if (!vertCompiled || !fragCompiled)
		return 0;

	debugOut( "\nShaders compiled. \n");


	// Create a program object and attach the two compiled shaders
	prog = glCreateProgram();
	glAttachShader(prog, VS);
	glAttachShader(prog, FS);

	// Clean up
	glDeleteShader (VS);
	glDeleteShader (FS);

	// Link the program and print log
	glLinkProgram(prog);
	printOpenGLError();
	glGetProgramiv(prog, GL_LINK_STATUS, &linked);
	printProgramInfoLog(prog);

	if (!linked)
		return 0;

	debugOut("Shader program linked. \n");

	return prog;
}


