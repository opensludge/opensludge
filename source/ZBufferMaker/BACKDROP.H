extern int WINWIDTH, WINHEIGHT;

bool loadBackDrop (char * fileName);
void backDropToScreenBuffer ();
bool initBackDrop (int, int);
void blankBackDrop ();
bool saveBackdropToTGA (char * filename);

bool saveHSI (char * filename);
void blankScreen ();
void screenBufferToWindow ();

bool convertTGA (char * filename);

