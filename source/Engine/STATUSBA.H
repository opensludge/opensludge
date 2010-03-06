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
void statusBarColour (unsigned char r, unsigned char g, unsigned char b);
void statusBarLitColour (unsigned char r, unsigned char g, unsigned char b);
void setLitStatus (int i);
char * statusBarText ();
void positionStatus (int, int);
void drawStatusBar ();

// Load and save
bool loadStatusBars (FILE * fp);
void saveStatusBars (FILE * fp);

// For freezing
void restoreBarStuff (statusStuff * here);
statusStuff * copyStatusBarStuff (statusStuff * here);
