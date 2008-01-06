#include "allfiles.h"
#include "rezchange.h"
#include "newfatal.h"

bool setMyResolution (int width, int height, int colours)
{
	DEVMODE devMode;
	
	for (int i = 0;; i++) {
		if (EnumDisplaySettings (NULL, i, &devMode) != 1) return 0;
		
		if (width == devMode.dmPelsWidth &&
			height == devMode.dmPelsHeight &&
			colours == devMode.dmBitsPerPel) /* Found one! */ break;
	}
	
	switch (ChangeDisplaySettings (& devMode, CDS_FULLSCREEN)) {
		case DISP_CHANGE_SUCCESSFUL:
		return 1;
		
		case DISP_CHANGE_RESTART:
		warning ("Can't change to that resolution without restarting!");
		return 0;

		case DISP_CHANGE_BADFLAGS:
		warning ("Can't change resolution - bad flags.");
		return 0;
		
		case DISP_CHANGE_BADPARAM:
		warning ("An invalid parameter was passed in.");
		return 0;
		
		case DISP_CHANGE_FAILED:
		warning ("The display driver failed the specified graphics mode.");
		return 0;
		
		case DISP_CHANGE_BADMODE:
		warning ("The graphics mode is not supported.");
	}
	return 0;
}


