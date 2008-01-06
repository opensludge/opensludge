#define BUILTINDEBUG	0

enum builtReturn {BR_KEEP_AND_PAUSE, BR_NOCOMMENT, BR_CONTINUE, BR_PAUSE, BR_CALLAFUNC, BR_ALREADY_GONE};

BOOL failSecurityCheck (char * fn);
builtReturn callBuiltIn (int whichFunc, int numParams, loadedFunction * fun);
void builtInDebugTick();
