/*
 *  shaders.cpp
 *  Sludge Engine
 *
 *  Created by Rikard Peterson on 2009-12-29.
 *
 */

#include <stdio.h>

#include "shaders.h"
#ifdef _WIN32
#include <GL\glu.h> // handy for gluErrorString
#else
#include <OpenGL/glu.h> // handy for gluErrorString
#endif

int
printOglError (const char *file,
               int         line)
{
	/* Returns 1 if an OpenGL error occurred, 0 otherwise. */
	GLenum glErr;
	int    retCode = 0;

	glErr = glGetError ();
	while (glErr != GL_NO_ERROR)
    {
		fprintf (stderr, "glError in file %s @ line %d: %s\n", file, line, gluErrorString (glErr));
		retCode = 1;
		glErr = glGetError ();
    }
	return retCode;
}

#define printOpenGLError() printOglError(__FILE__, __LINE__)

static void
printShaderInfoLog (GLuint shader)
{
	GLint     infologLength = 0;
	GLint     charsWritten  = 0;
	GLchar *infoLog;

	printOpenGLError ();  // Check for OpenGL errors
	glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &infologLength);
	printOpenGLError ();  // Check for OpenGL errors

	if (infologLength > 0)
    {
		infoLog = new GLchar [infologLength];
		if (infoLog == NULL)
        {
			fprintf (stderr, "ERROR: Could not allocate InfoLog buffer");
			return;
        }
		glGetShaderInfoLog (shader, infologLength, &charsWritten, infoLog);
		fprintf (stderr, "Shader InfoLog:\n%s\n\n", infoLog);
		delete infoLog;
    }
	printOpenGLError();  // Check for OpenGL errors
}

/* Print out the information log for a program object */
static void
printProgramInfoLog (GLuint program)
{
	GLint     infologLength = 0;
	GLint     charsWritten  = 0;
	GLchar *infoLog;

	printOpenGLError ();  // Check for OpenGL errors
	glGetProgramiv (program, GL_INFO_LOG_LENGTH, &infologLength);
	printOpenGLError ();  // Check for OpenGL errors

	if (infologLength > 0)
    {
		infoLog = new GLchar [infologLength];
		if (infoLog == NULL)
        {
			fprintf (stderr, "ERROR: Could not allocate InfoLog buffer");
			return;
        }
		glGetProgramInfoLog (program, infologLength, &charsWritten, infoLog);
		fprintf (stderr, "Program InfoLog:\n%s\n\n", infoLog);
		delete infoLog;
    }
	printOpenGLError ();  // Check for OpenGL errors
}

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

	fprintf (stderr, "Compiling vertex shader... ");

	// Compile vertex shader and print log
	glCompileShader(VS);
	printOpenGLError();
	glGetShaderiv(VS, GL_COMPILE_STATUS, &vertCompiled);
	printShaderInfoLog (VS);

	fprintf (stderr, "\n");
	fprintf (stderr, "Compiling fragment shader... ");

	// Compile fragment shader and print log
	glCompileShader(FS);
	printOpenGLError();
	glGetShaderiv(FS, GL_COMPILE_STATUS, &fragCompiled);
	printShaderInfoLog (FS);
	fprintf (stderr, "\n");

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
	printOpenGLError();
	glGetProgramiv(prog, GL_LINK_STATUS, &linked);
	printProgramInfoLog(prog);

	if (!linked)
		return 0;

	fprintf (stderr, "Shader program linked. \n");

	return prog;
}


