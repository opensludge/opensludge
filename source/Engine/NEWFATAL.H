#include "errors.h"

int fatal (const char * str);
int fatal (const char * str1, const char * str2);
int checkNew (const void * mem);
void displayFatal ();
void registerWindowForFatal (const HWND v);
void setFatalInfo (char * userFunc, char * BIF);
void warning (const char * l);
void setResourceForFatal (int n);
char * resourceNameFromNum (int i);