#ifndef _ZBUFFER_H_
#define _ZBUFFER_H_

#if !defined(HAVE_GLES2)
#include <GL/glew.h>
#else
#include <GLES2/gl2.h>
#endif

struct zBufferData {
	int width, height;
//	bool loaded;
	int numPanels;
	int panel[16];
	int originalNum;

	GLubyte * tex;
	GLuint texName;
};

bool setZBuffer (int y);
void killZBuffer ();
void drawZBuffer(int x, int y, bool upsidedown);

#endif
