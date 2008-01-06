extern int WINWIDTH, WINHEIGHT;

BOOL loadBackDrop (char * fileName);
void backDropToScreenBuffer ();
BOOL initBackDrop (int, int);
void blankBackDrop ();
BOOL saveBackdropToTGA (char * filename);