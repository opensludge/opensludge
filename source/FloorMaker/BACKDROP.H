extern int WINWIDTH, WINHEIGHT;

BOOL loadBackDrop (char * fileName);
void backDropToScreenBuffer ();
BOOL initBackDrop (int, int);
BOOL saveHSI (char * filename);
void blankScreen ();
void screenBufferToWindow ();