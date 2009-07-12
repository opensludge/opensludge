#include "splitter.h"

#ifdef WIN32

#include <windows.h>

extern HWND mainWin;

#define MESS(id,m,w,l) SendDlgItemMessage(mainWin,id,m,(WPARAM)w,(LPARAM)l)

void setWindowText (int where, char * tx) {
	MESS(where, WM_SETTEXT, 0, tx);
}

char * getWindowText (int where) {
	char tx[255];
	MESS(where, WM_GETTEXT, 255, tx);
	return joinStrings (tx, "");
}

#endif

//TODO

void setWindowText (int where, char * tx) {
}

char * getWindowText (int where) {
	return joinStrings ("", "");
}
