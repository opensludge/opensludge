/*
 *  Graphics.h
 *  SdlSludge
 *
 *  Created by Rikard Peterson on 2009-05-01.
 *
 */

#ifndef __SLUDGE_GRAPHICS_H__
#define __SLUDGE_GRAPHICS_H__

extern int winWidth, winHeight;
extern int viewportHeight, viewportWidth;
extern int viewportOffsetX, viewportOffsetY;
extern int realWinWidth, realWinHeight;

void setPixelCoords (bool pixels);
void setGraphicsWindow(bool fullscreen, bool restoreGraphics = true);


#endif