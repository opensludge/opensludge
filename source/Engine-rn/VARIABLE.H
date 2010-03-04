#ifndef VAR_HEADER
#define VAR_HEADER

enum variableType {SVT_NULL, SVT_INT, SVT_FUNC, SVT_STRING,
				   SVT_BUILT, SVT_FILE, SVT_STACK,
				   SVT_OBJTYPE, SVT_ANIM, SVT_COSTUME,
				   SVT_FASTARRAY, SVT_NUM_TYPES};

struct fastArrayHandler {
	struct variable *			fastVariables;
	int							size;
	int							timesUsed;
};

struct stackHandler {
	struct variableStack *		first;
	struct variableStack *		last;
	int							timesUsed;
};

union variableData {
	signed int					intValue;
	char *						theString;
	stackHandler *				theStack;
	struct personaAnimation *	animHandler;
	struct persona *    		costumeHandler;
	fastArrayHandler *			fastArray;
};

struct variable {
	variableType				varType;
	variableData				varData;
};

struct variableStack {
	variable					thisVar;
	variableStack	* 			next;
};

// Initialisation

#define initVarNew(thisVar) 	thisVar.varType = SVT_NULL

// Setting variables

void setVariable (variable & thisVar, variableType vT, int value);
bool copyVariable (const variable & from, variable & to);
bool loadStringToVar (variable & thisVar, int value);
void newAnimationVariable (variable & thisVar, struct personaAnimation * i);
void newCostumeVariable (variable & thisVar, struct persona * i);
void makeTextVar (variable & thisVar, char * txt);
void addVariablesInSecond (variable & var1, variable & var2);
void compareVariablesInSecond (const variable & var1, variable & var2);

// Misc.

void unlinkVar (variable & thisVar);
char * getNumberedString (int value);
char * getTextFromAnyVar (const variable & from);
struct persona * getCostumeFromVar (variable & thisVar);
struct personaAnimation * getAnimationFromVar (variable & thisVar);
bool getBoolean (const variable & from);
bool getValueType (int & toHere, variableType vT, const variable & v);

// Stacky stuff

bool addVarToStack (const variable & va, variableStack * & thisStack);
bool addVarToStackQuick (variable & va, variableStack * & thisStack);
void trimStack (variableStack * & stack);
int deleteVarFromStack (const variable & va, variableStack * & thisStack, bool allOfEm = false);
variableStack * stackFindLast (variableStack * hunt);
bool copyStack (const variable & from, variable & to);
int stackSize (const stackHandler * me);
bool stackSetByIndex (variableStack *, unsigned int, const variable &);
variable * stackGetByIndex (variableStack *, unsigned int);
bool getSavedGamesStack (stackHandler * sH, char * ext);

bool makeFastArrayFromStack (variable & to, const stackHandler * stacky);
bool makeFastArraySize (variable & to, int size);
variable * fastArrayGetByIndex (fastArrayHandler * vS, unsigned int theIndex);


#define DEBUG_STACKINESS	0

#if DEBUG_STACKINESS
#define stackDebug(params) {							\
	FILE * stackfp = fopen ("stackout.txt", "at");		\
	fprintf params;										\
	fclose (stackfp);									\
}
#else
#define stackDebug(a)  {}
#endif


#endif

