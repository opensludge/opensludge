#ifndef __SLUDGE_GAMEPAD_H__
#define __SLUDGE_GAMEPAD_H__

union SDL_Event;

void initGamepads ();
void handleGamepadEvent (const SDL_Event & event);

// Move the pointer with the sticks and d-pads of the connected controllers.
// Call at least once per frame. We move the pointer by the time since the
// last call, so the movement is smoother with more calls.
void updateGamepadPointer ();

#endif
