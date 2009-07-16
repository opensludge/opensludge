#include "Splitter.hpp"

bool checkNotKnown (const char * thisName, const char * filename);
bool checkNotKnown2 (const char * thisName, const char * filename);	// Locals can override globals, function names & objectTypes
void setGlobPointer (stringArray * * g);
bool checkValidName (const char * thisName, const char * error, const char * filename);
