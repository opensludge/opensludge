#include <windows.h>
#include "messbox.h"

BOOL getRegSetting (char * settingName) {
	HKEY gotcha;
	int r;
	unsigned char buff[10];
	unsigned long si = 8;

	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Software\\Hungry Software\\SLUDGE Compiler", 0, KEY_READ, & gotcha) != ERROR_SUCCESS) return FALSE;
	r = RegQueryValueEx (gotcha, settingName, NULL, NULL, buff, & si);
	RegCloseKey (gotcha);
	if (r != ERROR_SUCCESS) return FALSE;
//	messageBox (settingName, (char *) buff);
	return buff[0] == 'Y';
}

char * getRegString (char * settingName) {
	HKEY gotcha;
	int r;
	char * buff = NULL; 
	unsigned long si = 8;

	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Software\\Hungry Software\\SLUDGE Compiler", 0, KEY_READ, & gotcha) != ERROR_SUCCESS) return NULL;
//	errorBox ("Opened key...");
	r = RegQueryValueEx (gotcha, settingName, NULL, NULL, NULL, & si);
	if (r == ERROR_SUCCESS) {
//		errorBox ("Got", settingName);
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
//	} else {
//		errorBox ("Couldn't get", settingName);
	}
	RegCloseKey (gotcha);
	return buff;
}
