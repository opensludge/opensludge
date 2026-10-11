#ifndef SLUDGE_GL_H
#define SLUDGE_GL_H

/*
 * OpenGL headers for the engine.
 *
 * With desktop OpenGL, we load the functions newer than OpenGL 1.1 at run
 * time with SDL_GL_GetProcAddress, in sludge_gl.cpp, so we use no extension
 * loading library. With OpenGL ES 2.0, all the functions are in the system
 * library.
 */

#if defined(HAVE_GLES2)

#include <SDL3/SDL_opengles2.h>

#else

#include <SDL3/SDL_opengl.h>

#define SLUDGE_GL_FUNCTIONS(X) \
	X(PFNGLACTIVETEXTUREPROC, glActiveTexture) \
	X(PFNGLBLENDFUNCSEPARATEPROC, glBlendFuncSeparate) \
	X(PFNGLATTACHSHADERPROC, glAttachShader) \
	X(PFNGLBINDATTRIBLOCATIONPROC, glBindAttribLocation) \
	X(PFNGLCOMPILESHADERPROC, glCompileShader) \
	X(PFNGLCREATEPROGRAMPROC, glCreateProgram) \
	X(PFNGLCREATESHADERPROC, glCreateShader) \
	X(PFNGLDELETESHADERPROC, glDeleteShader) \
	X(PFNGLDISABLEVERTEXATTRIBARRAYPROC, glDisableVertexAttribArray) \
	X(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray) \
	X(PFNGLGETATTRIBLOCATIONPROC, glGetAttribLocation) \
	X(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog) \
	X(PFNGLGETPROGRAMIVPROC, glGetProgramiv) \
	X(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog) \
	X(PFNGLGETSHADERIVPROC, glGetShaderiv) \
	X(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation) \
	X(PFNGLLINKPROGRAMPROC, glLinkProgram) \
	X(PFNGLSHADERSOURCEPROC, glShaderSource) \
	X(PFNGLUNIFORM1FPROC, glUniform1f) \
	X(PFNGLUNIFORM1IPROC, glUniform1i) \
	X(PFNGLUNIFORM4FPROC, glUniform4f) \
	X(PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv) \
	X(PFNGLUSEPROGRAMPROC, glUseProgram) \
	X(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer)

#define SLUDGE_GL_DECLARE(type, name) extern type sludge_##name;
SLUDGE_GL_FUNCTIONS(SLUDGE_GL_DECLARE)
#undef SLUDGE_GL_DECLARE

// With these macros, a call to glUseProgram is a call through the function
// pointer sludge_glUseProgram, and so on. We define them after including the
// system headers, so that the prototypes in those headers stay unchanged.
#define glActiveTexture sludge_glActiveTexture
#define glBlendFuncSeparate sludge_glBlendFuncSeparate
#define glAttachShader sludge_glAttachShader
#define glBindAttribLocation sludge_glBindAttribLocation
#define glCompileShader sludge_glCompileShader
#define glCreateProgram sludge_glCreateProgram
#define glCreateShader sludge_glCreateShader
#define glDeleteShader sludge_glDeleteShader
#define glDisableVertexAttribArray sludge_glDisableVertexAttribArray
#define glEnableVertexAttribArray sludge_glEnableVertexAttribArray
#define glGetAttribLocation sludge_glGetAttribLocation
#define glGetProgramInfoLog sludge_glGetProgramInfoLog
#define glGetProgramiv sludge_glGetProgramiv
#define glGetShaderInfoLog sludge_glGetShaderInfoLog
#define glGetShaderiv sludge_glGetShaderiv
#define glGetUniformLocation sludge_glGetUniformLocation
#define glLinkProgram sludge_glLinkProgram
#define glShaderSource sludge_glShaderSource
#define glUniform1f sludge_glUniform1f
#define glUniform1i sludge_glUniform1i
#define glUniform4f sludge_glUniform4f
#define glUniformMatrix4fv sludge_glUniformMatrix4fv
#define glUseProgram sludge_glUseProgram
#define glVertexAttribPointer sludge_glVertexAttribPointer

#endif

// Load the OpenGL functions for the current context. Returns NULL on
// success, or the name of the first missing function.
const char * loadGLFunctions ();

// Return the OpenGL version of the current context as major * 10 + minor,
// for example 21 for OpenGL 2.1.
int getGLVersion ();

#endif
