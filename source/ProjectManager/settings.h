struct chrRenderingSettingsStruct
{
	bool defEnabled;
	int defSoftnessX, defSoftnessY;
	
	bool maxEnabled, maxReadIni;
	int maxSoftnessX, maxSoftnessY;
};

BOOL readSettings (FILE * fp);
void writeSettings (FILE * fp);
void noSettings ();
void blankSettings ();
void chrRenderingSettingsFillDefaults(bool enable);
