#include <stdint.h>

int get2bytes (FILE * fp);
void put2bytes (int numtoput, FILE * fp);
char * readString (FILE * fp);
void writeString (char * s, FILE * fp);
void putFloat (float f, FILE * fp);
float readFloat (FILE * fp);
void putSigned (short f, FILE * fp);
short readSigned (FILE * fp);
int32_t get4bytes (FILE * fp);
void put4bytes (int32_t f, FILE * fp);
/*
void newDebug (char * a);
void newDebug (char * a, int i);
void newDebug (char * a, char * b);
void initDebug ();
// */

char * encodeFilename (char * nameIn);
char * decodeFilename (char * nameIn);
