#include <stdio.h>
#include <string.h>
#include "stringy.h"
#include "newfatal.h"
#include "moreio.h"
#include "language.h"
#include "version.h"

int *languageTable;
char **languageName;
settingsStruct gameSettings;

unsigned int stringToInt (char * s) {
	int i = 0;
	bool negative = false;
	for (;;) {
		if (*s >= '0' && *s <= '9') {
			i *= 10;
			i += *s - '0';
			s ++;
		} else if (*s == '-') {
			negative = ! negative;
			s++;
		} else {
			if (negative)
				return -i;
			return i;
		}
	}
}


char * getPrefsFilename (char * filename) {
	// Yes, this trashes the original string, but
	// we also free it at the end (warning!)...

	int n, i;

	n = strlen (filename);


	if (n > 4 && filename[n-4] == '.') {
		filename[n-4] = 0;
	}

	char * f = filename;
	for (i = 0; i<n; i++) {
#ifdef _WIN32
		if (filename[i] == '\\')
#else
		if (filename[i] == '/')
#endif
			f = filename + i + 1;
	}

	char * joined = joinStrings (f, ".ini");

	delete filename;
	filename = NULL;
	return joined;
}

void readIniFile (char * filename) {
	char * langName = getPrefsFilename (copyString (filename));

	FILE * fp = fopen (langName, "rb");

	gameSettings.languageID = 0;
	gameSettings.userFullScreen = true;
	gameSettings.refreshRate = 0;
	gameSettings.antiAlias = 1;
	gameSettings.fixedPixels = false;
	gameSettings.noStartWindow = false;
	gameSettings.debugMode = false;

	delete langName;
	langName = NULL;

	if (fp) {
		char lineSoFar[257] = "";
		char secondSoFar[257] = "";
		unsigned char here = 0;
		char readChar = ' ';
		bool keepGoing = true;
		bool doingSecond = false;

		do {
			readChar = fgetc (fp);
			if (feof (fp)) {
				readChar = '\n';
				keepGoing = false;
			}
			switch (readChar) {
				case '\n':
				case '\r':
				if (doingSecond) {
					if (strcmp (lineSoFar, "LANGUAGE") == 0)
					{
						gameSettings.languageID = stringToInt (secondSoFar);
					}
					else if (strcmp (lineSoFar, "WINDOW") == 0)
					{
						gameSettings.userFullScreen = ! stringToInt (secondSoFar);
					}
					else if (strcmp (lineSoFar, "REFRESH") == 0)
					{
						gameSettings.refreshRate = stringToInt (secondSoFar);
					}
					else if (strcmp (lineSoFar, "ANTIALIAS") == 0)
					{
						gameSettings.antiAlias = stringToInt (secondSoFar);
					}
					else if (strcmp (lineSoFar, "FIXEDPIXELS") == 0)
					{
						gameSettings.fixedPixels = stringToInt (secondSoFar);
					}
					else if (strcmp (lineSoFar, "NOSTARTWINDOW") == 0)
					{
						gameSettings.noStartWindow = stringToInt (secondSoFar);
					}
					else if (strcmp (lineSoFar, "DEBUGMODE") == 0)
					{
						gameSettings.debugMode = stringToInt (secondSoFar);
					}
				}
				here = 0;
				doingSecond = false;
				lineSoFar[0] = 0;
				secondSoFar[0] = 0;
				break;

				case '=':
				doingSecond = true;
				here = 0;
				break;

				default:
				if (doingSecond) {
					secondSoFar[here ++] = readChar;
					secondSoFar[here] = 0;
				} else {
					lineSoFar[here ++] = readChar;
					lineSoFar[here] = 0;
				}
				break;
			}
		} while (keepGoing);

		fclose (fp);
	}
}

void saveIniFile (char * filename) {
	char * langName = getPrefsFilename (copyString (filename));
	FILE * fp = fopen (langName, "wb");
	delete langName;

	fprintf (fp, "LANGUAGE=%d\n", gameSettings.languageID);
	fprintf (fp, "WINDOW=%d\n", ! gameSettings.userFullScreen);
	fprintf (fp, "ANTIALIAS=%d\n", gameSettings.antiAlias);
	fprintf (fp, "FIXEDPIXELS=%d\n", gameSettings.fixedPixels);
	fprintf (fp, "NOSTARTWINDOW=%d\n", gameSettings.noStartWindow);
	fprintf (fp, "DEBUGMODE=%d\n", gameSettings.debugMode);

	fclose (fp);
}

void makeLanguageTable (FILE * table)
{
	languageTable = new int[gameSettings.numLanguages + 1];
	languageName = new char*[gameSettings.numLanguages + 1];

	for (unsigned int i = 0; i <= gameSettings.numLanguages; i ++) {
		languageTable[i] = i ? get2bytes (table) : 0;
		languageName[i] = 0;
		if (gameVersion >= VERSION(2,0)) {
			if (gameSettings.numLanguages)
				languageName[i] = readString (table);
		}
	}
}

int getLanguageForFileB ()
{
	int indexNum = -1;

	for (unsigned int i = 0; i <= gameSettings.numLanguages; i ++) {
		if (languageTable[i] == gameSettings.languageID) indexNum = i;
	}

	return indexNum;
}
