#ifdef WIN32

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "wincomp.h"
#include "WINBOX.H"
#include "WINTERFA.H"
#include "MessBox.h"
#include "SPLITTER.HPP"
#include "preproc.h"
#include "SLUDGE_Functions.H"
#include "REALPROC.H"
#include "registry.h"
#include "settings.h"
#include "LINKER.H"
#include "OBJTYPE.H"
#include "MOREIO.H"
#include "dumpfiles.h"
#include "PercBar.h"
#include "wintext.h"
#include "ALLKNOWN.H"
#include "backdrop.h"
#include "translation.h"
#include "checkUsed.h"

HWND compWin=NULL;
HWND warningWindowH=NULL;

extern HINSTANCE inst;

extern stringArray * functionNames;

extern stringArray * allFileHandles = NULL;

extern int numStringsFound;
extern int numFilesFound;


static compilationSpace globalSpace;
static int data1 = 0, numProcessed = 0;




bool APIENTRY warningBoxFunc (HWND h, UINT m, WPARAM w, LPARAM l) {
	switch (m) {
		case WM_COMMAND:
		switch (LOWORD (w)) {
			case IDCANCEL:
			return 1;
			
			case ID_WARNINGLIST:
			if (HIWORD(w) == LBN_DBLCLK)
			{
				int n = SendMessage (GetDlgItem (h, ID_WARNINGLIST), LB_GETCURSEL, 0, 0);
				if (n != LB_ERR)
					userClickedErrorLine(n);
			}
			return 1;
		}
		
//		case LBN_DBLCLK:
//		
//		break;
		
		case WM_INITDIALOG:
		warningWindowH = h;
		break;
	}
	
	return 0;
}


#endif