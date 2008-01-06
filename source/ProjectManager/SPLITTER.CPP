#include <windows.h>
#include <stdio.h>
#include <string.h>
//#include <values.h>
//#define MAXINT 32767

//#include "typedef.h"
#include "splitter.h"
//#include "fatal.h"

//stringArray * nullArray = NULL;

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
	while (trimStart (thisString, ' ')){;}
	while (trimEnd (thisString, ' ')){;}
}

void addToStringArray (stringArray * & theArray, const char * theString, int start, int size, BOOL trimSpa = TRUE) {
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

stringArray * splitString (const char * inString, const char findCharIn, const splitMode howMany) {
	stringArray * newStringArray = NULL;
	int a, stringLen = strlen (inString), lastStart = 0;
	char findChar = findCharIn;

	for (a = 0; a < stringLen; a ++) {
		if (inString[a] == findChar) {
			addToStringArray (newStringArray, inString, lastStart, a);
			lastStart = a + 1;
			if (howMany == ONCE) findChar = NULL;
		}
	}
	addToStringArray (newStringArray, inString, lastStart, stringLen);

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
int findElement (stringArray * sA, const char * findString) {
	int i = 0;

	while (sA) {
		if (strcmp (sA -> string, findString) == 0) return i;
		i ++;
		sA = sA -> next;
	}
	return -1;
}
*/
/*
int findOrAdd (stringArray * & sA, const char * addString, BOOL trimSpa) {
	int i = findElement (sA, addString);

	if (i == -1) {
		addToStringArray (sA, addString, 0, -1, trimSpa);
		return findElement (sA, addString);
	} else {
		return i;
	}
}
*/

/*int countElements (stringArray * sA) {
	int num = 0;

	while (sA) {
		sA = sA -> next;
		num ++;
	}
	return num;
}
*/
char * joinStrings (char * a, char * b) {
	char * nS = new char[strlen (a) + strlen (b) + 1];
//	checkNew (nS);
	sprintf (nS, "%s%s", a, b);
	return nS;
}
