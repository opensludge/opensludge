#ifndef __LANGUAGE_H__
#define __LANGUAGE_H__

struct iniStuff
{
	unsigned int languageID;
	BOOL userFullScreen;
	unsigned int refreshRate;
	int antiAlias;
};

void readIniFile (char * filename, iniStuff & iniFileSettings);
int getLanguageForFileB (int total, FILE * table, unsigned int languageID);

#endif