#include "allfiles.h"
#include "newfatal.h"
#include "registry.h"

#define GOOD DISP_CHANGE_SUCCESSFUL

#define DEBUG_RESCHANGE		0

extern int winWidth, winHeight, desiredfps;
static DEVMODE * dm = NULL;

#if DEBUG_RESCHANGE
static void resOut (char * txt, DEVMODE & dm, int score) {
	FILE * fp = fopen ("resout.txt", "at");
	fprintf (fp, "%s (%d x %d, %d BPP, %d Hz) score %d\n", txt, dm.dmPelsWidth, dm.dmPelsHeight, dm.dmBitsPerPel, dm.dmDisplayFrequency, score);
	fclose (fp);
}
#endif

BOOL fullScreenMe () {
	return dm ? ChangeDisplaySettings (dm, CDS_FULLSCREEN) == GOOD : FALSE;
}

void initResChange (int tryFindRefreshRate) {
	int depthArray[2] = {16, 32};
	char lookUp[20];
	sprintf (lookUp, "%d x %d", winWidth, winHeight);
	
	dm = new DEVMODE;
	if (! checkNew (dm)) return;
	
	DEVMODE devMode;
	int devModeBestScore = -1;

	if (tryFindRefreshRate == 0)
	{
		if (EnumDisplaySettings (NULL, ENUM_CURRENT_SETTINGS, &devMode) == 1)
		{
			tryFindRefreshRate = devMode.dmDisplayFrequency;
#if DEBUG_RESCHANGE
			resOut ("Got current settings", devMode, -1);
#endif		
		}
		else
		{
			tryFindRefreshRate = desiredfps;
		}
	}

	for (int i = 0;; i++) {
		if (EnumDisplaySettings (NULL, i, &devMode) != 1) break;
	
		if (winWidth == devMode.dmPelsWidth && winHeight == devMode.dmPelsHeight) {
			int thisScore = -1;

				if (devMode.dmDisplayFrequency)
					thisScore += 1000 - abs((int)(devMode.dmDisplayFrequency - tryFindRefreshRate)) + 1000 * (devMode.dmDisplayFrequency >= tryFindRefreshRate);
				else
					thisScore += 10;

				switch (devMode.dmBitsPerPel) {
					case 16:	// Best
					thisScore += 50;
					break;
						
					case 32:	// Really?
					case 24:
					break;
					
					default:
					thisScore = -1;
					break;
				}
#if DEBUG_RESCHANGE
			resOut ("Found", devMode, thisScore);
#endif

			if (thisScore > devModeBestScore) {
				memcpy (dm, &devMode, sizeof (DEVMODE));
				devModeBestScore = thisScore;
			}
		}
	}
	
	if (devModeBestScore < 0) {
		delete dm;
		dm = NULL;

#if DEBUG_RESCHANGE
	} else {
		resOut ("Best", *dm, devModeBestScore);
#endif

	}
}

void oldScreenSize () {
	ChangeDisplaySettings(NULL, 0);
}
