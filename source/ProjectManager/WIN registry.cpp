#ifdef WIN32

#include <windows.h>
#include "messbox.h"

char * getFolderFromSpecialRegistryKey (char * dir, char * settingName) {
	HKEY gotcha;
	int r;
	char * buff = NULL;
	unsigned long si = 8;

	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, dir, 0, KEY_READ, & gotcha) != ERROR_SUCCESS) return NULL;
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

bool getRegSetting (char * settingName) {
	HKEY gotcha;
	int r;
	unsigned char buff[10];
	unsigned long si = 8;
	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Software\\Hungry Software\\SLUDGE", 0, KEY_READ, & gotcha) != ERROR_SUCCESS) {
		return false;
	}
	r = RegQueryValueEx (gotcha, settingName, NULL, NULL, buff, & si);
	RegCloseKey (gotcha);
	if (r != ERROR_SUCCESS) {
		//		if (r == ERROR_MORE_DATA)
		//			errorBox ("It reckons there should be more data!", settingName);
		//		errorBox ("Can't open registry thingy in Software\\Hungry Software\\SLUDGE", settingName);
		return false;
	}
	if (buff[0] == 'Y') return true;
	return false;
}

bool putRegSetting (char * settingName, bool onOff) {
	HKEY gotcha;
	int r;
	unsigned char buff[2] = "-";
	buff[0] = onOff ? 'Y' : 'N';
	
	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Software\\Hungry Software\\SLUDGE", 0, KEY_WRITE, & gotcha) != ERROR_SUCCESS)
		return errorBox ("Can't open SLUDGE registry key...", settingName);
	
	r = RegSetValueEx (gotcha, settingName, NULL, REG_SZ, buff, 2);
	
	RegCloseKey (gotcha);
	if (r != ERROR_SUCCESS) return errorBox ("Can't update SLUDGE registry element...", settingName);
	return true;
}

bool setRegString (char * settingName, char * writeMe) {
	HKEY gotcha;
	int r;
	
	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Software\\Hungry Software\\SLUDGE", 0, KEY_WRITE, & gotcha) != ERROR_SUCCESS)
		return errorBox ("Can't open SLUDGE registry key...", settingName);
	
	r = RegSetValueEx (gotcha, settingName, NULL, REG_SZ, (unsigned char *) writeMe, strlen (writeMe) + 1);
	
	RegCloseKey (gotcha);
	if (r != ERROR_SUCCESS) return errorBox ("Can't update SLUDGE registry element...", settingName);
	return true;
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

int getRegInt (char * complete, int def) {
	HKEY gotcha;
	int r;
	int grab;
	unsigned long si = 4;
	
	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Software\\Hungry Software\\SLUDGE Compiler", 0, KEY_READ, & gotcha) != ERROR_SUCCESS) return def;
	r = RegQueryValueEx (gotcha, complete, NULL, NULL, (unsigned char *) & grab, & si);
	RegCloseKey (gotcha);
		
	return (r == ERROR_SUCCESS) ? grab : def;
}

bool putRegInt (char * complete, int num) {
	HKEY gotcha;
	int r;
	
	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Software\\Hungry Software\\SLUDGE Compiler", 0, KEY_WRITE, & gotcha) != ERROR_SUCCESS)
		return errorBox ("Can't open SLUDGE registry key", complete);
	
	r = RegSetValueEx (gotcha, complete, NULL, REG_DWORD, (unsigned char *) & num, 4);

	RegCloseKey (gotcha);
	return (r != ERROR_SUCCESS)
		?		errorBox ("Can't update registry element", complete)
		:		true;
}

#endif