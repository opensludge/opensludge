#include "errors.h"
//#include <windows.h>

bool hasFatal ();

int fatal (const char * str);
int fatal (const char * str1, const char * str2);
int checkNew (const void * mem);
void displayFatal ();
void registerWindowForFatal ();
void setFatalInfo (const char * userFunc, const char * BIF);
void warning (const char * l);
void setResourceForFatal (int n);
char * resourceNameFromNum (int i);