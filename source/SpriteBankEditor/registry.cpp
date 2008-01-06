#include <windows.h>
#include "messbox.h"

BOOL getRegSetting (char * settingName) {
	HKEY gotcha;
	int r;
	unsigned char buff[10];
	unsigned long si = 8;
	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Software\\Hungry Software\\SLUDGE", 0, KEY_READ, & gotcha) != ERROR_SUCCESS) {
//		errorBox ("Can't open", "Software\\Hungry Software\\SLUDGE");
		return FALSE;
	}
	r = RegQueryValueEx (gotcha, settingName, NULL, NULL, buff, & si);
	RegCloseKey (gotcha);
	if (r != ERROR_SUCCESS) {
//		if (r == ERROR_MORE_DATA)
//			errorBox ("It reckons there should be more data!", settingName);
//		errorBox ("Can't open registry thingy in Software\\Hungry Software\\SLUDGE", settingName);
		return FALSE;
	}
//	errorBox ("Queried", settingName);
//	errorBox ("Got", (char *) buff);
	if (buff[0] == 'Y') return TRUE;
	return FALSE;
}

BOOL putRegSetting (char * settingName, BOOL onOff) {
	HKEY gotcha;
	int r;
	unsigned char buff[2] = "-";
	buff[0] = onOff ? 'Y' : 'N';
	
	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Software\\Hungry Software\\SLUDGE", 0, KEY_WRITE, & gotcha) != ERROR_SUCCESS)
		return errorBox ("Can't open SLUDGE registry key...", settingName);
	
	r = RegSetValueEx (gotcha, settingName, NULL, REG_SZ, buff, 2);
	
	RegCloseKey (gotcha);
	if (r != ERROR_SUCCESS) return errorBox ("Can't update SLUDGE registry element...", settingName);
	return TRUE;
}

BOOL setRegString (char * settingName, char * writeMe) {
	HKEY gotcha;
	int r;
	
	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Software\\Hungry Software\\SLUDGE", 0, KEY_WRITE, & gotcha) != ERROR_SUCCESS)
		return errorBox ("Can't open SLUDGE registry key...", settingName);
	
	r = RegSetValueEx (gotcha, settingName, NULL, REG_SZ, (unsigned char *) writeMe, strlen (writeMe) + 1);
	
	RegCloseKey (gotcha);
	if (r != ERROR_SUCCESS) return errorBox ("Can't update SLUDGE registry element...", settingName);
	return TRUE;
}

char * getRegString (char * settingName) {
	HKEY gotcha;
	int r;
	char * buff = NULL; 
	unsigned long si = 8;

	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Software\\Hungry Software\\SLUDGE", 0, KEY_READ, & gotcha) != ERROR_SUCCESS) return NULL;
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
