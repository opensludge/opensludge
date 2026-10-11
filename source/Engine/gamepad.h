#ifndef __SLUDGE_GAMEPAD_H__
#define __SLUDGE_GAMEPAD_H__

union SDL_Event;

void initGamepads ();
void handleGamepadEvent (const SDL_Event & event);

// Set the function that we call each time the player uses a controller:
// when the player presses a button, pulls a trigger, or moves the pointer
// with a stick or the d-pad. We pass true for unmappedInput when the player
// presses a button or pulls a trigger that the player can map, and that is
// mapped to nothing.
void setGamepadUseHandler (void (* handler) (bool unmappedInput));

// Move the pointer with the sticks and d-pads of the connected controllers.
// Call at least once per frame. We move the pointer by the time since the
// last call, so the movement is smoother with more calls.
void updateGamepadPointer ();

#endif
