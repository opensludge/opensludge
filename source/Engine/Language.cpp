#include <stdio.h>
#include <string.h>
#include "stringy.h"
#include "newfatal.h"
#include "moreio.h"
#include "language.h"
#include "version.h"

int *languageTable;
char **languageName;

unsigned int stringToInt (char * s) {
	int i = 0;
	for (;;) {
		if (*s >= '0' && *s <= '9') {
			i *= 10;
			i += *s - '0';
			s ++;
		} else {
			return i;
		}
	}
}

char * getPrefsFilename (char * filename) {
	// Yes, this trashes the original string, but
	// sod it, we don't use it afterwards...

	int n;

	n = strlen (filename);

	if (n > 4 && filename[n-4] == '.') {
		filename[n-4] = NULL;
	}
	
	char * joined = joinStrings (filename, ".ini");
	
	delete filename;
	filename = NULL;
	return joined;
}

extern settingsStruct gameSettings;

void readIniFile (char * filename) {
	char * langName = getPrefsFilename (copyString (filename));
	FILE * fp = fopen (langName, "rb");

	gameSettings.languageID = 0;
	gameSettings.userFullScreen = true;
	gameSettings.refreshRate = 0;
	gameSettings.antiAlias = -1;

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
					if (strcmp (lineSoFar, "REFRESH") == 0)
					{
						gameSettings.refreshRate = stringToInt (secondSoFar);
					}
					if (strcmp (lineSoFar, "ANTIALIAS") == 0)
					{
						gameSettings.antiAlias = stringToInt (secondSoFar);
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

void makeLanguageTable (FILE * table)
{
	languageTable = new int[gameSettings.numLanguages];
	languageName = new char*[gameSettings.numLanguages];

	for (int i = 0; i <= gameSettings.numLanguages; i ++) {
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

	for (int i = 0; i <= gameSettings.numLanguages; i ++) {
		if (languageTable[i] == gameSettings.languageID) indexNum = i;
	}

	return indexNum;
}
