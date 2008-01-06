struct chrRenderingSettingsStruct
{
	bool defEnabled;
	int defSoftnessX, defSoftnessY;
	
	bool maxEnabled, maxReadIni;
	int maxSoftnessX, maxSoftnessY;
};

extern chrRenderingSettingsStruct chrRenderingSettings;

BOOL getSourceDirFromName (char * filename);
BOOL readSettings (FILE * fp);
BOOL gotoSourceDirectory ();
BOOL gotoTempDirectory ();
//BOOL gotoOutputDirectory ();
FILE * openFinalFile (char *, char *);
void writeFinalData (FILE * mainFile, BOOL);
