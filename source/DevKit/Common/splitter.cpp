#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "messbox.h"
#define MAXINT 32767

#include "splitter.hpp"

stringArray * nullArray = NULL;


bool trimStart (char * & thisString, char trimChar) {
	char * newString;
	if (thisString[0] == trimChar) {
		newString = new char[strlen (thisString)];
		strcpy (newString, thisString + 1);
		delete thisString;
		thisString = newString;
		return true;
	}
	return false;
}

bool trimEnd (char * & thisString, char trimChar) {
	int len = strlen (thisString);
	if (! len) return 0;
	if (thisString[len - 1] == trimChar) {
		thisString[len - 1] = NULL;
		return true;
	}
	return false;
}

void trimEdgeSpace (char * & thisString) {
	while (trimStart (thisString, ' ')){;}
	while (trimEnd (thisString, ' ')){;}
}

uint32_t readLineNumber (const char * textNumber) {
	uint32_t i = 0;
	int ac = 0;
	
	for (ac = 0; ac<5; ac++) {
		if (textNumber[ac] >= '0' && textNumber[ac] <= '9') {
			i = (i * 10) + textNumber[ac] - '0';
		}
	}
	
	return i;
}

void addToStringArray (stringArray * & theArray, const char * theString, int start, int size, bool trimSpa) {
	char * addMe;
	stringArray * newSection;
	stringArray * huntArray = theArray;

	unsigned int lineNum=0;
	
	if (! theString) return;
	
	if (size == -1) size = strlen (theString);
	size -= start;

	if (theString[start] == 1 && size >= 6) {
		lineNum = readLineNumber(theString+start+1);
		size-=6;
		start+=6;
	}
	
	if (trimSpa) {
		while (size && theString[start] == ' ') {
			start++;
			size--;
		}
		while (size && theString[start+size-1] == ' ') {
			size--;
		}
	}

	// Allocate memory
	addMe = new char[size + 1];
	//	checkNew (addMe);
	newSection = new stringArray;
	//	checkNew (newSection);
	
	// Create new stringArray section
	memcpy(addMe, theString+start, size);
	addMe[size] = NULL;

	newSection -> string = addMe;	
	newSection -> line = lineNum;
	newSection -> next = NULL;

	// Add it

	if (theArray) {
		while (huntArray -> next) {
			huntArray = huntArray -> next;
		}
		huntArray -> next = newSection;
	} else {
		theArray = newSection;
	}
}

bool destroyFirst (stringArray * & theArray) {
	if (! theArray) return false;
	stringArray * killMe = theArray;
	if (theArray) {
		theArray = theArray -> next;
		delete killMe -> string;
		delete killMe;
	}

	return (bool) (theArray != NULL);
}

char * joinStrings (char * a, char * b) {
	char * nS = new char[strlen (a) + strlen (b) + 1];
//	checkNew (nS);
	sprintf (nS, "%s%s", a, b);
	return nS;
}

char * returnElement (stringArray * sA, int i) {
	while (i -- && sA) {
		sA = sA -> next;
	}
	return sA -> string;
}

stringArray * returnArray (stringArray * sA, int i) {
	while (i -- && sA) {
		sA = sA -> next;
	}
	return sA;
}



int32_t stringToInt (const char * textNumber, int errorType) {
	uint32_t i = 0;
	int ac = 0;

	while (textNumber[ac]) {
		if (textNumber[ac] >= '0' && textNumber[ac] <= '9') {
			i = (i * 10) + textNumber[ac] - '0';
			if (i == 65535)
				return 65535;
			if (i > MAXINT) {
				addComment (errorType, "Number too large", textNumber, NULL, 0);
				return -1;
			}
		} else {
			addComment (errorType, "Oh no! I thought this was going to be a number, but there's a non-numerical character in it", textNumber, NULL, 0);
			return -1;
		}
		ac ++;			
	}
	
	return (int32_t) i;
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
				addComment (ERRORTYPE_PROJECTERROR, "Unexpected }, ] or ) in", inString, NULL, 0);
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

stringArray * splitString (const char * inString, const char findCharIn, const splitMode howMany, bool thenTrim) {
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
				if (!findChar && findCharIn == '*') {
					int a1 = a-6;
					int lineNum=0;
					while (--a1 >= 0) {
						if (inString[a1] == 1) {
							if (newStringArray) 
								lineNum = readLineNumber(inString+a1+1);
							a1+=6;
							break;
						}
					}
					if (a1<0) a1=0;
					for (; a < stringLen; a ++) {
						if (inString[a] == 1) {
							break;
						}
					}
					
					char * errStr = new char [a-a1+1];
					memcpy(errStr, inString+a1, a-a1);
					errStr[a-a1+1] = 0;
					addComment (ERRORTYPE_PROJECTERROR, "Unexpected }, ] or ) in", errStr, 
								(newStringArray)?newStringArray->string : NULL, lineNum);
					delete errStr;
					destroyAll (newStringArray);
					return NULL;
				} else {
					fprintf(stderr, "findChar: %c %d\n", findChar, findChar);
					addComment (ERRORTYPE_PROJECTERROR, "Unexpected }, ] or ) in", inString, NULL, 0);
					indent ++;
				}
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





int findElement (stringArray * sA, const char * findString) {
	int i = 0;

	while (sA) {
		if (strcmp (sA -> string, findString) == 0) return i;
		i ++;
		sA = sA -> next;
	}
	return -1;
}

int findOrAdd (stringArray * & sA, const char * addString, bool trimSpa) {
	int i = findElement (sA, addString);

	if (i == -1) {
		addToStringArray (sA, addString, 0, -1, trimSpa);
		return findElement (sA, addString);
	} else {
		return i;
	}
}

int countElements (stringArray * sA) {
	int num = 0;

	while (sA) {
		sA = sA -> next;
		num ++;
	}
	return num;
}

