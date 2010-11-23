#define er "SLUDGE error"
void messageBox (const char *, const char *);
void messageBox (const char * tx2, int f, int f2);

enum
{
	ERRORTYPE_PROJECTWARNING,
	ERRORTYPE_PROJECTERROR,
	ERRORTYPE_SYSTEMERROR,		// Files etc.
	ERRORTYPE_INTERNALERROR,	// SHOULD NEVER HAPPEN!
	ERRORTYPE_NUM
};

void addComment (int errorType, const char *, const char * filename);
bool addComment (int errorType, const char * txt1, const char * txt2, const char * filename);
void clearComments ();

bool ask (const char * txt);

struct errorLinkToFile
{
	int errorType;
	char * overview;
	char * filename;
	char * fullText;
	int lineNumber;
	struct errorLinkToFile * next;
};

extern struct errorLinkToFile * errorList;
extern int numErrors;
