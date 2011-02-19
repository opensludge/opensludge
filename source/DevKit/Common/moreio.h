#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdint.h>

// Input

short getSigned (FILE * fp);
unsigned int get2bytes (FILE * fp);
int get2bytesReverse (FILE * fp);
int32_t get4bytes (FILE * fp);
char * readString (FILE * fp);
char * readText (FILE * fp);
char * grabWholeFile (char * theName);


// Output

void putSigned (short, FILE * fp);
void put2bytes (unsigned int numtoput, FILE * fp);
void put2bytesR (int numtoput, FILE * fp);
void put4bytes (int32_t i, FILE * fp);
void writeString (const char * txt, FILE * fp);
void putFloat (float f, FILE * fp);

// Misc

bool newerFile (char * newFileN, char * oldFileN);
void deleteString(char *);
char * copyString (const char * c);


#ifdef __cplusplus
}
#endif
