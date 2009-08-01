#ifndef SLUDGER_H
#define SLUDGER_H

#include "variable.h"
#include "csludge.h"
#include "language.h"

#ifndef _WIN32
typedef struct _FILETIME {
	unsigned long dwLowDateTime;
	unsigned long dwHighDateTime;
} FILETIME;
#endif


struct eventHandlers {
	int leftMouseFunction;
	int leftMouseUpFunction;
	int rightMouseFunction;
	int rightMouseUpFunction;
	int moveMouseFunction;
	int focusFunction;
	int spaceFunction;
};

struct lineOfCode {
	sludgeCommand				theCommand;
	long						param;
};

struct loadedFunction {
	int							originalNumber;
	lineOfCode *				compiledLines;
	int							numLocals, timeLeft, numArgs;
	variable *					localVars;
	variableStack *				stack;
	variable					reg;
	unsigned int				runThisLine;
	loadedFunction *			calledBy;
	loadedFunction *			next;
	bool						returnSomething, isSpeech, unfreezable, cancelMe;
	unsigned char				freezerLevel;
};

struct inputType {
	bool leftClick, rightClick, justMoved, leftRelease, rightRelease;
	int mouseX, mouseY, keyPressed;
};

bool initSludge (char *);
void sludgeDisplay ();
int startNewFunctionNum (unsigned int, unsigned int, loadedFunction *, variableStack * &, bool = true);
bool handleInput ();
void restartFunction (loadedFunction * fun);
bool loadFunctionCode (loadedFunction * newFunc);
void loadHandlers (FILE * fp);
void saveHandlers (FILE * fp);
void finishFunction (loadedFunction * fun);
void abortFunction (loadedFunction * fun);
FILE * openAndVerify (char * filename, char extra1, char extra2, char * er, int & fileVersion);
void freezeSubs ();
void unfreezeSubs ();
void completeTimers ();
void killSpeechTimers ();
int cancelAFunction (int funcNum, loadedFunction * myself, bool & killedMyself);

#endif
