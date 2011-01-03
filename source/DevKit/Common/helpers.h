/*
 *  helpers.h
 *  Helper functions that don't depend on other source files.
 */

// Making something with a char *
char * joinStrings (const char * a, const char * b);
char * joinStrings (const char * a, const char * b, const char * c);
char * joinStrings (const char * a, const char * b, const char * c, const char * d);

char * joinQuote (char * a, char * b, char q1, char q2);
