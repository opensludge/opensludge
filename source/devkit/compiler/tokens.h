enum tokenType {TOK_UNKNOWN, TOK_SUB, TOK_VAR, TOK_OBJECTTYPE,
				TOK_IF, TOK_NOT, TOK_ELSE, TOK_RETURN, TOK_LOOP, TOK_WHILE,
                TOK_FOR, TOK_UNFREEZABLE, TOK_DEBUG,
                TOK_FLAG, TOK_FLAGS,
				numTokens};

tokenType getToken (char * inText);
