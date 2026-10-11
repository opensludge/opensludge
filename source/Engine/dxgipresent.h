#ifndef __SLUDGE_DXGIPRESENT_H__
#define __SLUDGE_DXGIPRESENT_H__

#if defined(_WIN32)

#include <SDL3/SDL_video.h>

// Show the frames in window through a DXGI swap chain. We create the OpenGL
// context in a hidden window, and we draw into a frame buffer object in
// that context. The window must not have SDL_WINDOW_OPENGL.
// Return the context, or NULL when this is not possible on this computer,
// in which case we leave nothing behind.
SDL_GLContext startDXGIPresentation (SDL_Window * window);

bool presentingWithDXGI ();

// Make the frame buffer and the swap chain as large as the window, in
// pixels.
void resizeDXGIPresentation (int width, int height);

// Show the frame that we drew, and wait for the display.
void presentWithDXGI ();

#endif

#endif
