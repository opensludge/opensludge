#include "variable.h"
#include "csludge.h"
#include "language.h"

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
	BOOL						returnSomething, isSpeech, unfreezable, cancelMe;
	byte						freezerLevel;
};

struct inputType {
	BOOL leftClick, rightClick, justMoved, leftRelease, rightRelease;
	int mouseX, mouseY, keyPressed;
};

BOOL initSludge (char *, iniStuff & iniFileSettings);
void sludgeDisplay ();
int startNewFunctionNum (unsigned int, unsigned int, loadedFunction *, variableStack * &, BOOL = TRUE);
BOOL handleInput ();
void restartFunction (loadedFunction * fun);
BOOL loadFunctionCode (loadedFunction * newFunc);
void loadHandlers (FILE * fp);
void saveHandlers (FILE * fp);
void finishFunction (loadedFunction * fun);
void abortFunction (loadedFunction * fun);
FILE * openAndVerify (char * filename, char extra1, char extra2, char * er, int & fileVersion);
void freezeSubs ();
void unfreezeSubs ();
void completeTimers ();
void killSpeechTimers ();
int cancelAFunction (int funcNum, loadedFunction * myself, BOOL & killedMyself);
