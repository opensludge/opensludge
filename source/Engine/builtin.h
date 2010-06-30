enum builtReturn {BR_KEEP_AND_PAUSE, BR_ERROR, BR_CONTINUE, BR_PAUSE, BR_CALLAFUNC, BR_ALREADY_GONE};

bool failSecurityCheck (char * fn);
builtReturn callBuiltIn (int whichFunc, int numParams, loadedFunction * fun);
