#include <windows.h>
#include <stdio.h>
#include <unistd.h>

#include "moreio.h"
#include "splitter.h"
#include "function.h"
#include "settings.h"
#include "messbox.h"
#include "linker.h"
#include "winterfa.h"
#include "wintext.h"
#include "checkUsed.h"

extern stringArray * functionNames;
extern stringArray * allKnownFlags;
extern stringArray * objectTypeNames;

char * sludgeText[] = {"?????",
  "RETURN", "BRANCH", "BR_ZERO", "SET_GLOBAL",
  "SET_LOCAL", "LOAD_GLOBAL", "LOAD_LOCAL",
  "PLUS", "MINUS", "MULT", "DIVIDE",
  "AND", "OR", "EQUALS", "NOT_EQ", "MODULUS", "LOAD_VALUE",
  "LOAD_BUILT", "LOAD_FUNC", "CALLIT", "LOAD_STRING", "LOAD_FILE",
  "LOAD_OBJ_TYPE", "NOT", "LOAD_NULL", "STACK_PUSH",
  "LESS_THAN", "MORE_THAN", "NEGATIVE", "UNREGMESSAGE", "LESS_EQUAL", "MORE_EQUAL",
  "INCREMENT_LOCAL", "DECREMENT_LOCAL", "INCREMENT_GLOBAL", "DECREMENT_GLOBAL",
  "INDEXSET", "INDEXGET", "INCREMENT_INDEX", "DECREMENT_INDEX", "QUICK_PUSH"};

//FILE * linkText = NULL;

//void initLinker () {
//	gotoOutputDirectory ();
//	linkText = fopen ("ASSEMBLE.TXT", "wt");
//}

BOOL runLinker (FILE * mainFile, FILE * indexFile, int functionNum, stringArray * globalVarNames, unsigned long calcIndexSize, stringArray * allSourceStrings) {
	char filename[32];
	char * grabbedText, * originalName;
	FILE * readObj, * totalsFile;
	unsigned int numArg, numLocals, numMarkers, a, b;
	unsigned int numLines;
	unsigned int * markers;
	halfCode halfType;
	long int numGlobalsSet = 0;
	BOOL unfreezable, debugMe;
	
	put4bytes (ftell (mainFile) + calcIndexSize, indexFile);

	if (! gotoTempDirectory ()) return FALSE;
	sprintf (filename, "_F%05i.DAT", functionNum);

	readObj = fopen (filename, "rb");
	if (readObj == NULL) return errorBox (ERRORTYPE_SYSTEMERROR, "Can't open", filename, NULL);

	originalName = readString (readObj);
	setWindowText (COM_ITEMTEXT, originalName);

	grabbedText = readString (readObj);
	setWindowText (COM_FILENAME, grabbedText);
	char * theOriginalFilename = grabbedText;

	char * classNameDot = readString (readObj);

	sprintf (filename, "_F%05iN.DAT", functionNum);
	totalsFile = fopen (filename, "rb");
	if (totalsFile == NULL) return errorBox (ERRORTYPE_SYSTEMERROR, "Can't open", filename, NULL);

//	if (! gotoOutputDirectory ()) return FALSE;
//	sprintf (filename, "F%05i.FUN", functionNum);
//	createFinalFile = fopen (filename, "wb");
//	if (createFinalFile == NULL) return errorBox ("Can't write", filename);

	// Now - let's get the totals

	unfreezable = fgetc (readObj);
	debugMe = fgetc (readObj);
//	errorBox (originalName, debugMe ? "Debugging" : "No debugging");
	numArg = fgetc (readObj);
	numLocals = fgetc (totalsFile);
	numMarkers = fgetc (totalsFile);
	numLines = get2bytes (totalsFile);
	fclose (totalsFile);
	
	BOOL * localsUsed = new BOOL[numLocals];
	memset (localsUsed, 0, sizeof(BOOL) * numLocals);

	fputc (unfreezable, mainFile);
	put2bytes (numLines, mainFile);
	put2bytes (numArg, mainFile);
	put2bytes (numLocals, mainFile);

	markers = new unsigned int [numMarkers];
//	checkNew (markers);

	if (! gotoTempDirectory ()) return FALSE;
	sprintf (filename, "_F%05iM.DAT", functionNum);
	totalsFile = fopen (filename, "rb");
	if (totalsFile == NULL) return errorBox (ERRORTYPE_SYSTEMERROR, "Can't open", filename, NULL);

	unsigned int n = numMarkers;

	while (n --) {
		a = get2bytes (totalsFile);
		b = get2bytes (totalsFile);
		markers[a] = b;
	}

	fclose (totalsFile);

	FILE * debugFile = NULL;

	if (debugMe) {
		sprintf (filename, "Debug function %s.txt", originalName);
		if (! gotoSourceDirectory ()) return FALSE;
		debugFile = fopen (filename, "wt");
		if (! gotoTempDirectory ()) return FALSE;
		fprintf (debugFile, "%i parameters...\n%i locals...\n\n", numArg, numLocals);
	}
	
	char * functionName = originalName;
	// delete originalName;
	// Now the assembludge

	unsigned int value;

	for (a = 0; a < numLines; a ++) {
		char * bigName;
		halfType = (halfCode) fgetc (readObj);
		b = fgetc (readObj);
		originalName = NULL;
		switch (halfType) {
			case HALF_DONE:
			value = get2bytes (readObj);
			if (b == SLU_LOAD_LOCAL)
			{
				localsUsed[value] = TRUE;
			}
			break;

			case HALF_MARKER:
			value = get2bytes (readObj);
			if (value > numMarkers) return errorBox (ERRORTYPE_INTERNALERROR, "Not that many markers!", NULL, theOriginalFilename);
			value = markers[(int) value];
			break;

			case HALF_FIND:
			grabbedText = readString (readObj);

			// For debugging...
			originalName = joinStrings ("", grabbedText);

			// We may be in a class...
			bigName = joinStrings (classNameDot, grabbedText);

			switch (b) {
/*				case SLU_LOAD_FUNC: // Parameter should be a user function
				value = findElement (functionNames, grabbedText);
				if (value == -1) return errorBox ("Can't find a function called", grabbedText);
				break;
*/
				case SLU_LOAD_GLOBAL: // Parameter should be a u-function, scene etc.
				b = SLU_LOAD_OBJTYPE;
				value = findElement (objectTypeNames, grabbedText);
				if (value != -1) break;

				b = SLU_LOAD_GLOBAL;
				value = findElement (globalVarNames, bigName);
				if (value != -1) break;
				
				b = SLU_LOAD_FUNC;
				value = findElement (functionNames, bigName);
				if (value != -1) break;

				b = SLU_LOAD_GLOBAL;
				value = findElement (globalVarNames, grabbedText);
				if (value != -1) break;
				
				b = SLU_LOAD_FUNC;
				value = findElement (functionNames, grabbedText);
				if (value != -1) break;

				b = SLU_LOAD_VALUE;
				value = findElement (allKnownFlags, grabbedText);
				if (value != -1) break;

				return errorBox (ERRORTYPE_PROJECTERROR, "Not a function, variable or object type!", grabbedText, theOriginalFilename);

				case SLU_DECREMENT_G:
				case SLU_INCREMENT_G:
				case SLU_SET_GLOBAL: // Parameter should be a global variable
				value = (long int) findElement (globalVarNames, grabbedText);
				if (value == -1) return errorBox (ERRORTYPE_PROJECTERROR, "Not a global variable name", grabbedText, theOriginalFilename);
				break;

				default:
				return errorBox (ERRORTYPE_INTERNALERROR, "Shouldn't have to resolve parameter for this command", sludgeText[b], theOriginalFilename);
			}
			delete grabbedText;
			delete bigName;
			break;

			default:
			return errorBox (ERRORTYPE_INTERNALERROR, "Unknown half code", NULL, theOriginalFilename);
		}

		if (b == SLU_LOAD_FUNC)
			setUsed(CHECKUSED_FUNCTIONS, value);

		if (b == SLU_LOAD_GLOBAL)
			setUsed(CHECKUSED_GLOBALS, value);

		fputc (b, mainFile);
		put2bytes (value, mainFile);
		
		if (functionNum == 0) {		// Extra error checking for global init function
			if (b == SLU_LOAD_GLOBAL && value >= numGlobalsSet) {
				addComment (ERRORTYPE_PROJECTERROR, "Trying to initialise a global variable using a later (or earlier and undefined) one", theOriginalFilename);
				return FALSE;
			} else if (b == SLU_SET_GLOBAL) {
				numGlobalsSet = value + 1;
			}
		}
		
		if (debugMe) {
			if (b == SLU_LOAD_STRING)
				originalName = joinStrings (returnElement (allSourceStrings, value), "");
		
			char padder[2] = {'\t', NULL};
			if (strlen (sludgeText[b]) > 10) padder[0] = NULL;
			if (originalName) {
				fprintf (debugFile, "%03i: %s%s\t%i\t(%s)\n", a, sludgeText[b], padder, value, originalName);
			} else {
				fprintf (debugFile, "%03i: %s%s\t%i\n", a, sludgeText[b], padder, value);
			}
		}
		if (originalName) {
			delete originalName;
			originalName = NULL;
		}
	}

	for (int h = 0; h < numLocals; h ++)
	{
		char * localName = readString (readObj);
		if (! localsUsed[h])
		{
			char * buff = joinStrings ("Function ", functionName, h < numArg ? " contains unused parameter " : " contains unused variable ", localName);
			addComment (ERRORTYPE_PROJECTWARNING, buff, theOriginalFilename);
			delete buff;
		}
		delete localName;
	}
	
	delete localsUsed;
	delete classNameDot;
	delete functionName;
	delete theOriginalFilename;
	if (debugMe) fclose (debugFile);
	fclose (readObj);

	sprintf (filename, "_F%05i.DAT", functionNum);
	unlink (filename);
	sprintf (filename, "_F%05iN.DAT", functionNum);
	unlink (filename);
	sprintf (filename, "_F%05iM.DAT", functionNum);
	unlink (filename);

	return TRUE;
}