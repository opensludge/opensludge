/*
 * draw2d.h - Part of the SLUDGE Dev Kit
 *
 * 2D drawing for the views of the floor maker, the z-buffer maker and the
 * sprite bank editor, with OpenGL 3.2.
 */

#ifndef __DRAW2D_H__
#define __DRAW2D_H__

#include <epoxy/gl.h>

// Create the shader program and the vertex buffer that we draw with, in the
// current OpenGL context. We draw in one context, so after the first call,
// startDrawing2D returns at once.
bool startDrawing2D ();

// Set up a view of w by h pixels, scrolled to (x, y), in which each pixel
// of the view covers zmul pixels of the picture. In setViewArea, y grows
// downwards, as in a backdrop, and in setSpriteViewArea, y grows upwards
// from the hotspot of a sprite.
void setViewArea (int x, int y, int w, int h, float zmul);
void setSpriteViewArea (int x, int y, int w, int h, float zmul);

// Set the colour of what we draw next. We multiply the colours of a texture
// by it.
void setDrawingColour (float r, float g, float b, float a);

// Draw count points, given as x, y pairs: as separate lines between pairs of
// points, as a closed line through all of them, or as a filled convex
// polygon.
void drawLines (const float * points, int count);
void drawLineLoop (const float * points, int count);
void drawPolygon (const float * points, int count);

// Draw the part of the texture from (tx1, ty1) to (tx2, ty2) in the rectangle
// from (x1, y1) to (x2, y2).
void drawTexture (GLuint texture, float x1, float y1, float x2, float y2,
	float tx1, float ty1, float tx2, float ty2);

#endif
