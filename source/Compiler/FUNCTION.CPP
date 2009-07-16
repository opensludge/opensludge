//#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "typedef.h"
#include "Splitter.hpp"
#include "function.h"
#include "settings.h"
#include "tokens.h"
#include "moreio.h"
#include "messbox.h"
#include "winterfa.h"
#include "percbar.h"
#include "wintext.h"
#include "allknown.h"

//extern HWND compWin;
extern char * inThisClass;

// Sludge commands

// Permanent things for finding mathematical symbols
// (Higher precedence towards the end)

const char mathsChar[] = {'=', CHAR_PLUS_EQ, CHAR_MINUS_EQ, CHAR_MULT_EQ, CHAR_DIV_EQ, CHAR_MOD_EQ, '?', CHAR_AND, CHAR_OR, CHAR_EQUALS,
						  CHAR_NOT_EQ, '<', CHAR_LESS_EQUAL, '>', CHAR_MORE_EQUAL, '+', '-', '*', '/',
						  '%', CHAR_INCREMENT, CHAR_DECREMENT};

enum mathsFunc {MATHS_SET, MATHS_PLUS_EQ, MATHS_MINUS_EQ, MATHS_MULT_EQ, MATHS_DIV_EQ, MATHS_MOD_EQ, MATHS_QMARK, MATH_AND, MATH_OR, MATH_EQUALS,
				MATHS_NOT_EQ, MATHS_LESSTHAN, MATHS_LESS_EQUAL, MATHS_MORETHAN, MATHS_MORE_EQUAL,
				MATHS_PLUS, MATHS_MINUS, MATHS_MULT, MATHS_DIVIDE,
				MATHS_MODULUS, MATHS_INCREMENT, MATHS_DECREMENT,
				numMathsFunc};

sludgeCommand math2Sludge[] = {SLU_UNKNOWN, SLU_PLUS, SLU_MINUS, SLU_MULT, SLU_DIVIDE, SLU_MODULUS, SLU_UNKNOWN, SLU_AND, SLU_OR, SLU_EQUALS,
							   SLU_NOT_EQ, SLU_LESSTHAN, SLU_LESS_EQUAL, SLU_MORETHAN, SLU_MORE_EQUAL, SLU_PLUS,
							   SLU_MINUS, SLU_MULT, SLU_DIVIDE, SLU_MODULUS, SLU_UNKNOWN, SLU_UNKNOWN};

// Build these as we go along

stringArray * functionNames = NULL;
stringArray * functionFiles = NULL;
stringArray * builtInFunc = NULL;
stringArray * allFileHandles = NULL;

stringArray * typeDefFrom = NULL;
stringArray * typeDefTo = NULL;

// From (St.) elsewhere

int numFilesFound, numStringsFound;
extern int subNum;

// The functions

void addMarker (compilationSpace & theSpace, int theMarker) {
	put2bytes (theMarker, theSpace.markerFile);
	put2bytes (theSpace.numLines, theSpace.markerFile);
//	printf ("%i %i\n", theMarker, theSpace.numLines);
}

void outputHalfCode (compilationSpace & theSpace, sludgeCommand theCommand, const char * stringy) {
	fputc (HALF_FIND,		theSpace.writeToFile);
	fputc (theCommand,		theSpace.writeToFile);
	writeString (stringy,	theSpace.writeToFile);
	
	theSpace.numLines ++;
}

int outputMarkerCode (compilationSpace & theSpace, sludgeCommand theCommand) {
	fputc (HALF_MARKER,						theSpace.writeToFile);
	fputc (theCommand,						theSpace.writeToFile);
	put2bytes (theSpace.numMarkers,			theSpace.writeToFile);
	
	theSpace.numLines ++;
	theSpace.numMarkers ++;
	
	return theSpace.numMarkers - 1;
}

void outputDoneCode (compilationSpace & theSpace, sludgeCommand theCommand, int value) {
	fputc (HALF_DONE,		theSpace.writeToFile);
	fputc (theCommand,		theSpace.writeToFile);
	put2bytes (value,		theSpace.writeToFile);
	
	theSpace.numLines ++;
}

bool startFunction (int num,
					int argNum,
					compilationSpace & theSpace,
					const char * theName, bool unfr, bool dbMe, const char * fileName) {
					
	char file1[30], file2[30];
	
	setWindowText (COM_ITEMTEXT, theName);
	sprintf (file1, "_F%05i.dat", num);
	sprintf (file2, "_F%05iM.dat", num);
	
	if (! gotoTempDirectory ()) return false;
		
	theSpace.writeToFile = fopen (file1, "wb");
	theSpace.markerFile = fopen (file2, "wb");
	theSpace.numMarkers = 0;
	theSpace.numLines = 0;
	theSpace.myNum = num;

	if (! theSpace.writeToFile) {
		errorBox (ERRORTYPE_SYSTEMERROR, "Can't write to temporary file", file1, NULL);
		return false;
	}
	if (! theSpace.markerFile) {
		errorBox (ERRORTYPE_SYSTEMERROR, "Can't write to temporary file", file2, NULL);
		return false;
	}

	writeString (theName, theSpace.writeToFile);
	writeString (fileName, theSpace.writeToFile);
	writeString (inThisClass, theSpace.writeToFile);
	fputc (unfr, theSpace.writeToFile);
	fputc (dbMe, theSpace.writeToFile);
//	if (unfr) errorBox ("Set unfreezable flag in function header", theName);
	fputc (argNum, theSpace.writeToFile);

	return true;
}

bool finishFunctionNew (compilationSpace & theSpace, stringArray * localVars) {
	FILE * theFileHandle;
	char filename[20];
	int localNum = countElements(localVars);

//	errorBox ("Calling finishFunction", "");	
	outputDoneCode (theSpace, SLU_LOAD_NULL, 0);
	outputDoneCode (theSpace, SLU_RETURN, 0);

	for (int i = 0; i < localNum; i ++)
	{
		writeString (localVars->string, theSpace.writeToFile);
		localVars = localVars->next;
	}
	
	fclose (theSpace.writeToFile);
	fclose (theSpace.markerFile);

	sprintf (filename, "_F%05iN.DAT", theSpace.myNum);
	
	theFileHandle = fopen (filename, "wb");
	if (! theFileHandle) return false;

	fputc (localNum, theFileHandle);
	fputc (theSpace.numMarkers, theFileHandle);
	put2bytes (theSpace.numLines, theFileHandle);
	
	fclose (theFileHandle);
	
//	errorBox ("Ending finishFunction", "");	
	return true;
}

int protoFunction (char * funcName, char * fileName) {
	addToStringArray (functionFiles, fileName);
	return findOrAdd (functionNames, funcName);
}

bool handleIf (char * theIf, stringArray * & localVars, compilationSpace & theSpace, stringArray * & theRest, const char * filename) {
	stringArray * getCondition, * getElse;
	int endMarker;

	getCondition = splitString (theIf, ')', ONCE);
	if (! trimStart (getCondition -> string, '(')) {
		return errorBox (ERRORTYPE_PROJECTERROR, "Bad if", theIf, filename);
	}

	if (! compileSourceLine (getCondition -> string, localVars, theSpace, nullArray, filename)) return false;

	if (! destroyFirst (getCondition)) {
		return errorBox (ERRORTYPE_PROJECTERROR, "Bad if", theIf, filename);
	}
	if (trimStart (getCondition -> string, '{')) {
		if (! trimEnd (getCondition -> string, '}')) return errorBox (ERRORTYPE_PROJECTERROR, "No matching }", getCondition -> string, filename);
	}
	endMarker = outputMarkerCode (theSpace, SLU_BR_ZERO);
	if (! compileSourceBlock (getCondition -> string, localVars, theSpace, filename)) {
		return errorBox (ERRORTYPE_PROJECTERROR, "Can't compile condition in if statement", NULL, filename);
	}	

	char * elseString = "";
	while (theRest) {
		char * newString;
		bool isAnElse;

		getElse = splitString (theRest -> string, ' ', ONCE);

		isAnElse = (getToken (getElse -> string) == TOK_ELSE);

		if (! isAnElse) {
			destroyAll (getElse);
			break;
		}

		// We've found an else!

		if (! destroyFirst (getElse)) return errorBox (ERRORTYPE_PROJECTERROR, "Bad else", theRest -> string, filename);

		if (elseString[0]) {
			newString = joinStrings (elseString, "; ");
			delete elseString;
			elseString = joinStrings (newString, theRest -> string);
			delete newString;
		} else {
			elseString = joinStrings (getElse -> string, "");
		}
		destroyAll (getElse);
		destroyFirst (theRest);
	}

	if (elseString[0]) {
		int endElseMarker;

		endElseMarker = outputMarkerCode (theSpace, SLU_BRANCH);

		addMarker (theSpace, endMarker);
		endMarker = endElseMarker;

		if (trimStart (elseString, '{')) {
			if (! trimEnd (elseString, '}')) return errorBox (ERRORTYPE_PROJECTERROR, "No matching }", getCondition -> string, filename);
		}
		if (! compileSourceBlock (elseString, localVars, theSpace, filename)) {
			return errorBox (ERRORTYPE_PROJECTERROR, "Can't compile", elseString, filename);
		}
	}

	addMarker (theSpace, endMarker);
	return true;
}

bool niceLoop (char * condition, char * & middle, char * endy, stringArray * & localVars, compilationSpace & theSpace, const char * filename) {
	int endMarker, startMarker;

	// Top of the loop...
	addMarker (theSpace, theSpace.numMarkers);
	startMarker = theSpace.numMarkers;
	theSpace.numMarkers ++;

	// Condition
	if (! compileSourceLine (condition, localVars, theSpace, nullArray, NULL)) return false;
	
	if (trimStart (middle, '{')) {
		if (! trimEnd (middle, '}')) return errorBox (ERRORTYPE_PROJECTERROR, "No matching }", middle, NULL);
	}

	// Break if condition is zero, otherwise a bunch of code
	endMarker = outputMarkerCode (theSpace, SLU_BR_ZERO);

	if (! compileSourceBlock (middle, localVars, theSpace, filename)) return false;

	if (endy) if (! compileSourceBlock (endy, localVars, theSpace, filename)) return false;

	fputc			(HALF_MARKER,		theSpace.writeToFile);
	fputc			(SLU_BRANCH,		theSpace.writeToFile);
	put2bytes		(startMarker,		theSpace.writeToFile);

	theSpace.numLines ++;

	// Here's that end-of-the-loop we were talking about
	addMarker (theSpace, endMarker);
	return true;
}

bool handleWhile (char * theCode, stringArray * & localVars, compilationSpace & theSpace, const char * filename) {
	stringArray * getCondition = splitString (theCode, ')', ONCE);
	if (! trimStart (getCondition -> string, '(')) return errorBox (ERRORTYPE_PROJECTERROR, "Bad while (no condition)", theCode, NULL);
	if (countElements (getCondition) != 2) return errorBox (ERRORTYPE_PROJECTERROR, "Bad while (no condition)", theCode, NULL);
	if (! niceLoop (getCondition -> string, getCondition -> next -> string, NULL, localVars, theSpace, filename)) {
		return errorBox (ERRORTYPE_PROJECTERROR, "Bad loop stuff (while loop)", getCondition -> string, NULL);
	} 
	destroyAll (getCondition);
	return true;
}

bool handleFor (char * theCode, stringArray * & localVars, compilationSpace & theSpace, const char * filename) {
	stringArray * getCondition = splitString (theCode, ')', ONCE);
	if (! trimStart (getCondition -> string, '(')) return errorBox (ERRORTYPE_PROJECTERROR, "Bad for (no condition)", theCode, filename);

	// Split the condition into bits
	stringArray * getBits = splitString (getCondition -> string, ';');
	if (! destroyFirst (getCondition)) return errorBox (ERRORTYPE_PROJECTERROR, "Bad for (no condition)", theCode, filename);

	if (countElements (getBits) != 3) return errorBox (ERRORTYPE_PROJECTERROR, "Bad for (not a;b;c in condition)", theCode, filename);

	if (! compileSourceLine (getBits -> string, localVars, theSpace, nullArray, filename)) return false;
	destroyFirst (getBits);

	if (! niceLoop (getBits -> string, getCondition -> string, getBits -> next -> string, localVars, theSpace, filename)) {
		return errorBox (ERRORTYPE_PROJECTERROR, "Bad loop stuff (for loop)", getBits -> string, filename);
	}
	destroyAll (getCondition);
	destroyAll (getBits);
	return true;
}

bool handleLoop (char * & theLoop, stringArray * & localVars, compilationSpace & theSpace, const char * filename) {
	int endMarker;

	if (trimStart (theLoop, '{')) {
		if (! trimEnd (theLoop, '}')) return errorBox (ERRORTYPE_PROJECTERROR, "No matching } in loop", theLoop, NULL);
	}

	addMarker (theSpace, theSpace.numMarkers ++);
	endMarker = theSpace.numMarkers - 1;

	if (! compileSourceBlock (theLoop, localVars, theSpace, filename)) return false;

	fputc			(HALF_MARKER,		theSpace.writeToFile);
	fputc			(SLU_BRANCH,		theSpace.writeToFile);
	put2bytes		(endMarker,			theSpace.writeToFile);
	theSpace.numLines ++;
	return true;
}

bool compileQMark (const char * condition, const char * doThis, stringArray * & localVarNames, compilationSpace & theSpace) {
	int elseMarker, endMarker;
	stringArray * splitter = splitString (doThis, ':', ONCE);

	if (! compileSourceLine (condition, localVarNames, theSpace, nullArray, NULL)) return false;
	elseMarker = outputMarkerCode (theSpace, SLU_BR_ZERO);
	if (! compileSourceLine (splitter -> string, localVarNames, theSpace, nullArray, NULL)) return false;
	endMarker = outputMarkerCode (theSpace, SLU_BRANCH);
	addMarker (theSpace, elseMarker);
	if (! destroyFirst (splitter)) return errorBox (ERRORTYPE_PROJECTERROR, "No : after ?", doThis, NULL);
	if (! compileSourceLine (splitter -> string, localVarNames, theSpace, nullArray, NULL)) return false;
	addMarker (theSpace, endMarker);
	destroyFirst (splitter);
	return true;
}

enum YNF {YNF_FAIL, YNF_YES, YNF_NO};

bool handleVar (const char * sourceCode, stringArray * & localVarNames, sludgeCommand ifLocal, sludgeCommand ifGlobal, sludgeCommand ifIndex, compilationSpace & theSpace, bool arrayPush) {
	int varNum = findElement (localVarNames, sourceCode);
	bool reply = false;

	if (varNum != -1) {
		outputDoneCode (theSpace, ifLocal, varNum);
		reply = true;
	} else {
		stringArray * indexMe = splitAtLast (sourceCode, '[');
		if (countElements (indexMe) == 1) {
			outputHalfCode (theSpace, ifGlobal, sourceCode);
			reply = true;
		} else {
			if (arrayPush) outputDoneCode (theSpace, SLU_STACK_PUSH, 0);
			if (compileSourceLine (indexMe -> string, localVarNames, theSpace, nullArray, NULL)) {

				// Push the array
				outputDoneCode (theSpace, SLU_STACK_PUSH, 0);
				destroyFirst (indexMe);

				// Compile the contents of the []
				if (! trimEnd (indexMe -> string, ']')) {
					errorBox (ERRORTYPE_PROJECTERROR, "No matching ] for [", sourceCode, NULL);
				} else if (compileSourceLine (indexMe -> string, localVarNames, theSpace, nullArray, NULL)) {
					outputDoneCode (theSpace, ifIndex, 0);
					reply = true;
				}
			}
		}
		destroyAll (indexMe);
	}
	return reply;
}

YNF checkMaths (const char * sourceCode, stringArray * & localVarNames, compilationSpace & theSpace) {
	int aa;
	stringArray * splitMaths;
	YNF reply = YNF_NO;
	bool done = false;

	for (aa = 0; aa < numMathsFunc; aa ++) {
		splitMaths = splitAtLast (sourceCode, mathsChar[aa]);
		if (countElements (splitMaths) == 2) {
			done = true;
			if (splitMaths -> string[0] && splitMaths -> next -> string[0]) {
				switch (aa) {
				
					case MATHS_SET:
					if (! compileSourceLine (splitMaths -> next -> string, localVarNames, theSpace, nullArray, NULL)) return YNF_FAIL;
					reply = (YNF) handleVar (splitMaths -> string, localVarNames, SLU_SET_LOCAL, SLU_SET_GLOBAL, SLU_INDEXSET, theSpace, true);
					break;

					case MATHS_QMARK:
					compileQMark (splitMaths -> string, splitMaths -> next -> string, localVarNames, theSpace);
					reply = YNF_YES;
					break;

					case MATHS_DECREMENT:
					case MATHS_INCREMENT:
					errorBox (ERRORTYPE_PROJECTERROR, "++ and -- must only be used with one variable at a time!", sourceCode, NULL);
					reply = YNF_FAIL;
					break;
					
					case MATHS_PLUS_EQ:
					case MATHS_MINUS_EQ:
					case MATHS_MULT_EQ:
					case MATHS_DIV_EQ:
					case MATHS_MOD_EQ:
					if (! compileSourceLine (splitMaths -> string, localVarNames, theSpace, nullArray, NULL)) return YNF_FAIL;
					outputDoneCode (theSpace, SLU_QUICK_PUSH, 0);

					if (! compileSourceLine (splitMaths -> next -> string, localVarNames, theSpace, nullArray, NULL)) return YNF_FAIL;
					outputDoneCode (theSpace, math2Sludge[aa], 0);
					if (handleVar (splitMaths -> string, localVarNames, SLU_SET_LOCAL, SLU_SET_GLOBAL, SLU_INDEXSET, theSpace, true)) {
						reply = YNF_YES;
					} else {
						errorBox (ERRORTYPE_PROJECTERROR, "Couldn't handle something-equals with", splitMaths -> string, NULL);
						reply = YNF_FAIL;
					}
					break;
					
					default:
					if (! compileSourceLine (splitMaths -> string, localVarNames, theSpace, nullArray, NULL)) return YNF_FAIL;
					destroyFirst (splitMaths);
					outputDoneCode (theSpace, SLU_QUICK_PUSH, 0);

					if (! compileSourceLine (splitMaths -> string, localVarNames, theSpace, nullArray, NULL)) return YNF_FAIL;
					destroyFirst (splitMaths);
					outputDoneCode (theSpace, math2Sludge[aa], 0);
					reply = YNF_YES;
				}
			} else if (splitMaths -> string[0]) {
				switch (aa) {
					case MATHS_INCREMENT:
					reply = (YNF) handleVar (splitMaths -> string, localVarNames, SLU_INCREMENT_L, SLU_INCREMENT_G, SLU_INCREMENT_I, theSpace, false);
					break;

					case MATHS_DECREMENT:
					reply = (YNF) handleVar (splitMaths -> string, localVarNames, SLU_DECREMENT_L, SLU_DECREMENT_G, SLU_DECREMENT_I, theSpace, false);
					break;
				}				
			} else {
				destroyFirst (splitMaths);
				switch (aa) {
					case MATHS_MINUS:
					if (! compileSourceLine (splitMaths -> string, localVarNames, theSpace, nullArray, NULL)) return YNF_FAIL;
					outputDoneCode (theSpace, SLU_NEGATIVE, 0);
					reply = YNF_YES;
					break;
				}
			}
		}
		destroyAll (splitMaths);
		if (done) break;
	}
	return reply;
}

bool checkLocal (const char * sourceLine, stringArray * & localVars, compilationSpace & theSpace) {
	if (theSpace.myNum) {
		int gotcha = findElement (localVars, sourceLine);

		if (gotcha != -1) {
			outputDoneCode (theSpace, SLU_LOAD_LOCAL, gotcha);
			return true;
		}
	}
	return false;
}

bool localVar (char * theString, stringArray * & localVars, compilationSpace & theSpace, const char * filename) {
	int numVar;

	stringArray * multi = splitString (theString, ',');

	while (multi) {
		stringArray * getVarName = splitString (multi -> string, '=', ONCE);

		if (! checkValidName (getVarName -> string, "Not a valid variable name", filename)) return false;
		if (! checkNotKnown2 (getVarName -> string, filename)) return false;
		numVar = findElement (localVars, getVarName -> string);
		if (numVar != -1) return errorBox (ERRORTYPE_PROJECTERROR, "Local variable declared twice", getVarName -> string, filename);
		addToStringArray (localVars, getVarName -> string);
		numVar = findElement (localVars, getVarName -> string);

		if (getVarName -> next) {
			if (! compileSourceLine (getVarName -> next -> string, localVars, theSpace, nullArray, NULL)) return false;
			outputDoneCode (theSpace, SLU_SET_LOCAL, numVar);
		}

		destroyAll (getVarName);
		destroyFirst (multi);
	}
	return true;
}

YNF expandTypeDefInner (char * & theString) {
	stringArray * typeDefFromLoop = typeDefFrom;
	stringArray * typeDefToLoop = typeDefTo;
	while (typeDefFromLoop) {
		if (strcmp (theString, typeDefFromLoop -> string) == 0) {
			delete theString;
			theString = joinStrings (typeDefToLoop -> string, "");
			if (! theString) {
				addComment (ERRORTYPE_PROJECTERROR, "Too many constant replacements", NULL);
				return YNF_FAIL;
			}
			return YNF_YES;
		}
		typeDefFromLoop = typeDefFromLoop -> next;
		typeDefToLoop = typeDefToLoop -> next;
	}
	return YNF_NO;
}

bool expandTypeDefOK (char * & theString) {
	int timesDone = 0;

	for (;;) {
		switch (expandTypeDefInner (theString)) {
			case YNF_YES:
			if (++ timesDone == 10) {
				addComment (ERRORTYPE_PROJECTERROR, "Too many constant replacements... you fool!", NULL);
				return false;
			}
			break;
			
			case YNF_NO:
			return true;
			break;
			
			case YNF_FAIL:
			return false;
			break;
		}
	}

	// We never get here
}

bool callAFunction (const char * sourceCode, stringArray * & localVarNames, compilationSpace & theSpace, const char * filename) {
	stringArray * splitParams;
	stringArray * eachParam;
	int varNum;
	char * keepNameIfUser;

	splitParams = splitAtLast (sourceCode, '(');
	if (! expandTypeDefOK (splitParams -> string)) return false;
	varNum = findElement (builtInFunc, splitParams -> string);

	keepNameIfUser = joinStrings (splitParams -> string, "");

	// Get rid of the sub name and the closing bracket...
	if (! destroyFirst (splitParams)) return errorBox (ERRORTYPE_PROJECTERROR, "Bad sub call", sourceCode, filename);
	trimEnd (splitParams -> string, ')');

	// Split the parameters up...
	eachParam = splitString (splitParams -> string, ',', REPEAT);
	if (! eachParam -> string[0]) destroyFirst (eachParam);
	if (destroyFirst (splitParams)) return errorBox (ERRORTYPE_PROJECTERROR, "Bad sub call", sourceCode, filename);

	// Store the number of parameters...
	int nP = countElements (eachParam);

	// Compile the parameters...
	while (eachParam) {
		if (! compileSourceLine (eachParam -> string, localVarNames, theSpace, nullArray, filename)) return false;
		destroyFirst (eachParam);
		outputDoneCode (theSpace, SLU_QUICK_PUSH, 0);
	}

	if (varNum != -1) {
		outputDoneCode (theSpace, SLU_LOAD_BUILT, varNum);
	} else {
		if (! compileSourceLine (keepNameIfUser, localVarNames, theSpace, nullArray, filename)) return false;
	}

	delete keepNameIfUser;

	outputDoneCode (theSpace, SLU_CALLIT, nP);
	return true;
}

bool getArrayIndex (const char * sourceCode, stringArray * & localVarNames, compilationSpace & theSpace, const char * filename) {
	stringArray * splitParams;

	splitParams = splitAtLast (sourceCode, '[');

	if (! compileSourceLine (splitParams -> string, localVarNames, theSpace, nullArray, NULL)) {
		return errorBox (ERRORTYPE_PROJECTERROR, "Can't compile", splitParams -> string, filename);
	}

	// Get rid of the array name and then the closing bracket...
	if (! destroyFirst (splitParams)) return errorBox (ERRORTYPE_PROJECTERROR, "Bad [] index", sourceCode, filename);
	trimEnd (splitParams -> string, ']');
	
	// Push the array onto the run-time stack and compile index number
	outputDoneCode (theSpace, SLU_QUICK_PUSH, 0);
	if (! compileSourceLine (splitParams -> string, localVarNames, theSpace, nullArray, filename)) return false;

	destroyAll (splitParams);
	outputDoneCode (theSpace, SLU_INDEXGET, 0);
	return true;
}

void compileNumber (const char * textNumber, compilationSpace & theSpace) {
	outputDoneCode (theSpace, SLU_LOAD_VALUE, stringToInt (textNumber, ERRORTYPE_PROJECTERROR));
}

bool checkString (const char * sourceCode, compilationSpace & theSpace) {
	bool reply = false;
	int i;
	stringArray * grabNum = splitString (sourceCode, 'g', ONCE);

	if (strcmp (grabNum -> string, "_strin") == 0) {
		if (destroyFirst (grabNum)) {
			i = stringToInt (grabNum -> string, ERRORTYPE_PROJECTERROR);
			if (i < numStringsFound && i >= 0) {
				outputDoneCode (theSpace, SLU_LOAD_STRING, i);
				reply = true;
			}
		}
	}
	destroyAll (grabNum);
	return reply;
}

bool checkFileHandle (const char * sourceCode, compilationSpace & theSpace) {
	bool reply = false;
	int i;
	stringArray * grabNum = splitString (sourceCode, 'e', ONCE);

	if (strcmp (grabNum -> string, "_fil") == 0) {
		if (destroyFirst (grabNum)) {
			i = stringToInt (grabNum -> string, ERRORTYPE_PROJECTERROR);
			if (i < numFilesFound) {
				outputDoneCode (theSpace, SLU_LOAD_FILE, i);
				reply = true;
			}
		}
	}
	destroyFirst (grabNum);
	return reply;
}

bool compileSourceLineInner (const char * sourceCodeIn, stringArray * & localVarNames, compilationSpace & theSpace, stringArray * & theRest, const char * filename) {
	char * sourceCode = joinStrings (sourceCodeIn, "");

	if (! expandTypeDefOK (sourceCode)) return false;

	stringArray * getType = splitString (sourceCode, ' ', ONCE);
	YNF ynf;

	switch (getToken (getType -> string)) {
		case TOK_ELSE:
		return errorBox (ERRORTYPE_PROJECTERROR, "Misplaced else", sourceCode, filename);

		case TOK_UNKNOWN:
		destroyAll (getType);	// Save a bit of memory...

		// Let's see... is it maths?
		ynf = checkMaths (sourceCode, localVarNames, theSpace);
		if (ynf == YNF_FAIL) return false;
		if (ynf == YNF_YES) break;

		// Is it a local?
		if (checkLocal (sourceCode, localVarNames, theSpace)) break;

		if (checkString (sourceCode, theSpace)) break;
		if (checkFileHandle (sourceCode, theSpace)) break;

		// OK, we know it's not maths... so, if it starts with a ( it should end
		// with a ), so we can just compile the insides

		if (sourceCode[0] == '(') {
			stringArray * insides = splitString (sourceCode, '(', ONCE);
			if (! destroyFirst (insides)) return errorBox (ERRORTYPE_PROJECTERROR, "Mismatched () - 001", sourceCode, filename);
			if (! trimEnd (insides -> string, ')')) return errorBox (ERRORTYPE_PROJECTERROR, "Mismatched () - 002", sourceCode, filename);
			if (! compileSourceLine (insides -> string, localVarNames, theSpace, nullArray, filename)) return false;
			if (destroyFirst (insides)) return errorBox (ERRORTYPE_PROJECTERROR, "Mismatched () - 003", sourceCode, filename);
			break;
		}

		// No? In that case, if it ends with a ')' it's a function call

		if (sourceCode[strlen (sourceCode) - 1] == ')') {
			if (! callAFunction (sourceCode, localVarNames, theSpace, filename)) return false;
			break;
		}

		// No? In that case, if it ends with a ']' it's an array index

		if (sourceCode[strlen (sourceCode) - 1] == ']') {
			if (! getArrayIndex (sourceCode, localVarNames, theSpace, filename)) return false;
			break;
		}

		// Is it a number, then?

		if (sourceCode[0] >= '0' && sourceCode[0] <= '9') {
			compileNumber (sourceCode, theSpace);
			break;
		}

		outputHalfCode (theSpace, SLU_LOAD_GLOBAL, sourceCode);
		break;

		case TOK_VAR:
//		printf ("It's a local variable!\n");
		if (! destroyFirst (getType)) return errorBox (ERRORTYPE_PROJECTERROR, "Bad local variable definition", sourceCode, filename);
		if (! localVar (getType -> string, localVarNames, theSpace, filename)) return false;
		break;

		case TOK_IF:
		if (! destroyFirst (getType)) return errorBox (ERRORTYPE_PROJECTERROR, "Dodgy if statement", sourceCode, filename);
		if (! handleIf (getType -> string, localVarNames, theSpace, theRest, filename)) return false;
		break;

		case TOK_WHILE:
		if (! destroyFirst (getType)) return errorBox (ERRORTYPE_PROJECTERROR, "Dodgy while statement", sourceCode, filename);
		if (! handleWhile (getType -> string, localVarNames, theSpace, filename)) return false;
		break;

		case TOK_FOR:
		if (! destroyFirst (getType)) return errorBox (ERRORTYPE_PROJECTERROR, "Dodgy for statement", sourceCode, filename);
		if (! handleFor (getType -> string, localVarNames, theSpace, filename)) return false;
		break;

		case TOK_LOOP:
		if (! destroyFirst (getType)) return errorBox (ERRORTYPE_PROJECTERROR, "Dodgy loop statement", sourceCode, filename);
		if (! handleLoop (getType -> string, localVarNames, theSpace, filename)) return false;
		break;

		case TOK_NOT:
		if (! destroyFirst (getType)) return errorBox (ERRORTYPE_PROJECTERROR, "Dodgy ! statement", sourceCode, filename);
		if (! compileSourceLine (getType -> string, localVarNames, theSpace, nullArray, filename)) return false;
		outputDoneCode (theSpace, SLU_NOT, 0);
		break;

		case TOK_RETURN:
		if (destroyFirst (getType)) {
			if (! compileSourceLine (getType -> string, localVarNames, theSpace, nullArray, filename)) return false;
		} else {
			outputDoneCode (theSpace, SLU_LOAD_NULL, 0);
		}
		outputDoneCode (theSpace, SLU_RETURN, 0);
		break;

		default:
		return errorBox (ERRORTYPE_PROJECTERROR, "Can't use this SLUDGE reserved word inside a sub", getType -> string, filename);
	}

	destroyAll (getType);

	delete sourceCode;
	return true;
}

bool compileSourceLine (const char * sourceCode, stringArray * & localVarNames, compilationSpace & theSpace, stringArray * & theRest, const char * filename) {
	return compileSourceLineInner (sourceCode, localVarNames, theSpace, theRest, filename); //? true : errorBox ("Couldn't compile SLUDGE source code", sourceCode);
}

bool compileSourceBlock (const char * sourceCode, stringArray * & localVarNames, compilationSpace & theSpace, const char * filename) {
	stringArray * splitBlock = splitString (sourceCode);

	do {
		if (splitBlock -> string[0]) {
			if (! compileSourceLine (splitBlock -> string, localVarNames, theSpace, splitBlock -> next, filename)) return false;
		}
	} while (destroyFirst (splitBlock));
	return true;
}

int defineFunction (char * funcName, char * args, char * sourceCode, bool unfreezable, bool debugMe, const char * fileName) {
	int funcNum;
	stringArray * localVarNames;
	compilationSpace localSpace;

//	if (unfreezable) errorBox ("Found an unfreezable function", funcName);
	if (! checkNotKnown (funcName, fileName)) return false;
	funcNum = findOrAdd (functionNames, funcName);
	localVarNames = splitString (args, ',', REPEAT);
	if (! localVarNames -> string[0]) destroyFirst (localVarNames);
	addToStringArray (functionFiles, fileName);
	
	if (! startFunction (funcNum, countElements (localVarNames), localSpace, funcName, unfreezable, debugMe, fileName)) {
		return -1;
	}
	if (! compileSourceBlock (sourceCode, localVarNames, localSpace, fileName)) {
		return -1;
	}
	if (! finishFunctionNew (localSpace, localVarNames)) return -1;
	destroyAll (localVarNames);
	return funcNum;
}


void initBuiltInFunc ()
{

#define FUNC(special,name) addToStringArray(builtInFunc, #name);
#include "functionList.h"
#undef FUNC
	
	addToStringArray (typeDefFrom, "NULL");			addToStringArray (typeDefTo, "0");

	addToStringArray (typeDefFrom, "false");		addToStringArray (typeDefTo, "0");
	addToStringArray (typeDefFrom, "true");			addToStringArray (typeDefTo, "1");

	addToStringArray (typeDefFrom, "CENTER");		addToStringArray (typeDefTo, "65535");
	addToStringArray (typeDefFrom, "CENTRE");		addToStringArray (typeDefTo, "65535");
	addToStringArray (typeDefFrom, "LEFT");			addToStringArray (typeDefTo, "999");
	addToStringArray (typeDefFrom, "RIGHT");		addToStringArray (typeDefTo, "1001");
	addToStringArray (typeDefFrom, "AUTOFIT");		addToStringArray (typeDefTo, "65535");

	addToStringArray (typeDefFrom, "NORTH");		addToStringArray (typeDefTo, "0");
	addToStringArray (typeDefFrom, "NORTHEAST");	addToStringArray (typeDefTo, "45");
	addToStringArray (typeDefFrom, "EAST");			addToStringArray (typeDefTo, "90");
	addToStringArray (typeDefFrom, "SOUTHEAST");	addToStringArray (typeDefTo, "135");
	addToStringArray (typeDefFrom, "SOUTH");		addToStringArray (typeDefTo, "180");
	addToStringArray (typeDefFrom, "SOUTHWEST");	addToStringArray (typeDefTo, "225");
	addToStringArray (typeDefFrom, "WEST");			addToStringArray (typeDefTo, "270");
	addToStringArray (typeDefFrom, "NORTHWEST");	addToStringArray (typeDefTo, "315");

	addToStringArray (typeDefFrom, "NORMAL");		addToStringArray (typeDefTo, "0");
	addToStringArray (typeDefFrom, "TRANSPARENT3");	addToStringArray (typeDefTo, "1");
	addToStringArray (typeDefFrom, "TRANSPARENT2");	addToStringArray (typeDefTo, "2");
	addToStringArray (typeDefFrom, "TRANSPARENT");	addToStringArray (typeDefTo, "2");
	addToStringArray (typeDefFrom, "TRANSPARENT1");	addToStringArray (typeDefTo, "3");
	addToStringArray (typeDefFrom, "DARK1");		addToStringArray (typeDefTo, "4");
	addToStringArray (typeDefFrom, "DARK2");		addToStringArray (typeDefTo, "5");
	addToStringArray (typeDefFrom, "DARK");			addToStringArray (typeDefTo, "5");
	addToStringArray (typeDefFrom, "DARK3");		addToStringArray (typeDefTo, "6");
	addToStringArray (typeDefFrom, "DARK4");		addToStringArray (typeDefTo, "7");
	addToStringArray (typeDefFrom, "BLACK");		addToStringArray (typeDefTo, "7");
	addToStringArray (typeDefFrom, "SHADOW1");		addToStringArray (typeDefTo, "8");
	addToStringArray (typeDefFrom, "SHADOW2");		addToStringArray (typeDefTo, "9");
	addToStringArray (typeDefFrom, "SHADOW");		addToStringArray (typeDefTo, "9");
	addToStringArray (typeDefFrom, "SHADOW3");		addToStringArray (typeDefTo, "10");
	addToStringArray (typeDefFrom, "FOGGY1");		addToStringArray (typeDefTo, "11");
	addToStringArray (typeDefFrom, "FOGGY2");		addToStringArray (typeDefTo, "12");
	addToStringArray (typeDefFrom, "FOGGY");		addToStringArray (typeDefTo, "12");
	addToStringArray (typeDefFrom, "FOGGY3");		addToStringArray (typeDefTo, "13");
	addToStringArray (typeDefFrom, "FOGGY4");		addToStringArray (typeDefTo, "14");
	addToStringArray (typeDefFrom, "GREY");			addToStringArray (typeDefTo, "14");
	addToStringArray (typeDefFrom, "GRAY");			addToStringArray (typeDefTo, "14");
	addToStringArray (typeDefFrom, "GLOW1");		addToStringArray (typeDefTo, "15");
	addToStringArray (typeDefFrom, "GLOW2");		addToStringArray (typeDefTo, "16");
	addToStringArray (typeDefFrom, "GLOW");			addToStringArray (typeDefTo, "16");
	addToStringArray (typeDefFrom, "GLOW3");		addToStringArray (typeDefTo, "17");
	addToStringArray (typeDefFrom, "GLOW4");		addToStringArray (typeDefTo, "18");
	addToStringArray (typeDefFrom, "WHITE");		addToStringArray (typeDefTo, "18");
	addToStringArray (typeDefFrom, "INVISIBLE");	addToStringArray (typeDefTo, "19");

	addToStringArray (typeDefFrom, "FADE");			addToStringArray (typeDefTo, "0");
	addToStringArray (typeDefFrom, "DISOLVE");		addToStringArray (typeDefTo, "1");
	addToStringArray (typeDefFrom, "DISOLVE1");		addToStringArray (typeDefTo, "1");
	addToStringArray (typeDefFrom, "DISOLVE2");		addToStringArray (typeDefTo, "2");
	addToStringArray (typeDefFrom, "TVSTATIC");		addToStringArray (typeDefTo, "3");
	addToStringArray (typeDefFrom, "BLINDS");		addToStringArray (typeDefTo, "4");
	addToStringArray (typeDefFrom, "CROSSFADE");	addToStringArray (typeDefTo, "5");
	addToStringArray (typeDefFrom, "SNAPSHOTFADE");	addToStringArray (typeDefTo, "5");
	addToStringArray (typeDefFrom, "SNAPSHOTBOX");	addToStringArray (typeDefTo, "6");

	addToStringArray (typeDefFrom, "SOUNDONLY");	addToStringArray (typeDefTo, "2");
	addToStringArray (typeDefFrom, "SOUNDANDTEXT");	addToStringArray (typeDefTo, "1");
	addToStringArray (typeDefFrom, "TEXTONLY");		addToStringArray (typeDefTo, "0");

	addToStringArray (typeDefFrom, "setStatusColor");		addToStringArray (typeDefTo, "setStatusColour");
	addToStringArray (typeDefFrom, "setLitStatusColor");	addToStringArray (typeDefTo, "setLitStatusColour");
	addToStringArray (typeDefFrom, "setPasteColor");		addToStringArray (typeDefTo, "setPasteColour");
	addToStringArray (typeDefFrom, "setBlankColor");		addToStringArray (typeDefTo, "setBlankColour");
	addToStringArray (typeDefFrom, "setBurnColor");			addToStringArray (typeDefTo, "setBurnColour");
	addToStringArray (typeDefFrom, "getPixelColor");		addToStringArray (typeDefTo, "getPixelColour");
	addToStringArray (typeDefFrom, "dequeue");				addToStringArray (typeDefTo, "popFromStack");
	addToStringArray (typeDefFrom, "deleteGame");			addToStringArray (typeDefTo, "deleteFile");
	addToStringArray (typeDefFrom, "renameGame");			addToStringArray (typeDefTo, "renameFile");
	addToStringArray (typeDefFrom, "getSavedGames");		addToStringArray (typeDefTo, "getMatchingFiles");
	
	addToStringArray (typeDefFrom, "FRONT");		addToStringArray (typeDefTo, "1");
	addToStringArray (typeDefFrom, "FIXEDSIZE");	addToStringArray (typeDefTo, "2");
	addToStringArray (typeDefFrom, "NOZBUFFER");	addToStringArray (typeDefTo, "4");
	addToStringArray (typeDefFrom, "FIXTOSCREEN");	addToStringArray (typeDefTo, "8");
	addToStringArray (typeDefFrom, "NOLIGHTMAP");	addToStringArray (typeDefTo, "16");
	addToStringArray (typeDefFrom, "ICON");			addToStringArray (typeDefTo, "31");
	addToStringArray (typeDefFrom, "NOREMOVE");		addToStringArray (typeDefTo, "32");
	addToStringArray (typeDefFrom, "RECTANGULAR");	addToStringArray (typeDefTo, "64");

	// Light map modes
	addToStringArray (typeDefFrom, "PERPIXEL");		addToStringArray (typeDefTo, "1");
	addToStringArray (typeDefFrom, "HOTSPOT");		addToStringArray (typeDefTo, "0");
}

void doDefines (char * fn, stringArray * & strings, stringArray * & fileHandles) {
	FILE * fp = fopen (fn, "rt");
	if (fp) {
		for (;;) {
			char * t = readText (fp);
			if (!t) break;
			stringArray * sa = splitString (t, '#');
			if (sa -> string[0]) {
				stringArray * bits = splitString (sa -> string, '=', ONCE);
				if (bits -> next) {
					trimEnd   (bits -> string,         '\t');
					trimStart (bits -> string,         '\t');
					trimEnd   (bits -> next -> string, '\t');
					trimStart (bits -> next -> string, '\t');

					if (trimStart (bits->next->string, '"')) {
						if (! trimEnd (bits->next->string, '"')) errorBox (ERRORTYPE_PROJECTERROR, "Definition starts with \", but doesn't end with it! Sorry, currently support for complex constants is very limited", bits->next->string, fn);
						char * newTarget = new char[30];
						sprintf (newTarget, "_string%i", findOrAdd (strings, bits->next->string, false));
						delete bits->next->string;
						bits->next->string = newTarget;
					} else if (trimStart (bits->next->string, '"')) {
						if (! trimEnd (bits->next->string, '\'')) errorBox (ERRORTYPE_PROJECTERROR, "Definition starts with ', but doesn't end with it! Sorry, currently support for complex constants is very limited", bits->next->string, fn);
						char * newTarget = new char[30];
						sprintf (newTarget, "_file%i", findOrAdd (fileHandles, bits->next->string, false));
						delete bits->next->string;
						bits->next->string = newTarget;
					}

					addToStringArray (typeDefFrom, bits -> string);
					addToStringArray (typeDefTo,   bits -> next -> string);
				} else {
					errorBox (ERRORTYPE_PROJECTERROR, "No = in definition line", sa -> string, fn);
				}
				destroyAll (bits);
			}
			destroyAll (sa);
			delete t;
		}
		fclose (fp);
	} else {
		errorBox (ERRORTYPE_PROJECTERROR, "Can't read definition file", fn, NULL);
	}
}

bool outdoorSub (char * code, const char * fileName) {
	stringArray * getArguments;
	stringArray * getContents = splitString (code, ')', ONCE);
	stringArray * getSpecial;
	bool unfreezable = false, debugMe = false;
	tokenType t;

	getArguments = splitString (getContents -> string, '(', ONCE);
//	errorBox ("getArguments -> string", getArguments -> string);
	
	getSpecial = splitString (getArguments -> string, ' ');
	destroyFirst (getArguments);

	while (countElements (getSpecial) > 1) {
		t = getToken (getSpecial -> string);
		switch (t) {
			case TOK_UNFREEZABLE:
			unfreezable = true;
			break;
			
			case TOK_DEBUG:
			debugMe = true;
			break;
			
			default:
			return errorBox (ERRORTYPE_PROJECTERROR, "Syntax error... this isn't an option which can be applied to a function", getSpecial -> string, fileName);
		}
		destroyFirst (getSpecial);
	}

	char * functionName = joinStrings (inThisClass, getSpecial -> string);

	setWindowText (COM_ITEMTEXT, functionName);

	if (! destroyFirst (getContents)) {
		errorBox (ERRORTYPE_PROJECTERROR, "Bad sub definition (no parameters)", functionName, fileName);
	} else if (countElements (getArguments) != 1) {
		errorBox (ERRORTYPE_PROJECTERROR, "Bad sub definition (no opening bracket)", functionName, fileName);
	} else if (! trimStart (getContents -> string, '{')) {
		errorBox (ERRORTYPE_PROJECTERROR, "No opening squirly brace in sub definition", functionName, fileName);
	} else if (! trimEnd (getContents -> string, '}')) {
		errorBox (ERRORTYPE_PROJECTERROR, "No closing squirly brace in sub definition", functionName, fileName);
	} else if (defineFunction (functionName,			 		// Name of function
							   getArguments -> string, 			// Args without ()
							   getContents -> string,			// Code without {}
							   unfreezable, debugMe,
							   fileName) == -1) {

		errorBox (ERRORTYPE_PROJECTERROR, "Couldn't compile function", functionName, fileName);
	} else {
		destroyAll (getContents);
		destroyAll (getArguments);
		destroyAll (getSpecial);
		return true;
	}
	delete functionName;
	return false;
}

void writeDebugData (FILE * mainFile) {
	stringArray * funcName;

	// Built in functions...

	funcName = builtInFunc;
	put2bytes (countElements (funcName), mainFile);
	while (funcName) {
		writeString (funcName -> string, mainFile);
		funcName = funcName -> next;
	}
	
	// User defined functions...
	
	funcName = functionNames;
	put2bytes (countElements (funcName), mainFile);
	while (funcName) {
		writeString (funcName -> string, mainFile);
		funcName = funcName -> next;
	}

	// Resource files...
	
	funcName = allFileHandles;
	put2bytes (countElements (funcName), mainFile);
	while (funcName) {
		writeString (funcName -> string, mainFile);
		funcName = funcName -> next;
	}
}

