#include <windows.h>
#include "winbox.h"
#include "registry.h"

HINSTANCE inst;

BOOL runMe = FALSE;

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
//	MSG msg;
	inst=hInstance;
	int flagHunt = 0;
	
	while (lpCmdLine[flagHunt] == '-') {
		flagHunt ++;
		switch (lpCmdLine[flagHunt]) {
			case 'r':
			flagHunt ++;
			runMe = TRUE;
			break;
		}
		while (lpCmdLine[flagHunt] == ' ') {
			flagHunt ++;
		}
	}

	if (lpCmdLine[flagHunt]) compileWindow (lpCmdLine + flagHunt, getRegString ("regName"), getRegString ("regCode"));

	return 0;
}
