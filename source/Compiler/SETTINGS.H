struct chrRenderingSettingsStruct
{
	bool defEnabled;
	int defSoftnessX, defSoftnessY;
	
	bool maxEnabled, maxReadIni;
	int maxSoftnessX, maxSoftnessY;
};

extern chrRenderingSettingsStruct chrRenderingSettings;

bool getSourceDirFromName (char * filename);
bool readSettings (FILE * fp);
bool gotoSourceDirectory ();
bool gotoTempDirectory ();
//bool gotoOutputDirectory ();
FILE * openFinalFile (char *, char *);
void writeFinalData (FILE * mainFile, bool);
