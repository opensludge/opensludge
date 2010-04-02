#include <stdint.h>

int get2bytes (FILE * fp);
void put2bytes (int numtoput, FILE * fp);
char * readString (FILE * fp);
void writeString (char * s, FILE * fp);
void putFloat (float f, FILE * fp);
float getFloat (FILE * fp);
void putSigned (short f, FILE * fp);
short getSigned (FILE * fp);
int32_t get4bytes (FILE * fp);
void put4bytes (uint32_t f, FILE * fp);

char * encodeFilename (char * nameIn);
char * decodeFilename (char * nameIn);
