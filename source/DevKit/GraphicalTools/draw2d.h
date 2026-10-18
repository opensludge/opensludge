/*
 * draw2d.h - Part of the SLUDGE Dev Kit
 *
 * 2D drawing for the views of the floor maker, the z-buffer maker and the
 * sprite bank editor.
 */

#ifndef __DRAW2D_H__
#define __DRAW2D_H__

// Set up a view of w by h pixels, scrolled to (x, y), in which each pixel
// of the view covers zmul pixels of the picture. In setViewArea, y grows
// downwards, as in a backdrop, and in setSpriteViewArea, y grows upwards
// from the hotspot of a sprite.
void setViewArea (int x, int y, int w, int h, float zmul);
void setSpriteViewArea (int x, int y, int w, int h, float zmul);

#endif
