/*
 * draw2d.cpp - Part of the SLUDGE Dev Kit
 */

#include <epoxy/gl.h>

#include "draw2d.h"

void setViewArea (int x, int y, int w, int h, float zmul) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(x*zmul, (x+w)*zmul, (-y+h)*zmul, -y*zmul, 1.0, -1.0);
	glMatrixMode(GL_MODELVIEW);
}

void setSpriteViewArea (int x, int y, int w, int h, float zmul) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(x*zmul, (x+w)*zmul, (y-h)*zmul, y*zmul, 1.0, -1.0);
	glMatrixMode(GL_MODELVIEW);
}
