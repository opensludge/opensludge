#ifndef __SLUDGE_POINTER_H__
#define __SLUDGE_POINTER_H__

#include <SDL3/SDL_stdinc.h>

// Save the game frame that we are drawing, before we draw the cursor in it.
void saveFrameWithoutCursor ();
// Stop moving the pointer between frames until the next saved frame, for
// example during a transition, in which we change the whole frame.
void forgetFrameWithoutCursor ();

// Clear the window and draw the last saved game frame, without the cursor.
// Return false when there is no saved frame.
bool drawFrameWithoutCursor ();

// Call while waiting for the next game frame. When the pointer has moved,
// show the last game frame again with the cursor at the new position.
void movePointerBetweenFrames (Uint32 millisecondsLeft);

#endif
