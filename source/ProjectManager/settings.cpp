#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "SLUDGE_Functions.H"
#include "SPLITTER.HPP"
#include "settings.h"
#include "translation.h"
#include "MOREIO.H"
#include "MessBox.h"
#include "version.h"
#include "Interface.h"

settingsStruct settings;
programSettingsStruct programSettings;

chrRenderingSettingsStruct chrRenderingSettings;

char * tempDirectory = NULL;
char * sourceDirectory = NULL;
bool silent = true;


void noSettings () {

	if (settings.quitMessage) delete settings.quitMessage;
	settings.quitMessage = joinStrings ("Are you sure you want to quit?", "");
	if (settings.customIcon) delete settings.customIcon;
	settings.customIcon = joinStrings ("", "");
	if (settings.runtimeDataFolder) delete settings.runtimeDataFolder;
	settings.runtimeDataFolder = joinStrings ("save", "");
	if (settings.finalFile) delete settings.finalFile;
	settings.finalFile =joinStrings ("Gamedata", "");
	if (settings.windowName) delete settings.windowName;
	settings.windowName = joinStrings ("New SLUDGE Game", "");
	if (settings.originalLanguage) delete settings.originalLanguage;
	settings.originalLanguage =joinStrings ("English", "");
	settings.screenWidth = 640;
	settings.screenHeight = 480;
	settings.frameSpeed = 20;
	settings.winMouseImage = 0;
	settings.ditherImages = true;
	settings.runFullScreen = true;
	settings.forceSilent = false;
	settings.startupShowLogo = true;
	settings.startupShowLoading = true;
	settings.startupInvisible = false;
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
			delete settings.quitMessage;
			settings.quitMessage = joinStrings ("", splitLine -> next -> string);

		} else if (strcmp (splitLine -> string, "customicon") == 0) {
			if (settings.customIcon) delete settings.customIcon;
			settings.customIcon = joinStrings ("", splitLine -> next -> string);

		} else if (strcmp (splitLine -> string, "datafolder") == 0) {
			if (settings.runtimeDataFolder) delete settings.runtimeDataFolder;
			settings.runtimeDataFolder = joinStrings ("", splitLine -> next -> string);

		} else if (strcmp (splitLine -> string, "finalfile") == 0) {
			if (settings.finalFile) delete settings.finalFile;
			settings.finalFile = joinStrings ("", splitLine -> next -> string);

		} else if (strcmp (splitLine -> string, "windowname") == 0) {
			if (settings.windowName) delete settings.windowName;
			settings.windowName = joinStrings ("", splitLine -> next -> string);

		} else if (strcmp (splitLine -> string, "language") == 0) {
			if (settings.originalLanguage) delete settings.originalLanguage;
			settings.originalLanguage = joinStrings ("", splitLine -> next -> string);

		// NEW MOUSE SETTING
		} else if (strcmp (splitLine -> string, "mouse") == 0) {
			settings.winMouseImage = stringToInt (splitLine -> next -> string);

		// OLD MOUSE SETTING
		} else if (strcmp (splitLine -> string, "hidemouse") == 0) {
			settings.winMouseImage = (splitLine -> next -> string[0] == 'Y') ? 2 : 1;

		} else if (strcmp (splitLine -> string, "ditherimages") == 0) {
			settings.ditherImages = (splitLine -> next -> string[0] == 'Y');

		} else if (strcmp (splitLine -> string, "fullscreen") == 0) {
			settings.runFullScreen = (splitLine -> next -> string[0] == 'Y');

		} else if (strcmp (splitLine -> string, "showlogo") == 0) {
			settings.startupShowLogo = (splitLine -> next -> string[0] == 'Y');

		} else if (strcmp (splitLine -> string, "showloading") == 0) {
			settings.startupShowLoading = (splitLine -> next -> string[0] == 'Y');

		} else if (strcmp (splitLine -> string, "invisible") == 0) {
			settings.startupInvisible = (splitLine -> next -> string[0] == 'Y');

		} else if (strcmp (splitLine -> string, "makesilent") == 0) {
			settings.forceSilent = (splitLine -> next -> string[0] == 'Y');

		} else if (strcmp (splitLine -> string, "height") == 0) {
			settings.screenHeight = stringToInt (splitLine -> next -> string);

		} else if (strcmp (splitLine -> string, "width") == 0) {
			settings.screenWidth = stringToInt (splitLine -> next -> string);

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
			settings.frameSpeed = stringToInt (splitLine -> next -> string);
		}
	}
	while (destroyFirst (splitLine)){;}
}


bool readSettings (FILE * fp) {
	char * grabLine;
	bool keepGoing = true;
	noSettings ();

	while (keepGoing) {
		grabLine = readText (fp);
		if (grabLine && grabLine[0]) {
			readDir (grabLine);
		} else
			keepGoing = false;
		delete grabLine;
	}

	if (! settings.finalFile) return addComment (ERRORTYPE_PROJECTERROR, "Vital line missing from project", "finalfile", NULL);

	return true;
}

void killSettingsStrings ()
{
	if (settings.quitMessage) delete settings.quitMessage;
	if (settings.customIcon) delete settings.customIcon;
	if (settings.runtimeDataFolder) delete settings.runtimeDataFolder;
	if (settings.finalFile) delete settings.finalFile;
	if (settings.windowName) delete settings.windowName;
}

char * newString (const char * old)
{
	char * nS = new char[strlen (old) + 1];
	//	checkNew (nS);
	sprintf (nS, "%s", old);
	return nS;
}


/* killTempDir - Removes the temporary directory
 *
 * Not really needed on Mac OS, since the OS takes care of that for us every reboot,
 * but it's nice to clean up anyway, and I don't think Windows cleans up that way.
 *
 */
void killTempDir() {
	gotoTempDirectory ();

	struct dirent **eps;
	int n = scandir (tempDirectory, &eps, NULL, NULL);
	if (n > 0)
	{
		int cnt;
		for (cnt = 0; cnt < n; ++cnt) {
			unlink (eps[cnt]->d_name);
			free (eps[cnt]);
		}
		free (eps);
	}

	gotoSourceDirectory ();
	rmdir(tempDirectory);
	tempDirectory = NULL;
}

static void fileWriteBool (FILE * fp, const char * theString, bool theBool)
{
	fprintf (fp, "%s=%c\n", theString, theBool ? 'Y' : 'N');
}

void writeSettings (FILE * fp) {

	fprintf 		(fp, "[SETTINGS]\nwindowname=%s\n", settings.windowName);
	fprintf 		(fp, "finalfile=%s\n", 				settings.finalFile);
	fprintf 		(fp, "language=%s\n", 				settings.originalLanguage);
	fprintf 		(fp, "datafolder=%s\n", 			settings.runtimeDataFolder);
	fprintf 		(fp, "quitmessage=%s\n", 			settings.quitMessage);
	fprintf 		(fp, "customicon=%s\n", 			settings.customIcon);
	fprintf 		(fp, "mouse=%lu\n", 				settings.winMouseImage);
	fprintf 		(fp, "fullscreen=%c\n", 			settings.runFullScreen ? 'Y':'N');
	fprintf 		(fp, "makesilent=%c\n", 			settings.forceSilent ? 'Y':'N');
	fprintf 		(fp, "showlogo=%c\n", 				settings.startupShowLogo ? 'Y':'N');
	fprintf 		(fp, "showloading=%c\n", 			settings.startupShowLoading ? 'Y':'N');
	fprintf 		(fp, "invisible=%c\n", 				settings.startupInvisible ? 'Y':'N');
	fileWriteBool 	(fp, "ditherimages", 				settings.ditherImages);
	fprintf 	  	(fp, "width=%lu\n",					settings.screenWidth);
	fprintf 	  	(fp, "height=%lu\n", 				settings.screenHeight);
	fprintf 		(fp, "speed=%lu\n", 				settings.frameSpeed);

	fileWriteBool	(fp, "chrRender_def_enabled", 		chrRenderingSettings.defEnabled);
	fprintf 	 	(fp, "chrRender_def_softX=%lu\n",	chrRenderingSettings.defSoftnessX);
	fprintf 	 	(fp, "chrRender_def_softY=%lu\n",	chrRenderingSettings.defSoftnessY);

	fileWriteBool	(fp, "chrRender_max_enabled", 		chrRenderingSettings.maxEnabled);
	fileWriteBool	(fp, "chrRender_max_readIni", 		chrRenderingSettings.maxReadIni);
	fprintf 	 	(fp, "chrRender_max_softX=%lu\n",	chrRenderingSettings.maxSoftnessX);
	fprintf 	 	(fp, "chrRender_max_softY=%lu\n",	chrRenderingSettings.maxSoftnessY);

	fprintf 		(fp, "\n[FILES]\n");
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


bool gotoSourceDirectory () {
	bool r = chdir (sourceDirectory);
	if (r) return addComment (ERRORTYPE_SYSTEMERROR, "Can't move to source directory", sourceDirectory, NULL);
//	fprintf (stderr, "Now in: %s\n", sourceDirectory);
	return true;
}

bool gotoTempDirectory () {
	if (! tempDirectory) {
		tempDirectory = joinStrings(getTempDir(), "/SLUDGE_Tmp_XXXXXX");
		fixPath (tempDirectory, true);
		if (mktemp (tempDirectory)) {
#ifdef WIN32
			if (mkdir (tempDirectory))
#else
			if (mkdir (tempDirectory, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
#endif
				tempDirectory = NULL;
		}
	}
	if (! tempDirectory) return false;
	bool r = chdir (tempDirectory);
	if (r) return addComment (ERRORTYPE_SYSTEMERROR, "Can't move to temporary directory", tempDirectory, NULL);
	return true;
}

FILE * openFinalFile (char * addMe, char * mode) {
	char * fullName = joinStrings (settings.finalFile, addMe);
	if (! fullName) return NULL;

	gotoSourceDirectory ();
	FILE * fp = fopen (fullName, mode);
	delete fullName;

	return fp;
}

typedef struct _FILETIME {
	unsigned long dwLowDateTime;
	unsigned long dwHighDateTime;
} FILETIME;


#define MOUSE_1		1 << 2
#define MOUSE_2		1 << 4

int winMouseLookup[4] = {
	MOUSE_2,
	0,
	MOUSE_1,
	MOUSE_2 | MOUSE_1
};


void writeFinalData (FILE * mainFile) {
	fprintf (mainFile, "SLUDGE");
	fputc (0, mainFile);
	fprintf (mainFile, "\r\nSLUDGE data file\r\nSLUDGE is (c) Hungry Software and contributors 2006-2010\r\nThis data file must be run using the SLUDGE engine available at http://www.hungrysoftware.com/\r\n");
	fputc (0, mainFile);

	fputc (MAJOR_VERSION, mainFile);		// Major version
	fputc (MINOR_VERSION, mainFile);		// Minor version

	if (programSettings.compilerVerbose) {
		fputc (1, mainFile);
		writeDebugData (mainFile);
	} else {
		fputc (0, mainFile);
	}

	put2bytes (settings.screenWidth, mainFile);
	put2bytes (settings.screenHeight, mainFile);
	fputc ((1 /* reg */	) +
		   (settings.runFullScreen								  << 1) +
		   (winMouseLookup[settings.winMouseImage & 3]				  ) +
		   (silent										  << 3) +
		   // 4 used for mouse image
		   (settings.startupInvisible 							  << 5) +
		   ((! settings.startupShowLogo)								  << 6) +
		   ((! settings.startupShowLoading)							  << 7),

		   mainFile);

	fputc (settings.frameSpeed, mainFile);
	writeString ("", mainFile); // Unused - was used for registration.

	// Now write the date and time of compilation...

	/* TODO - but not used by the engine anyway
		SYSTEMTIME systemTime;*/
	FILETIME fileTime;
	//GetSystemTime (& systemTime);
	//SystemTimeToFileTime (& systemTime, & fileTime);
	fwrite (& fileTime, sizeof (fileTime), 1, mainFile);

	// More bits

	writeString (settings.runtimeDataFolder, mainFile);

	addTranslationIDTable (mainFile, settings.originalLanguage);

	// Max anti-alias settings
	fputc (chrRenderingSettings.maxReadIni, mainFile);
	fputc (chrRenderingSettings.maxEnabled, mainFile);
	putFloat (chrRenderingSettings.maxSoftnessX / 16.f, mainFile);
	putFloat (chrRenderingSettings.maxSoftnessY / 16.f, mainFile);

	writeString ("okSoFar", mainFile);
}





bool getSourceDirFromName (const char * name) {
	char * filename = joinStrings (name, "");

	int a, lastSlash = -1;
	for (a = 0; filename[a]; a ++) {
		if (filename[a] == '/' || filename[a] == '\\') {
			lastSlash = a;
		}
	}
	if (lastSlash != -1) {
		char slashChar = filename[lastSlash];
		filename[lastSlash] = 0;
		if (chdir (filename)) return addComment (ERRORTYPE_SYSTEMERROR, "Can't move to source directory", filename, NULL);
		filename[lastSlash] = slashChar;
	}
	char buff[1000];
	if (! getcwd (buff, 1000)) {
		addComment (ERRORTYPE_SYSTEMERROR, "I can't work out which directory I'm in...", NULL);
		return false;
	}
	sourceDirectory = joinStrings (buff, "");
	return true;
}


// Fix pathnames, because Windows doesn't use the same path separator
// as the rest of the world
void fixPath (char *filename, bool makeGood) {
	if (! filename) return;
	char * ptr;
#ifdef _WIN32
	while (ptr = strstr (filename, "/")) {
		ptr[0] = '\\';
	}
#else
	if (makeGood) {
		while (ptr = strstr (filename, "\\")) {
			ptr[0] = '/';
		}
	} else {
		while (ptr = strstr (filename, "/")) {
			ptr[0] = '\\';
		}
	}
#endif
}


