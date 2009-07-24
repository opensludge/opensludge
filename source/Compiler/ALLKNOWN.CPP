#include "messbox.h"
#include "Splitter.hpp"

extern stringArray * functionNames;
extern stringArray * builtInFunc;
extern stringArray * typeDefFrom;
extern stringArray * objectTypeNames;
extern stringArray * allKnownFlags;
stringArray * * globalVarsP;

bool checkNotKnown (const char * thisName, const char * filename) {
	if (findElement (functionNames, thisName) != -1) return addComment (ERRORTYPE_PROJECTERROR, "Already the name of a function", thisName, filename);
	if (findElement (objectTypeNames, thisName) != -1) return addComment (ERRORTYPE_PROJECTERROR, "Already the name of an object type", thisName, filename);
	if (findElement (builtInFunc, thisName) != -1) return addComment (ERRORTYPE_PROJECTERROR, "Can't overload built-in functions", thisName, filename);
	if (findElement (typeDefFrom, thisName) != -1) return addComment (ERRORTYPE_PROJECTERROR, "Already the name of a typedef", thisName, filename);
	if (findElement (* globalVarsP, thisName) != -1) return addComment (ERRORTYPE_PROJECTERROR, "Already the name of a global variable", thisName, filename);
	if (findElement (allKnownFlags, thisName) != -1) return addComment (ERRORTYPE_PROJECTERROR, "Already the name of an object flag", thisName, filename);
	return true;
}

bool checkNotKnown2 (const char * thisName, const char * filename) {
//	if (findElement (functionNames, thisName) != -1) return addComment ("Already the name of a function", thisName);
//	if (findElement (objectTypeNames, thisName) != -1) return addComment ("Already the name of an object type", thisName);
	if (findElement (builtInFunc, thisName) != -1) return addComment (ERRORTYPE_PROJECTERROR, "Can't overload built-in functions", thisName, filename);
	if (findElement (typeDefFrom, thisName) != -1) return addComment (ERRORTYPE_PROJECTERROR, "Already the name of a typedef", thisName, filename);
//	if (findElement (* globalVarsP, thisName) != -1) return addComment ("Already the name of a global variable", thisName);
	return true;
}

void setGlobPointer (stringArray * * g) {
	globalVarsP = g;
}

bool checkValidName (const char * thisName, const char * error, const char * filename)
{
	if (thisName[0])
	{
		if ((thisName[0] >= 'a' && thisName[0] <= 'z') ||
			(thisName[0] >= 'A' && thisName[0] <= 'Z') ||
			(thisName[0] == '_'))
		{
			for (int i = 1;; i ++)
			{
				if (! thisName[i])
					return true;

				if ((thisName[i] < 'a' || thisName[i] > 'z') &&
					(thisName[i] < 'A' || thisName[i] > 'Z') &&
					(thisName[i] < '0' || thisName[i] > '9') &&
					(thisName[i] != '_'))
					break;
			}
		}
	}
	
	addComment (ERRORTYPE_PROJECTERROR, error, thisName, filename);
	return false;
}


