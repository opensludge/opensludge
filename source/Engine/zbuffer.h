#ifndef _ZBUFFER_H_
#define _ZBUFFER_H_

#ifdef __linux__
//#include <SDL/SDL_opengl.h>
#else
//#include <SDL_opengl.h>
#endif
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
