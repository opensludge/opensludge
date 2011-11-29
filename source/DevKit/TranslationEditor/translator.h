/*
 *  Translator.h
 *  Sludge Dev Kit
 *
 *  Created by Rikard Peterson on 2009-08-06.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif	
	

enum lineType {TYPE_NEW, TYPE_NONE, TYPE_TRANS, TYPE_SEARCH};


struct transLine {
	char * transFrom, * transTo;
	enum lineType type;
	char exists;
	struct transLine * next;
};

bool loadTranslationFile (char * fileIn, struct transLine ** firstTransLine, char **langName, unsigned int *lanID);
bool saveTranslationFile (const char * filename, struct transLine * firstTransLine, const char *langName, unsigned int lan);
bool updateFromProject (const char * filename, struct transLine **firstTransLine);
void newFile (struct transLine ** firstTransLine);

#ifdef __cplusplus
}
#endif
