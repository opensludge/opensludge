enum halfCode {HALF_FIND, HALF_MARKER, HALF_DONE};

#include "csludge.h"

struct compilationSpace {
	int numMarkers;
	int numLines;
	int myNum;
	FILE * writeToFile;
	FILE * markerFile;
};

void initBuiltInFunc (BOOL reg);
BOOL startFunction (int num, int argNum, compilationSpace & theSpace, const char * theName, BOOL unfr, BOOL dbMe, const char *);
BOOL finishFunctionNew (compilationSpace & theSpace, stringArray * locals);
int protoFunction (char * funcName, char * fileName);
int defineFunction (char * funcName, char * args, char * sourceCode, BOOL unfr, BOOL dbMe, const char *);
BOOL compileSourceBlock (const char *, stringArray * &, compilationSpace &, const char * filename);
BOOL compileSourceLine (const char *, stringArray * &, compilationSpace &, stringArray * &, const char * filename);

void outputHalfCode (compilationSpace & theSpace, sludgeCommand theCommand, const char * stringy);
int outputMarkerCode (compilationSpace & theSpace, sludgeCommand theCommand);
void outputDoneCode (compilationSpace & theSpace, sludgeCommand theCommand, int value);
BOOL outdoorSub (char *, const char *);
void writeDebugData (FILE * mainFile);
void doDefines (char * fn, stringArray * &, stringArray * &);