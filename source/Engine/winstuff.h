#ifdef _WIN32
#include <windows.h>
#endif

#ifndef _WIN32
#ifndef HINSTANCE
#define HINSTANCE int
#endif
#endif

void setWindowName (const char * tx);
bool InitApplication (HINSTANCE hInstance);
bool InitInstance (HINSTANCE hInstance, const char *);
