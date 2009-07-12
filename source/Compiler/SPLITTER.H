enum splitMode {ONCE, REPEAT};

struct stringArray {
	char * string;
	stringArray * next;
};

extern stringArray * nullArray;

//-------------//
// PROTOTYPES! //
//-------------//

// Make an entire stringArray from a char *
stringArray * splitString (const char *, const char = ';', const splitMode = REPEAT, bool = true);
stringArray * splitAtLast (const char * inString, const char findCharIn);

// Handling a stringArray
void addToStringArray (stringArray * &, const char *, int = 0, int = -1, bool = true);
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

// Making something with a char *
long stringToInt (const char * textNumber, int errorType);
char * joinStrings (const char * a, const char * b);
char * joinStrings (const char * a, const char * b, const char * c);
char * joinStrings (const char * a, const char * b, const char * c, const char * d);
char * copyString (const char * a);

