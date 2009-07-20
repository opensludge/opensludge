#include <stdio.h>

// Input

short getSigned (FILE * fp);
unsigned int get2bytes (FILE * fp);
int get2bytesReverse (FILE * fp);
long get4bytes (FILE * fp);
char * readString (FILE * fp);
char * readText (FILE * fp);
char * grabWholeFile (char * theName);


// Output

void putSigned (short, FILE * fp);
void put2bytes (unsigned int numtoput, FILE * fp);
void put2bytesR (int numtoput, FILE * fp);
void put4bytes (long int i, FILE * fp);
void writeString (const char * txt, FILE * fp);
void putFloat (float f, FILE * fp);

// Misc

bool newerFile (char * newFileN, char * oldFileN);
char * copyString (const char * c);