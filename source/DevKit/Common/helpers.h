/*
 *  helpers.h
 *  Helper functions that don't depend on other source files.
 */

// Making something with a char *
char * joinStrings (const char * a, const char * b);
char * joinStrings (const char * a, const char * b, const char * c);
char * joinStrings (const char * a, const char * b, const char * c, const char * d);

char * joinQuote (char * a, char * b, char q1, char q2);

#ifdef __cplusplus
extern "C" {
#endif

extern char * sourceDirectory;

bool getSourceDirFromName (const char * filename);
bool gotoSourceDirectory ();

void fixPath (char *filename, bool makeGood);

#ifdef __cplusplus
}
#endif
