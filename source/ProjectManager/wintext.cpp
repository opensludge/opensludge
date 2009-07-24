#include "Splitter.hpp"


void setWindowText (int where, const char * tx) {
}

char * getWindowText (int where) {
	return joinStrings ("", "");
}

/*
//extern HWND compWin;

//#define COMPMESS(id,m,w,l) SendDlgItemMessage(compWin,id,m,(WPARAM)w,(LPARAM)l)

void setWindowText (const int where, const char * theText) {
	//	COMPMESS (where, WM_SETTEXT, 0, theText);
}

void setWindowInt (const int where, const int val)
{
	//	char buff[255];
	//	sprintf (buff, "%i", val);
	//	setWindowText (where, buff);
}*/

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
