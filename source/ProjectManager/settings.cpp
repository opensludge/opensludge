#include <string.h>
#include <stdio.h>
//#include <unistd.h>

#include "splitter.h"
#include "settings.h"
#include "winterfa.h"
#include "moreio.h"
#include "wintext.h"

extern char * quitMessage, * customIcon, * runtimeDataFolder;
extern unsigned int screenWidth, screenHeight, frameSpeed, winMouseImage;
extern bool runFullScreen, forceSilent, ditherImages;
extern bool startupShowLogo, startupShowLoading, startupInvisible;
extern chrRenderingSettingsStruct chrRenderingSettings;

void blankSettings () {
	// TODO setWindowText (ID_EDIT_OUTPUTFILE, "");
	// TODO setWindowText (ID_EDIT_NAME, "");
}

void noSettings () {
	delete quitMessage;
	delete customIcon;
	quitMessage = joinStrings ("Are you sure you want to quit?", "");
	customIcon = joinStrings ("", "");
	runtimeDataFolder = joinStrings ("", "");
	// TODO setWindowText (ID_EDIT_OUTPUTFILE, "myNewProject");
	// TODO setWindowText (ID_EDIT_NAME, "Untitled SLUDGE project");
	screenWidth = 640;
	screenHeight = 480;
	frameSpeed = 20;
	winMouseImage = 0;
	ditherImages = true;
	runFullScreen = true;
	forceSilent = false;
	startupShowLogo = true;
	startupShowLoading = true;
	startupInvisible = false;
	chrRenderingSettingsFillDefaults(true);
}

unsigned int stringToInt (char * st) {
	unsigned int a = 0;
	for (int i = 0; st[i]; i ++) {
		a *= 10;
		a += st[i] - '0';
	}
	return a;
}

void readDir (char * t) {
	stringArray * splitLine = splitString (t, '=', ONCE);
	if (splitLine -> next) {
		if (strcmp (splitLine -> string, "quitmessage") == 0) {
			delete quitMessage;
			quitMessage = joinStrings ("", splitLine -> next -> string);

		} else if (strcmp (splitLine -> string, "customicon") == 0) {
			delete customIcon;
			customIcon = joinStrings ("", splitLine -> next -> string);

		} else if (strcmp (splitLine -> string, "datafolder") == 0) {
			delete runtimeDataFolder;
			runtimeDataFolder = joinStrings ("", splitLine -> next -> string);

		} else if (strcmp (splitLine -> string, "finalfile") == 0) {
			// TODO setWindowText (ID_EDIT_OUTPUTFILE, splitLine -> next -> string);

		} else if (strcmp (splitLine -> string, "windowname") == 0) {
			// TODO setWindowText (ID_EDIT_NAME, splitLine -> next -> string);
			
		// NEW MOUSE SETTING
		} else if (strcmp (splitLine -> string, "mouse") == 0) {
			winMouseImage = stringToInt (splitLine -> next -> string);

		// OLD MOUSE SETTING
		} else if (strcmp (splitLine -> string, "hidemouse") == 0) {
			winMouseImage = (splitLine -> next -> string[0] == 'Y') ? 2 : 1;

		} else if (strcmp (splitLine -> string, "ditherimages") == 0) {
			ditherImages = (splitLine -> next -> string[0] == 'Y');

		} else if (strcmp (splitLine -> string, "fullscreen") == 0) {
			runFullScreen = (splitLine -> next -> string[0] == 'Y');

		} else if (strcmp (splitLine -> string, "showlogo") == 0) {
			startupShowLogo = (splitLine -> next -> string[0] == 'Y');

		} else if (strcmp (splitLine -> string, "showloading") == 0) {
			startupShowLoading = (splitLine -> next -> string[0] == 'Y');

		} else if (strcmp (splitLine -> string, "invisible") == 0) {
			startupInvisible = (splitLine -> next -> string[0] == 'Y');

		} else if (strcmp (splitLine -> string, "makesilent") == 0) {
			forceSilent = (splitLine -> next -> string[0] == 'Y');

		} else if (strcmp (splitLine -> string, "height") == 0) {
			screenHeight = stringToInt (splitLine -> next -> string);

		} else if (strcmp (splitLine -> string, "width") == 0) {
			screenWidth = stringToInt (splitLine -> next -> string);

		} else if (strcmp (splitLine -> string, "chrRender_def_enabled") == 0) {
			chrRenderingSettings.defEnabled = (splitLine -> next -> string[0] == 'Y');
		} else if (strcmp (splitLine -> string, "chrRender_def_softX") == 0) {
			chrRenderingSettings.defSoftnessX = stringToInt (splitLine -> next -> string);
		} else if (strcmp (splitLine -> string, "chrRender_def_softY") == 0) {
			chrRenderingSettings.defSoftnessY = stringToInt (splitLine -> next -> string);
		} else if (strcmp (splitLine -> string, "chrRender_max_enabled") == 0) {
			chrRenderingSettings.maxEnabled = (splitLine -> next -> string[0] == 'Y');
		} else if (strcmp (splitLine -> string, "chrRender_max_readIni") == 0) {
			chrRenderingSettings.maxReadIni = (splitLine -> next -> string[0] == 'Y');
		} else if (strcmp (splitLine -> string, "chrRender_max_softX") == 0) {
			chrRenderingSettings.maxSoftnessX = stringToInt (splitLine -> next -> string);
		} else if (strcmp (splitLine -> string, "chrRender_max_softY") == 0) {
			chrRenderingSettings.maxSoftnessY = stringToInt (splitLine -> next -> string);

		} else if (strcmp (splitLine -> string, "speed") == 0) {
			frameSpeed = stringToInt (splitLine -> next -> string);
		}
	}
	while (destroyFirst (splitLine)){;}
}

static void fileWriteBool (FILE * fp, const char * theString, bool theBool)
{
	fprintf (fp, "%s=%c\n", theString, theBool ? 'Y' : 'N');
}

void writeSettings (FILE * fp) {
	char * winName = getWindowText (ID_EDIT_NAME),
		 * finFile = getWindowText (ID_EDIT_OUTPUTFILE);
		 
	fprintf 		(fp, "[SETTINGS]\nwindowname=%s\n", winName);
	fprintf 		(fp, "finalfile=%s\n", 				finFile);
	fprintf 		(fp, "datafolder=%s\n", 			runtimeDataFolder);
	fprintf 		(fp, "quitmessage=%s\n", 			quitMessage);
	fprintf 		(fp, "customicon=%s\n", 			customIcon);
	fprintf 		(fp, "mouse=%lu\n", 				winMouseImage);
	fprintf 		(fp, "fullscreen=%c\n", 			runFullScreen ? 'Y':'N');
	fprintf 		(fp, "makesilent=%c\n", 			forceSilent ? 'Y':'N');
	fprintf 		(fp, "showlogo=%c\n", 				startupShowLogo ? 'Y':'N');
	fprintf 		(fp, "showloading=%c\n", 			startupShowLoading ? 'Y':'N');
	fprintf 		(fp, "invisible=%c\n", 				startupInvisible ? 'Y':'N');
	fileWriteBool 	(fp, "ditherimages", 				ditherImages);
	fprintf 	  	(fp, "width=%lu\n",					screenWidth);
	fprintf 	  	(fp, "height=%lu\n", 				screenHeight);
	fprintf 		(fp, "speed=%lu\n", 				frameSpeed);
	
	fileWriteBool	(fp, "chrRender_def_enabled", 		chrRenderingSettings.defEnabled);
	fprintf 	 	(fp, "chrRender_def_softX=%lu\n",	chrRenderingSettings.defSoftnessX);
	fprintf 	 	(fp, "chrRender_def_softY=%lu\n",	chrRenderingSettings.defSoftnessY);

	fileWriteBool	(fp, "chrRender_max_enabled", 		chrRenderingSettings.maxEnabled);
	fileWriteBool	(fp, "chrRender_max_readIni", 		chrRenderingSettings.maxReadIni);
	fprintf 	 	(fp, "chrRender_max_softX=%lu\n",	chrRenderingSettings.maxSoftnessX);
	fprintf 	 	(fp, "chrRender_max_softY=%lu\n",	chrRenderingSettings.maxSoftnessY);
	
	fprintf 		(fp, "\n[FILES]\n");
		
	delete winName;
	delete finFile;
}

void chrRenderingSettingsFillDefaults(bool enable)
{
	chrRenderingSettings.defEnabled = true;
	chrRenderingSettings.defSoftnessX = 4;
	chrRenderingSettings.defSoftnessY = 4;

	chrRenderingSettings.maxEnabled = enable;
	chrRenderingSettings.maxReadIni = true;
	chrRenderingSettings.maxSoftnessX = 100;
	chrRenderingSettings.maxSoftnessY = 100;
}
