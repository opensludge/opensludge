#include "allfiles.h"
#include "stringy.h"
#include "newfatal.h"

#define DO_TEST		0

#if DO_TEST
static OSVERSIONINFOEX * winfo = NULL;
#endif

BOOL winVersionInit () {
#if DO_TEST
	winfo = new OSVERSIONINFOEX;
	if (! checkNew (winfo)) return FALSE;

	ZeroMemory (winfo, sizeof (OSVERSIONINFOEX));
	winfo->dwOSVersionInfoSize = sizeof (OSVERSIONINFOEX);
	
	if (GetVersionEx ((OSVERSIONINFO *) winfo) == 0) {
		warning ("Couldn't determine which version of Windows you're using! I'll try and struggle on regardless...");
		delete winfo;
		winfo = NULL;
	}
#endif
	return TRUE;
}

BOOL winVersionNeedDumbMidiFix() {
#if DO_TEST
	if (! winfo) return TRUE;	// Let's do things properly if on a platform we know nothing about

	warning ("Do we need the dumb MIDI fix?");
	
	if (winfo->dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		warning ("You're using Windows 95, 98 or ME!");
		
		char buff[200];
		sprintf (buff, "dwMinorVersion == %d", winfo->dwMinorVersion);
		warning (buff);
		
		if (winfo->dwMinorVersion >= 90) {
			warning ("That makes it Windows ME! MIDI bodge ahoy!");
			return TRUE;
		}
	}
#endif
	return FALSE;
} 

