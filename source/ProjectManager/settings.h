struct chrRenderingSettingsStruct
{
	bool defEnabled;
	int defSoftnessX, defSoftnessY;
	
	bool maxEnabled, maxReadIni;
	int maxSoftnessX, maxSoftnessY;
};

extern chrRenderingSettingsStruct chrRenderingSettings;


struct settingsStruct
{
	char *quitMessage;
	char *customIcon;
	char *runtimeDataFolder;
	
	char *finalFile;
	char *windowName;
	
	int winMouseImage;
	bool ditherImages;
	bool runFullScreen;
	bool startupShowLogo;
	bool startupShowLoading;
	bool startupInvisible;
	bool forceSilent;
	
	int	screenHeight;
	int screenWidth;
		
	int frameSpeed;
};

extern settingsStruct settings;
extern bool silent;
extern char * sourceDirectory;

bool readSettings (FILE * fp);
void writeSettings (FILE * fp);
void noSettings ();
void blankSettings ();
void chrRenderingSettingsFillDefaults(bool enable);


bool getSourceDirFromName (char * filename);
bool gotoSourceDirectory ();
bool gotoTempDirectory ();
bool gotoOutputDirectory ();
FILE * openFinalFile (char *, char *);
void writeFinalData (FILE * mainFile);
