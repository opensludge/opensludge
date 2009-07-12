#ifndef __LANGUAGE_H__
#define __LANGUAGE_H__
/*
#if defined(_WIN32)
#include "windef.h"
#endif
*/

struct settingsStruct
{
	unsigned int languageID;
	unsigned int numLanguages;
	bool userFullScreen;
	unsigned int refreshRate;
	int antiAlias;
};

void readIniFile (char * filename);
int getLanguageForFileB ();
void makeLanguageTable (FILE * table);

#endif
