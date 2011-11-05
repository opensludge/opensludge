bool loadFont (int filenum, const char * charOrder, int);
void pasteString (char * theText, int, int, spritePalette &);
void fixFont (spritePalette & spal);
void setFontColour (spritePalette & sP, byte r, byte g, byte b);
int stringWidth (char * theText);
int stringLength (char * theText);
void pasteStringToBackdrop (char * theText, int xOff, int y, spritePalette & thePal);
void burnStringToBackdrop (char * theText, int xOff, int y, spritePalette & thePal);

bool isInFont (char * theText);
