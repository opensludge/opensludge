#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "messbox.h"
#define MAXINT 65535

//#include "typedef.h"
#include "splitter.h"
//#include "fatal.h"

stringArray * nullArray = NULL;

long stringToInt (const char * textNumber, int errorType) {
	unsigned long i = 0;
	int ac = 0;

	while (textNumber[ac]) {
		if (textNumber[ac] >= '0' && textNumber[ac] <= '9') {
			i = (i * 10) + textNumber[ac] - '0';
			if (i > MAXINT) {
				errorBox (errorType, "Number too large", textNumber, NULL);
				return -1;
			}
		} else {
			errorBox (errorType, "Oh no! I thought this was going to be a number, but there's a non-numerical character in it", textNumber, NULL);
			return -1;
		}
		ac ++;			
	}
	
	return (long) i;
}

void addToStringArray (stringArray * & theArray, const char * theString, int start, int size, BOOL trimSpa) {
	char * addMe;
	stringArray * newSection;
	stringArray * huntArray = theArray;
	int looper;

	if (size == -1) size = strlen (theString);
	size -= start;

	// Allocate memory

	addMe = new char[size + 1];
//	checkNew (addMe);
	newSection = new stringArray;
//	checkNew (newSection);

	// Create new stringArray section

	for (looper = 0; looper < size; looper ++) {
		addMe[looper] = theString[looper + start];
	}
	addMe[size] = NULL;

	if (trimSpa) trimEdgeSpace (addMe);
//	else errorBox ("Just added without trimming", addMe);

	newSection -> string = addMe;
	newSection -> next = NULL;

//	printf ("I want to add... \"%s\"\n", addMe);

	// Add it

	if (theArray) {
		while (huntArray -> next) {
//			printf ("Skipping %s... ", huntArray -> string);
			huntArray = huntArray -> next;
		}
//		printf ("Adding after %s\n", huntArray -> string);
		huntArray -> next = newSection;
	} else {
		theArray = newSection;
	}
}

stringArray * splitAtLast (const char * inString, const char findCharIn) {
	stringArray * newStringArray = NULL;
	int a, gotcha = -1, indent = 0, addIndent;
	char findChar = findCharIn;

	for (a = 0; inString[a]; a ++) {
		addIndent = 0;
		switch (inString[a]) {
			case '{':
			case '[':
			case '(':
			addIndent = 1;
			break;

			case '}':
			case ']':
			case ')':
			if (! (indent --)) {
				errorBox (ERRORTYPE_PROJECTERROR, "Unexpected }, ] or ) in", inString, NULL);
				indent ++;
			}
			break;
		}
		if ((inString[a] == findChar) && (indent == 0)) {
			gotcha = a;
		}
		indent += addIndent;
	}
	if (gotcha != -1) {
		addToStringArray (newStringArray, inString, 0, gotcha);
		addToStringArray (newStringArray, inString, gotcha + 1);
	} else {
		addToStringArray (newStringArray, inString);
	}
	return newStringArray;
}

stringArray * splitString (const char * inString, const char findCharIn, const splitMode howMany, BOOL thenTrim) {
	stringArray * newStringArray = NULL;
	int a, stringLen = strlen (inString), addIndent, indent = 0, lastStart = 0;
	char findChar = findCharIn;

	for (a = 0; a < stringLen; a ++) {
		addIndent = 0;
		switch (inString[a]) {
			case '{':
			case '[':
			case '(':
			addIndent = 1;
			break;

			case '}':
			case ']':
			case ')':
			if (! (indent --)) {
				errorBox (ERRORTYPE_PROJECTERROR, "Unexpected }, ] or ) in", inString, NULL);
				indent ++;
//				return NULL;
			}
			break;
		}
		if ((inString[a] == findChar) && (indent == 0)) {
			addToStringArray (newStringArray, inString, lastStart, a, thenTrim);
			lastStart = a + 1;
			if (howMany == ONCE) findChar = NULL;
		}
		indent += addIndent;
	}
	addToStringArray (newStringArray, inString, lastStart, stringLen, thenTrim);

	return newStringArray;
}

BOOL destroyFirst (stringArray * & theArray) {
	stringArray * killMe = theArray;
	if (theArray) {
//   	printf ("Destroying %s\n", killMe -> string);
		theArray = theArray -> next;
		delete killMe -> string;
		delete killMe;
	}

	return (BOOL) (theArray != NULL);
}
/*
void displayAllInArray (stringArray * sA) {
	if (sA) {
		while (sA) {
			printf ("[%s]%s", sA -> string, sA -> next ? ", " : "\n");
			sA = sA -> next;
		}
	} else {
   	printf ("Nothing in this stringArray!\n");
	}
}
*/

char * returnElement (stringArray * sA, int i) {
	while (i --) {
		sA = sA -> next;
	}
	return sA -> string;
}

int findElement (stringArray * sA, const char * findString) {
	int i = 0;

	while (sA) {
		if (strcmp (sA -> string, findString) == 0) return i;
		i ++;
		sA = sA -> next;
	}
	return -1;
}

int findOrAdd (stringArray * & sA, const char * addString, BOOL trimSpa) {
	int i = findElement (sA, addString);

	if (i == -1) {
		addToStringArray (sA, addString, 0, -1, trimSpa);
		return findElement (sA, addString);
	} else {
		return i;
	}
}

BOOL trimStart (char * & thisString, char trimChar) {
	char * newString;
	if (thisString[0] == trimChar) {
		newString = new char[strlen (thisString)];
		strcpy (newString, thisString + 1);
		delete thisString;
		thisString = newString;
		return TRUE;
	}
	return FALSE;
}

BOOL trimEnd (char * & thisString, char trimChar) {
	int len = strlen (thisString);
	if (thisString[len - 1] == trimChar) {
		thisString[len - 1] = NULL;
		return TRUE;
	}
	return FALSE;
}

void trimEdgeSpace (char * & thisString) {
	while (trimStart (thisString, ' ')) {;}
	while (trimEnd (thisString, ' ')) {;}
}

int countElements (stringArray * sA) {
	int num = 0;

	while (sA) {
		sA = sA -> next;
		num ++;
	}
	return num;
}

char * joinStrings (const char * a, const char * b) {
	char * nS = new char[strlen (a) + strlen (b) + 1];
	if (nS) sprintf (nS, "%s%s", a, b);
	return nS;
}

char * joinStrings (const char * a, const char * b, const char * c) {
	char * nS = new char[strlen (a) + strlen (b) + strlen (c) + 1];
	if (nS) sprintf (nS, "%s%s%s", a, b, c);
	return nS;
}

char * joinStrings (const char * a, const char * b, const char * c, const char * d) {
	char * nS = new char[strlen (a) + strlen (b) + strlen (c) + strlen (d) + 1];
	if (nS) sprintf (nS, "%s%s%s%s", a, b, c, d);
	return nS;
}

char * copyString (const char * a) {
	char * nS = new char[strlen (a) + 1];
	if (nS) strcpy (nS, a);
	return nS;
}
