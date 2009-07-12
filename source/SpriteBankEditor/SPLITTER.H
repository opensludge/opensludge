struct stringArray {
	char * string;
	stringArray * next;
};

//extern stringArray * nullArray;

enum splitMode {ONCE, REPEAT};

//int stringToInt (const char * textNumber);
stringArray * splitString (const char *, const char = ';', const splitMode = REPEAT);
//void addToStringArray (stringArray * &, const char *, int = 0, int = -1, bool = true);
bool destroyFirst (stringArray * &);
//void displayAllInArray (stringArray *);
//int findElement (stringArray * sA, const char * findString);
//int findOrAdd (stringArray * & sA, const char * addString, bool = true);
//void trimEdgeSpace (char * & thisString);
//int countElements (stringArray * sA);
//bool trimStart (char * & thisString, char trimChar);
//bool trimEnd (char * & thisString, char trimChar);
char * joinStrings (char * a, char * b);

