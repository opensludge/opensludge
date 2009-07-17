#define er "SLUDGE error"
void messageBox (const char *, const char *);
void messageBox (const char * tx2, int f, int f2);

bool errorBox (const char *, const char *);
bool errorBox (const char * txt1, unsigned int number);
bool errorBox (int errorType, const char *, const char *, const char * filename);
bool errorBox (const char * txt1, unsigned int number, unsigned int n2);

enum
{
	ERRORTYPE_PROJECTWARNING,
	ERRORTYPE_PROJECTERROR,
	ERRORTYPE_SYSTEMERROR,		// Files etc.
	ERRORTYPE_INTERNALERROR,	// SHOULD NEVER HAPPEN!
	ERRORTYPE_NUM
};

void addComment (int errorType, const char *, const char * filename);
void userClickedErrorLine (int n);

bool ask (const char * txt);
