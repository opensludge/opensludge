void pickAnimCursor (struct personaAnimation * pp);
// Draw the current frame of the mouse cursor at its position, and move its
// animation on to the next frame.
void displayCursor ();
// Draw the current frame of the mouse cursor at a position in game
// coordinates, without moving its animation on.
void drawCursor (int x, int y);
bool hasCursorSprite ();
void pasteCursor (int x, int y, struct personaAnimation * c);
