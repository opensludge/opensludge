#include <stdio.h>

#include "typedef.h"
#include "splitter.h"
#include "moreio.h"
#include "settings.h"
#include "messbox.h"
#include "winbox.h"
#include "dumpfiles.h"

#define numDoubleCharTypes 13

const char * doubleChangeMe[] = {"&&", "==", "||", "!=", "<=", ">=", "++", "--", "+=", "-=", "*=", "/=", "%="};
const char   doubleBecome  [] = {CHAR_AND, CHAR_EQUALS, CHAR_OR, CHAR_NOT_EQ, CHAR_LESS_EQUAL, CHAR_MORE_EQUAL, CHAR_INCREMENT, CHAR_DECREMENT, CHAR_PLUS_EQ, CHAR_MINUS_EQ, CHAR_MULT_EQ, CHAR_DIV_EQ, CHAR_MOD_EQ};


bool preProcess (char * codeFileName, int fileNumber, stringArray * & strings, stringArray * & fileHandles) {
	FILE * outputFile;
	char * wholeFile;
	char outputName[13], quoteChar = ' ';
	bool showStringWhenFinished = false;
	int index = 0, stringPosition = 0;
	bool readingQuote = false, spaceLast = true;
	bool escapeCharNext = false, justAComment = false;
	char grabString[1000];
	int doubleCheckLoop;
	
	if (! gotoTempDirectory ()) return false;
	
	sprintf (outputName, "_T%05i.TMP", fileNumber);
	outputFile = fopen (outputName, "wt");
	if (! outputFile) {
		addComment (ERRORTYPE_SYSTEMERROR, "Can't create temporary file!\n\nPerhaps you've left another SLUDGE compiler window open - if so, please close it (and this one!) and then try compiling again.\n\nOtherwise, either you're out of disk space or you don't have access to the temporary folder.", NULL);
		return false;
	}

	if (! gotoSourceDirectory ()) return false;
	wholeFile = grabWholeFile (codeFileName);
	if (wholeFile == NULL) {
		addComment (ERRORTYPE_PROJECTERROR, "Either this file contains nothing, or it doesn't exist...", codeFileName);
		return false;
	}

	fprintf (outputFile, "%s*", codeFileName);
	while (wholeFile[index]) {
		if (readingQuote) {
			if ((wholeFile[index] == quoteChar) && (! escapeCharNext)) {
				readingQuote = false;
				grabString[stringPosition] = NULL;
				stringPosition = 0;
				if (showStringWhenFinished) {
					char buff[1030];
					sprintf (buff, "String in question: \"%s\"", grabString);
					addComment (ERRORTYPE_PROJECTWARNING, buff, codeFileName);
				}

				if (quoteChar == '\'' && audioFile (grabString) && settings.forceSilent)
					grabString[0] = NULL;

				if (quoteChar == '\"')
				{
					fprintf (outputFile, "_%s%i ", "string", findOrAdd (strings, grabString, false));
				}
				else
				{
					fprintf (outputFile, "_%s%i ", "file", findOrAdd (fileHandles, grabString, false));
				}
			}
			else if ((wholeFile[index] == '\\') && (quoteChar == '\"') && (! escapeCharNext))
			{
				escapeCharNext = true;
			}
			else if (wholeFile[index] == '\n')
			{
				grabString[stringPosition] = NULL;
				addComment (ERRORTYPE_PROJECTERROR, "Runaway string", grabString, codeFileName);
				return false;
			}
			else
			{
				if (escapeCharNext) {
					if (wholeFile[index] != '\"' && wholeFile[index] != '\\') {
						char buff[255];
						sprintf (buff, "Unrecognised excape character found in a string... I don't know what \"\\%c\" is meant to mean. Tell you what - I'll interpret it as just \"%c\", OK?", wholeFile[index], wholeFile[index]);
						showStringWhenFinished = true;
						addComment (ERRORTYPE_PROJECTWARNING, buff, codeFileName);
					}
				}
				escapeCharNext = false;
				grabString[stringPosition ++] = wholeFile[index];
			}
		} else if (justAComment) {
			if (wholeFile[index] == '\n') justAComment = false;
		} else {
			switch (wholeFile[index]) {
				case '\"':
				case '\'':
				if (! spaceLast) fprintf (outputFile, " ");
				quoteChar = wholeFile[index];
				readingQuote = true;
				showStringWhenFinished = false;
				spaceLast = false;
				break;

				case ' ':
				case '\r':
				case '\n':
				case '\t':
				if (! spaceLast) fprintf (outputFile, " ");
				spaceLast = true;
				break;

				case '}':
				spaceLast = false;
				fprintf (outputFile, "};");
				break;

				case '#':
				justAComment = true;
				break;

				case '{':
				case '(':
				case '[':
				if (!spaceLast) fprintf (outputFile, " ");
				// No break!

				default:
				spaceLast = false;
				for (doubleCheckLoop = 0; doubleCheckLoop < numDoubleCharTypes; doubleCheckLoop ++) {
					if ((wholeFile[index]     == doubleChangeMe[doubleCheckLoop][0]) &&
						 (wholeFile[index + 1] == doubleChangeMe[doubleCheckLoop][1])) {
						index ++;
						wholeFile[index] = doubleBecome[doubleCheckLoop];
						break;
					}
				}
				fprintf (outputFile, "%c", wholeFile[index]);
				break;
			}
		}
		index ++;
	}

	delete wholeFile;
	fclose (outputFile);
	return true;
}
