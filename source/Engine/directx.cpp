#include "allfiles.h"
#include <ddraw7.h>

extern int winWidth, winHeight;
LPDIRECTDRAW7  lpdd7;
extern HWND hMainWindow;

BOOL initResChange () {
	CoInitialize (NULL);
	CoCreateInstance(CLSID_DirectDraw, NULL, CLSCTX_ALL, IID_IDirectDraw7, (void**)&lpdd7);
	lpdd7->Initialize(NULL);

//	return (DirectDrawCreate (NULL, & lpDD, NULL) == DD_OK);
}

BOOL fullScreenMe () {
	if (lpdd7 -> SetDisplayMode (winWidth, winHeight, 16) == DD_OK) return TRUE;
	return lpdd7 -> SetDisplayMode (winWidth, winHeight, 24) == DD_OK;
}

void oldScreenSize () {
	lpdd7 -> RestoreDisplayMode ();
}
