#ifdef _WIN32
#include <windows.h>

#include "allfiles.h"

#include "winstuff.h"
#include "newfatal.h"
#include "sprites.h"
#include "sprbanks.h"
#include "fonttext.h"
#include "backdrop.h"
#include "sludger.h"
#include "cursors.h"
#include "objtypes.h"
#include "region.h"
#include "people.h"
#include "talk.h"
#include "direct.h"
#include "sound.h"
#include "colours.h"
#include "moreio.h"
#include "stringy.h"

HINSTANCE hInst;  				// Handle of the main instance
extern HWND hMainWindow;

extern variableStack * noStack;


// The platform-specific functions - Windows edition.

char * grabFileName () {
	OPENFILENAME ofn;
	char path[MAX_PATH];
	char file[MAX_PATH]="";
	
	memset (& ofn, 0, sizeof (ofn));
	ofn.lStructSize = sizeof (ofn);
	ofn.hwndOwner = NULL;
	ofn.hInstance = hInst;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrInitialDir = path;
	ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;
	ofn.lpstrFilter = "SLUDGE games (*.SLG)\0*.slg\0\0";
	ofn.lpstrFile = file;
	
	if (GetOpenFileName (& ofn)) {
		return copyString (file);
	} else {
		return NULL;
	}
}

void msgBox (const char * head, const char * msg) {
	MessageBox (NULL, msg, head, MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL | MB_SETFOREGROUND);
}

int msgBoxQuestion (const char * head, const char * msg) {
	if (MessageBox (NULL, msg, head, MB_YESNO | MB_SETFOREGROUND | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
		return false;
	return true;
}


#endif
