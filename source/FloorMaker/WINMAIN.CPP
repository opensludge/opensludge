#ifdef WIN32

#include <windows.h>
//#include <stdlib.h>
//#include <malloc.h>
//#include <string.h>

#include "winstuff.h"
#include "backdrop.h"
#include "data.h"

//#pragma argsused
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
							LPSTR lpCmdLine, int nCmdShow) {
	MSG msg;

	if (! initBackDrop (640, 480)) return false;
	noFloor ();
	if (lpCmdLine[0]) {
		if (loadFromFile (lpCmdLine)) setLoadedFile (lpCmdLine);
	}

	if (! InitApplication (hInstance)) return false;

	// Finally, start the application...
	if (! InitInstance (hInstance, nCmdShow)) return false;
	
	GetMessage(&msg, NULL, 0, 0);
	// Handle all the input...
	while (GetMessage (& msg, NULL, 0, 0)) {
		TranslateMessage (& msg);
		DispatchMessage (& msg);
	};

	return (msg.wParam);
}

#endif