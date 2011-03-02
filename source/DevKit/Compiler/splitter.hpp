#ifndef _SPLITTER_H_
#define _SPLITTER_H_

#include <stdint.h>
#include "stringarray.h"

// Make an entire stringArray from a char *
stringArray * splitString (const char *, const char = ';', const splitMode = REPEAT, bool = true);
stringArray * splitAtLast (const char * inString, const char findCharIn);

int32_t stringToInt (const char * textNumber, int errorType);

#endif
