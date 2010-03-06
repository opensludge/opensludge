#ifndef __LANGUAGE_H__
#define __LANGUAGE_H__

struct settingsStruct
{
	unsigned int languageID;
	unsigned int numLanguages;
	bool userFullScreen;
	unsigned int refreshRate;
	int antiAlias;
	bool fixedPixels;
	bool noStartWindow;
};

struct cmdlineSettingsStruct
{
	bool languageSet;
	unsigned int languageID;
	bool fullscreenSet;
	bool userFullScreen;
};

void readIniFile (char * filename);
void saveIniFile (char * filename);
int getLanguageForFileB ();
void makeLanguageTable (FILE * table);

#endif
