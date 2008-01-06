BOOL getRegSetting (char * settingName);
BOOL putRegSetting (char * settingName, BOOL onOff);
char * getRegString (char * settingName);
BOOL setRegString (char * settingName, char * writeMe);
int getRegInt (char * complete, int def);
BOOL putRegInt (char * complete, int num);
char * getFolderFromSpecialRegistryKey (char * dir, char * key);