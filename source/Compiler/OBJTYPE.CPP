#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "interface.h"
#include "SPLITTER.HPP"
#include "SLUDGE_Functions.H"
#include "settings.h"
#include "ALLKNOWN.H"
#include "MOREIO.H"
#include "REALPROC.H"
#include "MessBox.h"
#include "checkUsed.h"

stringArray * objectTypeNames = NULL;
extern stringArray * functionNames;
extern char * inThisClass;
extern char * emptyString;

stringArray * allKnownFlags = NULL;

unsigned short handleFlags (char * instring) {
	int f = 0;
	stringArray * bits = splitString (instring, ' ');
	while (bits) {
		trimEnd (bits->string, ',');
		if (bits->string[0]) {
			int index = findElement (allKnownFlags, bits->string);
			if (index == -1) {
				if (countElements (allKnownFlags) >= 16)
					addComment (ERRORTYPE_PROJECTERROR, "Already got 16 object flags specified, so can't add new flag", bits->string, NULL);
				else if (checkNotKnown (bits->string, NULL)) {
					addToStringArray (allKnownFlags, bits->string);
					index = findElement (allKnownFlags, bits->string);
				}
			}
			if (index >= 0) {
				f |= 1<<index;
			}
		}
		destroyFirst (bits);
	}
	return f;
}

bool createObjectType (char * code, const char * fName, stringArray * & globalVars, compilationSpace & globalSpace, char * originalName) {
	stringArray * getContents = splitString (code, '{', ONCE);
	stringArray * getBits, * getBitType, * getBitInsides;
	char * objNam;
	char fbuff[15];
	int r = 255, g = 0, b = 0, speechGap = 8, i, numCombis = 0, walkSpeed = 5;
	int aaOnOff = chrRenderingSettings.defEnabled, aaBlurX = chrRenderingSettings.defSoftnessX, aaBlurY = chrRenderingSettings.defSoftnessY;
	char * keepName;
	int wrapSpeech = 30, spinSpeed = 0;
	unsigned short flags = 0;

//	printf ("FOUND AN OBJECT TYPE! \"%s\"\n", getContents -> string);
	getBits = splitString (getContents -> string, '(', ONCE);

//	printf ("New object type: \"%s\"\n", getBits -> string);

	char * displayName = joinStrings (getBits -> string, "");
	inThisClass = joinStrings (displayName, ".");
	setCompilerText (COMPILER_TXT_ITEM, displayName);
	objNam = joinStrings (getBits -> string, ".");

	if (! checkNotKnown (getBits -> string, fName)) return false;
	keepName = joinStrings ("", getBits -> string);
	addToStringArray (objectTypeNames, getBits -> string);
	i = findElement (objectTypeNames, getBits -> string);
	sprintf (fbuff, "obj%05i.ob~", i);

	if (! destroyFirst (getBits)) return addComment (ERRORTYPE_PROJECTERROR, "Bad object type definition (no on-screen text in brackets)", code, fName);
	if (! trimEnd (getBits -> string, ')')) return addComment (ERRORTYPE_PROJECTERROR, "Bad object type definition (no on-screen text in brackets)", code, fName);

	// Good grief, what a horrible way to do this!

	trimEdgeSpace (getBits -> string);
	if (! trimStart (getBits -> string, '_')) return addComment (ERRORTYPE_PROJECTERROR, "Not a fixed string", getBits -> string, fName);
	if (! trimStart (getBits -> string, 's')) return addComment (ERRORTYPE_PROJECTERROR, "Not a fixed string", getBits -> string, fName);
	if (! trimStart (getBits -> string, 't')) return addComment (ERRORTYPE_PROJECTERROR, "Not a fixed string", getBits -> string, fName);
	if (! trimStart (getBits -> string, 'r')) return addComment (ERRORTYPE_PROJECTERROR, "Not a fixed string", getBits -> string, fName);
	if (! trimStart (getBits -> string, 'i')) return addComment (ERRORTYPE_PROJECTERROR, "Not a fixed string", getBits -> string, fName);
	if (! trimStart (getBits -> string, 'n')) return addComment (ERRORTYPE_PROJECTERROR, "Not a fixed string", getBits -> string, fName);
	if (! trimStart (getBits -> string, 'g')) return addComment (ERRORTYPE_PROJECTERROR, "Not a fixed string", getBits -> string, fName);

	int stringValue = stringToInt (getBits -> string, ERRORTYPE_PROJECTERROR);
//	printf ("Shown on-screen as string: [%i]\n", stringValue);

	if (! gotoTempDirectory ()) return false;
	FILE * objFile = fopen (fbuff, "wb");
	if (! objFile) return addComment (ERRORTYPE_SYSTEMERROR, "Can't write object type file", fbuff, NULL);
	writeString (keepName, objFile);
	writeString (fName, objFile);
	delete keepName;
	put2bytes (stringValue, objFile);

	if (destroyFirst (getBits)) return addComment (ERRORTYPE_INTERNALERROR, "OT1: What's going on?", getBits -> string, NULL);

	if (! destroyFirst (getContents)) return addComment (ERRORTYPE_PROJECTERROR, "Bad object type definition (no opening squirly brace)", code, fName);
	if (! trimEnd (getContents -> string, '}')) return addComment (ERRORTYPE_PROJECTERROR, "No object type closing squirly brace", getContents -> string, fName);

	getBits = splitString (getContents -> string, ';');
	do {
		if (getBits -> string[0]) {
			getBitType = splitString (getBits -> string, ' ', ONCE);
			if (strcmp ("event", getBitType -> string) == 0) {
				if (! destroyFirst (getBitType)) return addComment (ERRORTYPE_PROJECTERROR, "Syntax error in event element of objectType", getBitType -> string, fName);
				getBitInsides = splitString (getBitType -> string, '{', ONCE);
				if (countElements (getBitInsides) > 1) {
					if (! trimEnd (getBitInsides -> next -> string, '}')) return addComment (ERRORTYPE_PROJECTERROR, "No closing {}", getBitInsides -> next -> string, fName);
					char * newFuncName = joinStrings (objNam, getBitInsides -> string);
//					printf ("Combination: %s %s\n", getBitInsides -> string, objNam);
					writeString (getBitInsides -> string, objFile);
					writeString (newFuncName, objFile);
					if (! destroyFirst (getBitInsides)) return addComment (ERRORTYPE_PROJECTERROR, "No code for event", newFuncName, fName);

					if (
						defineFunction (newFuncName,					// Name of function
								"",										// Args without ()
						getBitInsides -> string, false, false, fName)	// Code without {}
					< 0 ) return false;

					setCompilerText (COMPILER_TXT_ITEM, displayName);
					destroyAll (getBitInsides);
					delete newFuncName;
					numCombis ++;
				} else {
					destroyFirst (getBitInsides);
					getBitInsides = splitString (getBitType -> string, '=', ONCE);
					if (countElements (getBitInsides) > 1) {
						writeString (getBitInsides -> string, objFile); destroyFirst (getBitInsides);
						writeString (getBitInsides -> string, objFile); destroyFirst (getBitInsides);
						numCombis ++;
					} else {
						return addComment (ERRORTYPE_PROJECTERROR, "Event not followed by = or {", getBitInsides -> string, fName);
					}
				}
				if (destroyFirst (getBitType)) return addComment (ERRORTYPE_PROJECTERROR, "Syntax error in event element of objectType", getBits -> string, fName);

			} else if (strcmp ("sub", getBitType -> string) == 0) {
				if (! destroyFirst (getBitType)) {
					return addComment (ERRORTYPE_PROJECTERROR, "Bad member sub declaration", getBits -> string, fName);
				}
				if (! outdoorSub (getBitType -> string, originalName)) {
					return false;
				}
				destroyFirst (getBitType);

			} else if (strcmp ("var", getBitType -> string) == 0) {
				if (! destroyFirst (getBitType)) return addComment (ERRORTYPE_PROJECTERROR, "Bad member variable definition", getBits -> string, fName);
				globalVar (getBitType -> string, globalVars, globalSpace, fName);
				destroyFirst (getBitType);

			} else if ((strcmp ("speechColour", getBitType -> string) == 0) ||
					   (strcmp ("speechColor", getBitType -> string) == 0)) {
				if (! destroyFirst (getBitType)) return addComment (ERRORTYPE_PROJECTERROR, "Syntax error in speechColour element of objectType", getBits -> string, fName);
				getBitInsides = splitString (getBitType -> string, ',');
				if (countElements (getBitInsides) != 3) return addComment (ERRORTYPE_PROJECTERROR, "speechColour syntax is \"n, n, n\"", getBitType -> string, fName);
				r = stringToInt (getBitInsides -> string, ERRORTYPE_PROJECTERROR);
				destroyFirst (getBitInsides);
				g = stringToInt (getBitInsides -> string, ERRORTYPE_PROJECTERROR);
				destroyFirst (getBitInsides);
				b = stringToInt (getBitInsides -> string, ERRORTYPE_PROJECTERROR);
				destroyFirst (getBitInsides);
				if (r < 0 || g < 0 || b < 0)
					return addComment (ERRORTYPE_PROJECTERROR, "Red, green and blue values in speechColour should all be positive integers", getBits->string, fName);

			} else if ((strcmp ("antiAlias", getBitType -> string) == 0)) {
				if (! destroyFirst (getBitType)) return addComment (ERRORTYPE_PROJECTERROR, "Syntax error in antiAlias element of objectType", getBits -> string, fName);
				getBitInsides = splitString (getBitType -> string, ',');
				if (countElements (getBitInsides) != 3) return addComment (ERRORTYPE_PROJECTERROR, "antiAlias syntax is \"onOff, blurX, blurY\"", getBitType -> string, fName);
				if (strcmp (getBitInsides -> string, "true") == 0)
				{
					aaOnOff = 1;
				}
				else if (strcmp (getBitInsides->string, "false") == 0)
				{
					aaOnOff = 0;
				}
				else
				{
					aaOnOff = -1;
				}
				destroyFirst (getBitInsides);
				aaBlurX = stringToInt (getBitInsides -> string, ERRORTYPE_PROJECTERROR);
				destroyFirst (getBitInsides);
				aaBlurY = stringToInt (getBitInsides -> string, ERRORTYPE_PROJECTERROR);
				destroyFirst (getBitInsides);
				if (aaOnOff < 0 || aaOnOff > 1)
					return addComment (ERRORTYPE_PROJECTERROR, "Anti-aliasing settings: First parameter (on/off value) should either be true or false", getBits->string, fName);
				if (aaBlurX < 0 || aaBlurY < 0)
					return addComment (ERRORTYPE_PROJECTERROR, "Anti-aliasing settings: X and Y blur values should be positive integers", getBits->string, fName);

			} else if (strcmp ("speechGap", getBitType -> string) == 0) {
				if (! destroyFirst (getBitType)) return addComment (ERRORTYPE_PROJECTERROR, "Syntax error in speechGap element of objectType", getBits -> string, fName);
				speechGap = stringToInt (getBitType -> string, ERRORTYPE_PROJECTERROR);

			} else if (strcmp ("flag", getBitType -> string) == 0 ||
					   strcmp ("flags", getBitType -> string) == 0) {
				if (! destroyFirst (getBitType)) return addComment (ERRORTYPE_PROJECTERROR, "Syntax error in flags element of objectType", getBits -> string, fName);
				flags |= handleFlags (getBitType -> string);

			} else if (strcmp ("walkSpeed", getBitType -> string) == 0) {
				if (! destroyFirst (getBitType)) return addComment (ERRORTYPE_PROJECTERROR, "Syntax error in walkSpeed element of objectType", getBits -> string, fName);
				walkSpeed = stringToInt (getBitType -> string, ERRORTYPE_PROJECTERROR);

			} else if (strcmp ("spinSpeed", getBitType -> string) == 0) {
				if (! destroyFirst (getBitType)) return addComment (ERRORTYPE_PROJECTERROR, "Syntax error in spinSpeed element of objectType", getBits -> string, fName);
				spinSpeed = stringToInt (getBitType -> string, ERRORTYPE_PROJECTERROR);

			} else if (strcmp ("wrapSpeech", getBitType -> string) == 0) {
				if (! destroyFirst (getBitType)) return addComment (ERRORTYPE_PROJECTERROR, "Syntax error in wrapSpeech element of objectType", getBits -> string, fName);
				wrapSpeech = stringToInt (getBitType -> string, ERRORTYPE_PROJECTERROR);

			} else if (strcmp ("wrapSpeechPixels", getBitType -> string) == 0) {
				if (! destroyFirst (getBitType)) return addComment (ERRORTYPE_PROJECTERROR, "Syntax error in wrapSpeechPixels element of objectType", getBits -> string, fName);
				wrapSpeech = - stringToInt (getBitType -> string, ERRORTYPE_PROJECTERROR);

			} else {
				return addComment (ERRORTYPE_PROJECTERROR, "Only \"event\", \"speechGap\", \"walkSpeed\" and \"speechColour\" elements are allowed in an objectType, not this", getBitType -> string, fName);
			}
		}
	} while (destroyFirst (getBits));

	destroyAll (getContents);

	delete objNam;
	delete displayName;

	// Forget "class"... no more member variables
	delete inThisClass;
	inThisClass = emptyString;

	fclose (objFile);
	sprintf (fbuff, "obj%05i.nu~", i);
	objFile = fopen (fbuff, "wb");
	if (! objFile) return addComment (ERRORTYPE_SYSTEMERROR, "Can't write object num file", fbuff, NULL);

	put2bytes (numCombis, objFile);

	fputc (r, objFile);
	fputc (g, objFile);
	fputc (b, objFile);
	fputc (speechGap, objFile);
	fputc (walkSpeed, objFile);
	put4bytes (wrapSpeech, objFile);
	put2bytes (spinSpeed, objFile);

	fputc (aaOnOff, objFile);
	putFloat (aaBlurX / 16.f, objFile);
	putFloat (aaBlurY / 16.f, objFile);

	put2bytes (flags, objFile);

	fclose (objFile);

	return true;
}

bool linkObjectFile (FILE * mainFile, FILE * indexFile, int i, unsigned long calcIndexSize) {
	char fbuff[15];
	FILE * fileIn, * theNumFile;
	int a, f;
	char * objNam, * funcToCall;

	put4bytes (ftell (mainFile) + calcIndexSize, indexFile);

	if (! gotoTempDirectory ()) return false;
	sprintf (fbuff, "obj%05i.nu~", i);
	theNumFile = fopen (fbuff, "rb");
	if (! theNumFile) return addComment (ERRORTYPE_SYSTEMERROR, "Can't read object num file", fbuff, NULL);
	sprintf (fbuff, "obj%05i.ob~", i);
	fileIn = fopen (fbuff, "rb");
	if (! fileIn) return addComment (ERRORTYPE_SYSTEMERROR, "Can't read object type file", fbuff, NULL);

	objNam = readString (fileIn);
	setCompilerText (COMPILER_TXT_ITEM, objNam);
	char * fileNam = readString (fileIn);
	setCompilerText (COMPILER_TXT_FILENAME, fileNam);

	for (a = 0; a < 2; a ++) {
		fputc (fgetc (fileIn), mainFile);
	}

	// Number of combinations
	int numCom = get2bytes (theNumFile);

	// Copy RGB, speechGap, walkSpeed, wrapSpeech, spinSpeed and flag values
	for (;;)
	{
		int ch = fgetc(theNumFile);
		if (! feof (theNumFile))
		{
			fputc (ch, mainFile);
		}
		else
		{
			break;
		}
	}

#if 0
	{
		char buff[100];
		sprintf (buff, "%s has %i combination(s)", objNam, numCom);
		addComment (3, buff, NULL);
	}
#endif

	delete objNam;
	delete fileNam;
	fclose (theNumFile);

	put2bytes (numCom, mainFile);
	while (numCom --) {
		objNam = readString (fileIn);
		funcToCall = readString (fileIn);

		f = findElement (objectTypeNames, objNam);
		if (f == -1) return addComment (ERRORTYPE_PROJECTERROR, "Not an object name", objNam, NULL);
		put2bytes (f, mainFile);

		f = findElement (functionNames, funcToCall);
		if (f == -1) return addComment (ERRORTYPE_PROJECTERROR, "Not a function name", funcToCall, NULL);
		put2bytes (f, mainFile);

		setUsed (CHECKUSED_FUNCTIONS, f);

		delete objNam;
		delete funcToCall;
	}
	fclose (fileIn);
	sprintf (fbuff, "obj%05i.nu~", i);
	unlink (fbuff);
	sprintf (fbuff, "obj%05i.ob~", i);
	unlink (fbuff);
	return true;
}

