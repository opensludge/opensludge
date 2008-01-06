struct statusBar {
	char * text;
	statusBar * next;
};

struct statusStuff {
	statusBar * firstStatusBar;
	unsigned short alignStatus;
	int litStatus;
	int statusX, statusY;
	int statusR, statusG, statusB;
	int statusLR, statusLG, statusLB;
};

void initStatusBar ();

void setStatusBar (char * txt);
void clearStatusBar ();
void addStatusBar ();
void killLastStatus ();
void statusBarColour (byte r, byte g, byte b);
void statusBarLitColour (byte r, byte g, byte b);
void setLitStatus (int i);
char * statusBarText ();
void positionStatus (int, int);
void drawStatusBar ();

// Load and save
BOOL loadStatusBars (FILE * fp);
void saveStatusBars (FILE * fp);

// For freezing
void restoreBarStuff (statusStuff * here);
statusStuff * copyStatusBarStuff (statusStuff * here);
