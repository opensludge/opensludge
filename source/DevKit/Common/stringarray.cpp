#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "stringarray.h"

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
		thisString[len - 1] = 0;
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
	addMe[size] = 0;

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

