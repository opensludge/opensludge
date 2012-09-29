#include <stdio.h>

#include "typedef.h"
#include "splitter.hpp"
#include "moreio.h"
#include "settings.h"
#include "helpers.h"
#include "messbox.h"
#include "dumpfiles.h"
#include "utf8.h"

#define numDoubleCharTypes 13

const char * doubleChangeMe[] = {"&&", "==", "||", "!=", "<=", ">=", "++", "--", "+=", "-=", "*=", "/=", "%="};
const char   doubleBecome  [] = {CHAR_AND, CHAR_EQUALS, CHAR_OR, CHAR_NOT_EQ, CHAR_LESS_EQUAL, CHAR_MORE_EQUAL, CHAR_INCREMENT, CHAR_DECREMENT, CHAR_PLUS_EQ, CHAR_MINUS_EQ, CHAR_MULT_EQ, CHAR_DIV_EQ, CHAR_MOD_EQ};

static unsigned int currentLine;
static bool printLine;
static FILE * outputFile;
static bool printSpace;
static bool spaceLast;

void doLine() {
	if (printLine) {
		fprintf(outputFile, "%c%05d", 1,currentLine);
		printLine = false;
	}
	if (printSpace) {
		fprintf(outputFile, " ");
		printSpace = false;
	}
}

bool preProcess (char * codeFileName, int fileNumber, stringArray * & strings, stringArray * & fileHandles) {
	unsigned char * wholeFile;
	char outputName[13], quoteChar = ' ';
	bool showStringWhenFinished = false;
	int index = 0, stringPosition = 0;
	bool readingQuote = false;
	bool escapeCharNext = false, justAComment = false;
	char grabString[1000];
	int doubleCheckLoop;
	
	spaceLast = true;
	printSpace = false;
	currentLine = 1;
	printLine = false;

	if (! gotoTempDirectory ()) return false;

	sprintf (outputName, "_T%05i.TMP", fileNumber);
	outputFile = fopen (outputName, "wt");
	if (! outputFile) {
		addComment (ERRORTYPE_SYSTEMERROR, "Can't create temporary file!\n\nYou're probably out of disk space or you don't have access to the temporary folder.", NULL);
		return false;
	}

	if (! gotoSourceDirectory ()) return false;
	wholeFile = (unsigned char *) grabWholeFile (codeFileName);
	if (wholeFile == NULL) {
		addComment (ERRORTYPE_PROJECTERROR, "Either this file contains nothing, or it doesn't exist...", codeFileName);
		return false;
	}
	if (! u8_isvalid((char *) wholeFile)) {
		return addComment (ERRORTYPE_PROJECTERROR, "Invalid string found. (It is not UTF-8 encoded.)", NULL, codeFileName, 0);
	}
	
	// Check for BOM (shouldn't really be there in UTF-8, but we don't want to choke on it)
	if (wholeFile[0] == 0xEF && wholeFile[1] == 0xBB && wholeFile[2] == 0xBF)
		index = 3;

	fprintf (outputFile, "%s*", codeFileName);
	fprintf(outputFile, "%c%05d", 1,currentLine);
	while (wholeFile[index]) {
		if (readingQuote) {
			if ((wholeFile[index] == quoteChar) && (! escapeCharNext)) {
				readingQuote = false;
				grabString[stringPosition] = 0;
				stringPosition = 0;
				if (showStringWhenFinished) {
					char buff[1030];
					sprintf (buff, "String in question: \"%s\"", grabString);
					addCommentWithLine  (ERRORTYPE_PROJECTWARNING, buff, codeFileName, currentLine);
				}

				if (quoteChar == '\'' && audioFile (grabString) && settings.forceSilent)
					grabString[0] = 0;

				if (quoteChar == '\"')
				{
					doLine();
					fprintf (outputFile, "_%s%i ", "string", findOrAdd (strings, grabString, false));
				}
				else
				{
					doLine();
					fprintf (outputFile, "_%s%i ", "file", findOrAdd (fileHandles, grabString, false));
				}
			}
			else if ((wholeFile[index] == '\\') && (quoteChar == '\"') && (! escapeCharNext))
			{
				escapeCharNext = true;
			}
			else if (wholeFile[index] == '\n')
			{
				grabString[stringPosition] = 0;
				addComment (ERRORTYPE_PROJECTERROR, "Runaway string", grabString, codeFileName, currentLine);
				return false;
			}
			else
			{
				if (escapeCharNext) {
					if (wholeFile[index] != '\"' && wholeFile[index] != '\\') {
						char buff[255];
						sprintf (buff, "Unrecognised excape character found in a string... I don't know what \"\\%c\" is meant to mean. Tell you what - I'll interpret it as just \"%c\", OK?", wholeFile[index], wholeFile[index]);
						showStringWhenFinished = true;
						addCommentWithLine(ERRORTYPE_PROJECTWARNING, buff, codeFileName, currentLine);
					}
				}
				escapeCharNext = false;
				grabString[stringPosition ++] = wholeFile[index];
			}
		} else if (justAComment) {
			if (wholeFile[index] == '\n') {
				if (currentLine) {
					if (++currentLine >= 65535)
						currentLine=0;
				}
				justAComment = false;
			}
		} else {
			switch (wholeFile[index]) {
				case '\"':
				case '\'':
					if (! spaceLast) {
						printSpace = true;
					}
				quoteChar = wholeFile[index];
				readingQuote = true;
				showStringWhenFinished = false;
				spaceLast = false;
				break;

				case '\n':
					if (currentLine) {
						if (++currentLine >= 65535)
							currentLine=0;
					}
					// No break!
				case '\r':
				case ' ':
				case '\t':
					if (! spaceLast) {
						printSpace = true;
					}
				spaceLast = true;
				break;

				case '}':
				spaceLast = false;
				doLine();
				fprintf (outputFile, "};");
				printLine = true;
				break;

				case '#':
				justAComment = true;
				break;

				case '{':
				case '(':
				case '[':
					if (!spaceLast) {
						printSpace = true;
					}
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
					
				doLine();
				fprintf (outputFile, "%c", wholeFile[index]);
				if (wholeFile[index] == ';') 	{	
					printLine = true;
				}
					
				break;
			}
		}
		index ++;
	}

	delete wholeFile;
	fclose (outputFile);
	return true;
}
