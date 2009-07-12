bool getRegSetting (char * settingName);
bool putRegSetting (char * settingName, bool onOff);
char * getRegString (char * settingName);
bool setRegString (char * settingName, char * writeMe);
int getRegInt (char * complete, int def);
bool putRegInt (char * complete, int num);
char * getFolderFromSpecialRegistryKey (char * dir, char * key);