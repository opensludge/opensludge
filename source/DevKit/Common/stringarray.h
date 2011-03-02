#ifndef _STRINGARRAY_H_
#define _STRINGARRAY_H_

#include <stdint.h>

enum splitMode {ONCE, REPEAT};

struct stringArray {
	char * string;
	unsigned int line;
	struct stringArray * next;
};

extern struct stringArray * nullArray;

// Handling a stringArray
void addToStringArray (stringArray * & theArray, const char * theString, int start = 0, int size = -1, bool trimSpa = true);

//void addToStringArray (stringArray * &, const char *, int, int, bool);
bool destroyFirst (stringArray * &);
char * returnElement (stringArray * sA, int i);
int findElement (stringArray * sA, const char * findString);
int findOrAdd (stringArray * & sA, const char * addString, bool = true);
int countElements (stringArray * sA);
#define destroyAll(a) while(destroyFirst(a)){;}

// General char * & trimmage
void trimEdgeSpace (char * & thisString);
bool trimStart (char * & thisString, char trimChar);
bool trimEnd (char * & thisString, char trimChar);

uint32_t readLineNumber (const char * textNumber);

//void displayAllInArray (stringArray *);
//int findElement (stringArray * sA, const char * findString);
//int findOrAdd (stringArray * & sA, const char * addString, bool = true);
//int countElements (stringArray * sA);
//bool trimStart (char * & thisString, char trimChar);
stringArray * returnArray (stringArray * sA, int i);

#endif
