#include <stdio.h>
#include "splitter.hpp"

enum halfCode {HALF_FIND, HALF_MARKER, HALF_DONE};

#include "csludge.h"

struct compilationSpace {
	int numMarkers;
	int numLines;
	int myNum;
	FILE * writeToFile;
	FILE * markerFile;
};

void initBuiltInFunc ();
bool startFunction (int num, int argNum, compilationSpace & theSpace, const char * theName, bool unfr, bool dbMe, const char *);
bool finishFunctionNew (compilationSpace & theSpace, stringArray * locals);
int protoFunction (const char * funcName, const char * fileName);
int defineFunction (char * funcName, const char * args, char * sourceCode, bool unfr, bool dbMe, const char *);
bool compileSourceBlock (const char *, stringArray * &, compilationSpace &, const char * filename);
bool compileSourceLine (const char *, stringArray * &, compilationSpace &, stringArray * &, const char * filename, unsigned int fileline);

void outputHalfCode (compilationSpace & theSpace, sludgeCommand theCommand, const char * stringy);
int outputMarkerCode (compilationSpace & theSpace, sludgeCommand theCommand);
void outputDoneCode (compilationSpace & theSpace, sludgeCommand theCommand, int value);
bool outdoorSub (char *, const char *);
void writeDebugData (FILE * mainFile);
void doDefines (char * fn, stringArray * &, stringArray * &);
