extern int WINWIDTH, WINHEIGHT;

bool loadBackDrop (char * fileName);
void backDropToScreenBuffer ();
bool initBackDrop (int, int);
bool saveHSI (char * filename);
void blankScreen ();
void screenBufferToWindow ();