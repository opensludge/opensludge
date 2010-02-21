#ifndef _ZBUFFER_H_
#define _ZBUFFER_H_

#include "GLee.h"

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
