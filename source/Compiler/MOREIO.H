// Input

unsigned int get2bytes (FILE * fp);
long get4bytes (FILE * fp);
char * readString (FILE * fp);
char * readText (FILE * fp);
char * grabWholeFile (char * theName);

// Output

void put2bytes (unsigned int numtoput, FILE * fp);
void put4bytes (long int i, FILE * fp);
void writeString (const char * txt, FILE * fp);
void putFloat (float f, FILE * fp);

// Misc

BOOL newerFile (char * newFileN, char * oldFileN);