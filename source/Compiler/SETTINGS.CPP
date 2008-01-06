#include <windows.h>
#include <stdio.h>
#include <unistd.h>

#include "version.h"
#include "splitter.h"
#include "moreio.h"
#include "function.h"
#include "messbox.h"
#include "registry.h"
#include "translation.h"
#include "tga.h"
#include "settings.h"

char * tempDirectory = NULL;
char * sourceDirectory = NULL;
char * windowName = joinStrings ("Untitled SLUDGE project", "");
char * quitMessage = joinStrings ("Are you completely sure that you're determined that you really want to exit?", "");
char * finalFile = joinStrings ("Untitled SLUDGE project", "");
char * customIcon = joinStrings ("", "");
char * dataFolderName = joinStrings ("", "");
int finalWidth = 640;
int finalHeight = 480;
int frameRate = 20;

chrRenderingSettingsStruct chrRenderingSettings =
{
	true, 4, 4, false, true, 100, 100
};

extern char * myNameIs;

BOOL leaveCompressedImages = TRUE;
BOOL useCompressedImages = TRUE;
int winMouseImage = 0;
BOOL runFullScreen = FALSE;
BOOL silent = TRUE;
BOOL forceSilent = FALSE;
BOOL startupHideLogo = TRUE;
BOOL startupHideLoading = TRUE;
BOOL startupInvisible = FALSE;

BOOL getSourceDirFromName (char * filename) {
	int a, lastSlash = -1;
	for (a = 0; filename[a]; a ++) {
		if (filename[a] == '/' || filename[a] == '\\') {
			lastSlash = a;
		}
	}
	if (lastSlash != -1) {
		char slashChar = filename[lastSlash];
		filename[lastSlash] = 0;
		if (chdir (filename)) return errorBox (ERRORTYPE_SYSTEMERROR, "Can't move to source directory", filename, NULL);
		filename[lastSlash] = slashChar;
	}
	char buff[1000];
	if (! getcwd (buff, 1000)) return errorBox (ERRORTYPE_SYSTEMERROR, "I can't work out which directory I'm in...", NULL, NULL);
	sourceDirectory = joinStrings (buff, "");
//	errorBox ("I think the source directory is", sourceDirectory);
	return TRUE;
}

char * grabEnv (const char * la) {
	char buffer[500];
	char * returnVal;
	if (ExpandEnvironmentStrings (la, buffer, 499) == 0) {
		errorBox (ERRORTYPE_SYSTEMERROR, "Can't expand string containing environment variable(s)", la, NULL);
		return NULL;
	}
	if (! buffer[0]) {
		errorBox (ERRORTYPE_SYSTEMERROR, "No environment variable(s)", la, NULL);
		return NULL;
	}
	returnVal = joinStrings (buffer, "");
//	errorBox ("Expanded to", returnVal);
	return returnVal;
}

void readDir (char * t) {
	stringArray * splitLine = splitString (t, '=', ONCE);
	if (countElements (splitLine) == 2) {
		if (strcmp (splitLine -> string, "finalfile") == 0) {
			delete finalFile;
			finalFile = joinStrings (splitLine -> next -> string, "");
		} else if (strcmp (splitLine -> string, "quitmessage") == 0) {
			delete quitMessage;
			quitMessage = joinStrings (splitLine -> next -> string, "");
		} else if (strcmp (splitLine -> string, "customicon") == 0) {
			delete customIcon;
			customIcon = joinStrings (splitLine -> next -> string, "");
		} else if (strcmp (splitLine -> string, "windowname") == 0) {
			delete windowName;
			windowName = joinStrings (splitLine -> next -> string, "");
		} else if (strcmp (splitLine -> string, "datafolder") == 0) {
			delete dataFolderName;
			dataFolderName = joinStrings (splitLine -> next -> string, "");
		} else if (strcmp (splitLine -> string, "hidemouse") == 0) {
			winMouseImage = (splitLine -> next -> string[0] == 'Y') ? 2 : 1;
		} else if (strcmp (splitLine -> string, "fullscreen") == 0) {
			runFullScreen = (splitLine -> next -> string[0] == 'Y');
		} else if (strcmp (splitLine -> string, "makesilent") == 0) {
			forceSilent = (splitLine -> next -> string[0] == 'Y');
		} else if (strcmp (splitLine -> string, "ditherimages") == 0) {
			setDither (splitLine -> next -> string[0] == 'Y');

		} else if (strcmp (splitLine -> string, "invisible") == 0) {
			startupInvisible = (splitLine -> next -> string[0] == 'Y');

		} else if (strcmp (splitLine -> string, "showlogo") == 0) {
			startupHideLogo = (splitLine -> next -> string[0] == 'N');

		} else if (strcmp (splitLine -> string, "showloading") == 0) {
			startupHideLoading = (splitLine -> next -> string[0] == 'N');

		} else if (strcmp (splitLine -> string, "speed") == 0) {
			frameRate = stringToInt (splitLine -> next -> string, ERRORTYPE_PROJECTERROR);
		} else if (strcmp (splitLine -> string, "mouse") == 0) {
			winMouseImage = stringToInt (splitLine -> next -> string, ERRORTYPE_PROJECTERROR);
		} else if (strcmp (splitLine -> string, "width") == 0) {
			finalWidth = stringToInt (splitLine -> next -> string, ERRORTYPE_PROJECTERROR);
		} else if (strcmp (splitLine -> string, "height") == 0) {
			finalHeight = stringToInt (splitLine -> next -> string, ERRORTYPE_PROJECTERROR);

		} else if (strcmp (splitLine -> string, "chrRender_def_enabled") == 0) {
			chrRenderingSettings.defEnabled = (splitLine -> next -> string[0] == 'Y');
		} else if (strcmp (splitLine -> string, "chrRender_def_softX") == 0) {
			chrRenderingSettings.defSoftnessX = stringToInt (splitLine -> next -> string, ERRORTYPE_PROJECTERROR);
		} else if (strcmp (splitLine -> string, "chrRender_def_softY") == 0) {
			chrRenderingSettings.defSoftnessY = stringToInt (splitLine -> next -> string, ERRORTYPE_PROJECTERROR);
		} else if (strcmp (splitLine -> string, "chrRender_max_enabled") == 0) {
			chrRenderingSettings.maxEnabled = (splitLine -> next -> string[0] == 'Y');
		} else if (strcmp (splitLine -> string, "chrRender_max_readIni") == 0) {
			chrRenderingSettings.maxReadIni = (splitLine -> next -> string[0] == 'Y');
		} else if (strcmp (splitLine -> string, "chrRender_max_softX") == 0) {
			chrRenderingSettings.maxSoftnessX = stringToInt (splitLine -> next -> string, ERRORTYPE_PROJECTERROR);
		} else if (strcmp (splitLine -> string, "chrRender_max_softY") == 0) {
			chrRenderingSettings.maxSoftnessY = stringToInt (splitLine -> next -> string, ERRORTYPE_PROJECTERROR);
		}
	}
	destroyAll (splitLine);
}

BOOL readSettings (FILE * fp) {
	char * grabLine;
	BOOL keepGoing = TRUE;

	while (keepGoing) {
		grabLine = readText (fp);
		if (grabLine[0])
			readDir (grabLine);
		else
			keepGoing = FALSE;
		delete grabLine;
	}

	if (! finalFile) return errorBox (ERRORTYPE_PROJECTERROR, "Vital line missing from project", "finalfile", NULL);
//	if (! outputDirectory) return errorBox ("Vital line missing from project", "outputdir");

//	tempDirectory;
	return ((tempDirectory = grabEnv ("%temp%")) != NULL);
}

BOOL gotoSourceDirectory () {
	BOOL r = chdir (sourceDirectory);
	if (r) return errorBox (ERRORTYPE_SYSTEMERROR, "Can't move to source directory", sourceDirectory, NULL);
	return TRUE;
}

BOOL gotoTempDirectory () {
	BOOL r = chdir (tempDirectory);
	if (r) return errorBox (ERRORTYPE_SYSTEMERROR, "Can't move to temporary directory", tempDirectory, NULL);
	return TRUE;
}
/*
BOOL gotoOutputDirectory () {
	BOOL r = chdir (outputDirectory);
	if (r) return errorBox ("Can't move to output directory", outputDirectory);
	return TRUE;
}
*/
FILE * openFinalFile (char * addMe, char * mode) {
	char * fullName = joinStrings (finalFile, addMe);
	if (! fullName) return NULL;

	gotoSourceDirectory ();
	FILE * fp = fopen (fullName, mode);
	delete fullName;
	
	return fp;
}

#define MOUSE_1		1 << 2
#define MOUSE_2		1 << 4

int winMouseLookup[4] = {
		MOUSE_2,
		0,
		MOUSE_1,
		MOUSE_2 | MOUSE_1
};

void writeFinalData (FILE * mainFile, BOOL reg) {
	fprintf (mainFile, "SLUDGE");
	fputc (0, mainFile);
	fprintf (mainFile, "\r\nSLUDGE data file\r\nSLUDGE is (c) Hungry Software 2006\r\nThis data file must be run using the SLUDGE engine available at http://www.hungrysoftware.com/\r\n");
	fputc (0, mainFile);

	fputc (MAJOR_VERSION, mainFile);		// Major version
	fputc (MINOR_VERSION, mainFile);		// Minor version

	if (getRegSetting ("compilerVerbose")) {
		fputc (1, mainFile);
		writeDebugData (mainFile);
	} else {
		fputc (0, mainFile);
	}
	
	if (! reg) {
		startupHideLogo = FALSE;
		startupHideLoading = FALSE;
		startupInvisible = FALSE;
	}

	put2bytes (finalWidth, mainFile);
	put2bytes (finalHeight, mainFile);
	fputc ((reg											/*<< 0 */) +
		   (runFullScreen								  << 1) +
		   (winMouseLookup[winMouseImage & 3]				  ) +
		   (silent										  << 3) +
		   // 4 used for mouse image
		   (startupInvisible 							  << 5) +
		   (startupHideLogo								  << 6) +
		   (startupHideLoading							  << 7),
		   
		   mainFile);
		   
	fputc (frameRate, mainFile);
	writeString (myNameIs ? myNameIs : "", mainFile);

	// Now write the date and time of compilation...
	
	SYSTEMTIME systemTime;
	FILETIME fileTime;
	GetSystemTime (& systemTime);
	SystemTimeToFileTime (& systemTime, & fileTime);
	fwrite (& fileTime, sizeof (fileTime), 1, mainFile);
	
	// More bits
	
	writeString (dataFolderName, mainFile);
	addTranslationIDTable (mainFile);
	
	// Max anti-alias settings
	fputc (chrRenderingSettings.maxReadIni, mainFile);
	fputc (chrRenderingSettings.maxEnabled, mainFile);
	putFloat (chrRenderingSettings.maxSoftnessX / 16.f, mainFile);
	putFloat (chrRenderingSettings.maxSoftnessY / 16.f, mainFile);
	
	writeString ("okSoFar", mainFile);
}
