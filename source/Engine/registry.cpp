#include <windows.h>

#include "stringy.h"

char * getRegString (char * folder, char * settingName) {
	HKEY gotcha;
	int r;
	char * buff = NULL; 
	unsigned long si = 8;

	char * wholePath = joinStrings("Software\\", folder);

	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, wholePath, 0, KEY_READ, & gotcha) != ERROR_SUCCESS) {
		delete wholePath;
		return NULL;
	}

	delete wholePath;

	r = RegQueryValueEx (gotcha, settingName, NULL, NULL, NULL, & si);
	if (r == ERROR_SUCCESS) {
		buff = new char[si + 1];
		if (buff) {
			r = RegQueryValueEx (gotcha, settingName, NULL, NULL, (unsigned char *) buff, & si);
			if (r != ERROR_SUCCESS) {
				delete buff;
				buff = NULL;
			} else if (buff[0]) {
				int j = strlen (buff);
				if (buff[j - 1] == '\\' || buff[j - 1] == '/')
					buff[j - 1] = NULL;
			}
		}			
	}
	RegCloseKey (gotcha);
	return buff;
}

int getRegInt (char * complete, int def) {
	HKEY gotcha;
	int r;
	int grab;
	unsigned long si = 4;
	
	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Software\\Hungry Software\\SLUDGE\\Display", 0, KEY_READ, & gotcha) != ERROR_SUCCESS) return def;
	r = RegQueryValueEx (gotcha, complete, NULL, NULL, (unsigned char *) & grab, & si);
	RegCloseKey (gotcha);
		
	return (r == ERROR_SUCCESS) ? grab : def;
}
