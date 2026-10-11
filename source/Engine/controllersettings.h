#ifndef __SLUDGE_CONTROLLERSETTINGS_H__
#define __SLUDGE_CONTROLLERSETTINGS_H__

union SDL_Event;

bool controllerSettingsAreOpen ();
void openControllerSettings ();

// Handle a controller event. While the settings are open, we use every
// button and axis event of the controllers. While they are closed, we use
// the Back button, which opens them. Return whether we used the event.
bool handleControllerSettingsEvent (const SDL_Event & event);

// Call instead of running the game while the settings are open. We read
// the mouse and keyboard input, draw the settings and show them.
void updateControllerSettings ();

// Remember that the player used a controller, to show the hint about how to
// open the settings: the first time after a controller connects, and when
// unmappedInput is true. We pass this function to setGamepadUseHandler.
void noteControllerUse (bool unmappedInput);

// Draw the hint about how to open the settings, when we show it. Call
// while drawing a game frame.
void drawControllerSettingsHint ();

#endif
