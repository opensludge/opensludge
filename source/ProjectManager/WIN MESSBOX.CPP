#ifdef WIN32

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "messbox.h"

extern HWND mainWin;

void messageBox (const char * tx2, const char * tx1) {
		MessageBox (mainWin, tx1, tx2, MB_OK | MB_SETFOREGROUND);
}

bool errorBox (const char * txt1, const char * txt2) {
	char * a = new char[strlen (txt1) + strlen (txt2) + 10];
	if (a) {
		sprintf (a, "%s:\n%s", txt1, txt2);
		messageBox (er, a);
		delete a;
	} else {
		messageBox (er, "Out of memory displaying error message");
	}
	return false;
}

bool errorBox (const char * txt1, unsigned int number) {
	char * a = new char[strlen (txt1) + 30];
	if (a) {
		sprintf (a, "%s:\n%lu", txt1, number);
		messageBox (er, a);
		delete a;
	} else {
		messageBox (er, "Out of memory displaying error message");
	}
	return false;
}

#endif