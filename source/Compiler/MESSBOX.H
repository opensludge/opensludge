enum
{
	ERRORTYPE_PROJECTWARNING,
	ERRORTYPE_PROJECTERROR,
	ERRORTYPE_SYSTEMERROR,		// Files etc.
	ERRORTYPE_INTERNALERROR,	// SHOULD NEVER HAPPEN!
	ERRORTYPE_NUM
};

void addComment (int errorType, const char *, const char * filename);
BOOL errorBox (int errorType, const char *, const char *, const char * filename);
//BOOL errorBox (const char * txt1, unsigned int i);
void userClickedErrorLine (int n);
