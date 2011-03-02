#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "messbox.h"
#define MAXINT 32767

#include "splitter.hpp"
#include "stringarray.h"

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
