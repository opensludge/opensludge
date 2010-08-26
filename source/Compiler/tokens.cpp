#include <string.h>

#include "TOKENS.H"

char * tokText[] = {"", "sub", "var", "objectType", "if", "!", "else", "return",
					"loop", "while", "for", "unfreezable", "debug", "flag","flags"};

tokenType getToken (char * inText) {
	int i;
	for (i = 1; i < numTokens; i ++) {
		if (strcmp (tokText[i], inText) == 0) return (tokenType) i;
	}
	return TOK_UNKNOWN;
}


