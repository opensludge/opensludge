enum
{
	CHECKUSED_FUNCTIONS,
	CHECKUSED_GLOBALS,
	CHECKUSED_NUM
};

void checkUsedInit (int type, int n);
void setUsed (int type, int i);
void warnAboutUnused (int type, stringArray * temp, const char * preamble, stringArray * files);
