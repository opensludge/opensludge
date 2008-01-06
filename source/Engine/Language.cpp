#include <stdio.h>
#include "stringy.h"
#include "newfatal.h"
#include "moreio.h"
#include "language.h"

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
	
	int n = 0, putBack = -1, evilFuckyHack = 0;
	do {
		evilFuckyHack = 0;
		if (filename[n] == '.') {
			if (putBack != -1) filename[putBack] = '.';
			putBack = n;
			filename[n] = NULL;
			evilFuckyHack = 1;
		}
	} while (filename[n ++] || evilFuckyHack);

	char * joined = joinStrings (filename, ".ini");

	delete filename;
	return joined;
}

/*
void ouch (char * o)
{
	FILE * fp = fopen ("ouch.txt", "at");
	fprintf (fp, "%s\n", o);
	fclose (fp);
}*/

void readIniFile (char * filename, iniStuff & iniFileSettings) {
	char * langName = getPrefsFilename (copyString (filename));
	FILE * fp = fopen (langName, "rt");

	iniFileSettings.languageID = 0;
	iniFileSettings.userFullScreen = TRUE;
	iniFileSettings.refreshRate = 0;
	iniFileSettings.antiAlias = -1;

//	ouch ("Original filename:");
//	ouch (filename);
//	ouch (".ini filename:");
//	ouch (langName);
//	ouch (fp ? "open" : "not open");

	delete langName;

	if (fp) {
		char lineSoFar[257] = "";
		char secondSoFar[257] = "";
		unsigned char here = 0;
		char readChar = ' ';
		BOOL keepGoing = TRUE;
		BOOL doingSecond = FALSE;
		
		do {
			readChar = fgetc (fp);
			if (feof (fp)) {
				readChar = '\n';
				keepGoing = FALSE;
			}
			switch (readChar) {
				case '\n':
				case '\r':
				if (doingSecond) {
					if (strcmp (lineSoFar, "LANGUAGE") == 0)
					{
						iniFileSettings.languageID = stringToInt (secondSoFar);
					}
					else if (strcmp (lineSoFar, "WINDOW") == 0)
					{
						iniFileSettings.userFullScreen = ! stringToInt (secondSoFar);
					}
					if (strcmp (lineSoFar, "REFRESH") == 0)
					{
						iniFileSettings.refreshRate = stringToInt (secondSoFar);
					}
					if (strcmp (lineSoFar, "ANTIALIAS") == 0)
					{
						iniFileSettings.antiAlias = stringToInt (secondSoFar);
					}
				}
				here = 0;
				doingSecond = FALSE;
				lineSoFar[0] = NULL;
				secondSoFar[0] = NULL;
				break;
				
				case '=':
				doingSecond = TRUE;
				here = 0;
				break;
				
				default:
				if (doingSecond) {
					secondSoFar[here ++] = readChar;
					secondSoFar[here] = NULL;
				} else {
					lineSoFar[here ++] = readChar;
					lineSoFar[here] = NULL;
				}
				break;
			}
		} while (keepGoing);
		
		fclose (fp);
	}
}
	
int getLanguageForFileB (int total, FILE * table, unsigned int languageID)
{
	int indexNum = -1;

	for (int i = 0; i <= total; i ++) {
		int theID = i ? get2bytes (table) : 0;
#if 0
		char buff[100];
		sprintf (buff, "%i languages... looking for %i... ID[%i] = %i", total + 1, languageID, i, theID);
		warning (buff);
#endif
		if (theID == languageID) indexNum = i;
	}
	
#if 0
	char buff[100];
	sprintf (buff, "returning %i", indexNum);
	warning (buff);
#endif

	return indexNum;
}
