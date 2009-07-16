/*
 *  Compiler.cpp
 *  Sludge Dev Kit
 *
 *  Created by Rikard Peterson on 2009-07-16.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>


#include "Compiler.hpp"
#include "AllKnown.h"
#include "CheckUsed.h"
#include "DumpFiles.h"
#include "Function.h"
#include "Linker.h"
#include "MessBox.h"
#include "PreProc.h"
#include "RealProc.h"
#include "Splitter.hpp"
#include "Translation.h"
#include "MoreIO.h"
#include "settings.h"
#include "Registry.h"
#include "Backdrop.h"
#include "ObjType.h"

enum { CSTEP_INIT,
	   CSTEP_PARSE,
	   CSTEP_COMPILEINIT,
	   CSTEP_COMPILE,
	   CSTEP_AFTERCOMPILE,
	   CSTEP_LINKSCRIPTS,
	   CSTEP_AFTERLINKSCRIPTS,
	   CSTEP_LINKOBJECTS,
	   CSTEP_AFTERLINKOBJECTS,
	   CSTEP_DUMPFILES,
	   CSTEP_DONE,
	   CSTEP_ERROR };

char * stageName[] = {
	"Initialisation", "Parsing", "Precompile", "Compiling",
	"Prelink", "Linking scripts", "Linking objects", "Linking objects",
	"Converting resources", "Attaching resources", "Done", "Compiliation aborted!"
};

static int compileStep = CSTEP_INIT;
static stringArray * globalVarNames = NULL;

extern char * loadedFile;
static FILE * projectFile;
static stringArray * allSourceStrings = NULL;
static stringArray * allTheFunctionNamesTemp = NULL;
static compilationSpace globalSpace;

static FILE * tempIndex;
static FILE * tempData;
static unsigned long iSize;
static int tot;
static bool runMe = false;


extern stringArray * allFileHandles;
extern int numStringsFound;
extern int numFilesFound;
extern stringArray * functionNames;
extern stringArray * objectTypeNames;


static int data1 = 0, numProcessed = 0;

char * errorTypeStrings[ERRORTYPE_NUM] =
{
	"WARNING: ",
	"ERROR: ",
	"SYSTEM ERROR: ",
	"INTERNAL ERROR: "
};

struct errorLinkToFile
{
	int errorType;
	char * overview;
	char * filename;
	int lineNumber;
	errorLinkToFile * next;
};

errorLinkToFile * errorList = NULL;
int numErrors = 0;


void addComment (int errorType, const char * comment, const char * filename/*, int lineNumber*/) {
	
	fprintf (stderr, "addComment: %s %s\n", comment, filename);
	
	if (filename && filename[0] == '\0')
		filename = NULL;
	
	errorLinkToFile * newLink = new errorLinkToFile;
	
	if (newLink)
	{
		newLink->errorType = errorType;
		newLink->overview = copyString (comment);
		newLink->filename = filename ? copyString (filename) : NULL;
		newLink->lineNumber = 0;
		newLink->next = errorList;
		errorList = newLink;
		
		char * after = filename ? joinStrings (" (in ", filename, ")") : copyString ("");
		char * wholeThing = joinStrings (errorTypeStrings[errorType], comment, after);
//		SendDlgItemMessage(warningWindowH, ID_WARNINGLIST, LB_ADDSTRING, (WPARAM) 0, (LPARAM) wholeThing);
//		SendDlgItemMessage(warningWindowH, ID_WARNINGLIST, LB_SELECTSTRING, (WPARAM) 0, (LPARAM) wholeThing);
		delete after;
		delete wholeThing;
//		ShowWindow (warningWindowH, SW_SHOW);
		
		numErrors ++;
	}
}


void runCompiledGame () {
	char * wholePath = new char[strlen (sourceDirectory) + strlen (settings.finalFile) + 20];
	if (wholePath)
	{
		sprintf (wholePath, "%s\\%s%s", sourceDirectory, settings.finalFile, settings.forceSilent ? " (silent).slg" : ".slg");
		//		errorBox (ERRORTYPE_SYSTEMERROR, "Execute", wholePath, NULL);
		//TODO unsigned long reply = (unsigned long) ShellExecute (h, "open", wholePath, NULL, NULL, SW_SHOWNORMAL);
		//		errorBox (ERRORTYPE_SYSTEMERROR, "Got reply", wholePath, NULL);
		
		/*
		if (reply <= 31)
		{
			errorBox (ERRORTYPE_SYSTEMERROR, "Compiled OK, but can't determine the location of the SLUDGE engine on this machine...", NULL, NULL);
			errorBox (ERRORTYPE_SYSTEMERROR, "You HAVE installed the SLUDGE engine, haven't you?", NULL, NULL);
			errorBox (ERRORTYPE_SYSTEMERROR, "If not, please visit http://www.hungrysoftware.com/tools/sludge/ and download it.", NULL, NULL);
			errorBox (ERRORTYPE_SYSTEMERROR, "You need only do this once - as soon as you've got a copy, your games will run automatically. Thanks!", NULL, NULL);
		}
		else
		{
			DestroyWindow (h);
		}*/
		delete wholePath;
	}
}



void setCompileStep (int a, int totalBits)
{
	compileStep = a;
	data1 = 0;
	
	//	errorBox (ERRORTYPE_SYSTEMERROR, "Step", stageName[a], NULL);
	
	//	setWindowText (COM_PROJTEXT, windowName);
	//	setWindowText (COM_PROGTEXT, stageName[a]);
	
	if (a <= CSTEP_DONE)
	{
	//	setWindowText (COM_FILENAME, "");
	//	setWindowText (COM_ITEMTEXT, "");
		
	//	clearRect (CSTEP_DONE, P_TOP);
	//	percRect (a, P_TOP);
	//	clearRect (totalBits, P_BOTTOM);
	}
}


bool doSingleCompileStep () {
	switch (compileStep)
	{
		case CSTEP_INIT:
		setGlobPointer (& globalVarNames);

		if (! getSourceDirFromName (loadedFile)) return errorBox (ERRORTYPE_INTERNALERROR, "Error initialising!", NULL, NULL);

		projectFile = fopen (loadedFile, "rt");
		if (! projectFile) return errorBox (ERRORTYPE_SYSTEMERROR, "Can't read project file", loadedFile, NULL);
		if (! readSettings (projectFile)) return false;
	
		addToStringArray (allSourceStrings, "");
		addToStringArray (allSourceStrings, settings.windowName);
		addToStringArray (allSourceStrings, settings.quitMessage);
		setCompileStep (CSTEP_PARSE, 1);
		break;

		case CSTEP_PARSE:
		{
			char * tx = readText (projectFile);
			if (! tx) {
				setCompileStep (CSTEP_COMPILEINIT, 1);
			} else if (tx[0] && tx[0] != '[') {
				fixPath(tx, true);
				fprintf(stderr, "%s\n ", tx);
				//setWindowText (COM_FILENAME, tx);
				
				char * compareMe = tx + (strlen (tx) - 4);
				char * lowExt = compareMe;
				while (*lowExt) {
					*lowExt = tolower (*lowExt);
					lowExt ++;
				}
				if (strcmp (compareMe, ".sld") == 0) {
					doDefines (tx, allSourceStrings, allFileHandles);
				} else if (strcmp (compareMe, ".slu") == 0) {
					if (! preProcess (tx, numProcessed ++, allSourceStrings, allFileHandles)) 
						return false;
				} else if (strcmp (compareMe, ".tra") == 0) {
					registerTranslationFile (tx);
				} else {
					return errorBox (ERRORTYPE_PROJECTERROR, "What on Earth is this file doing in a project?", tx, NULL);
				}
			}
			delete tx;
		}
		break;
		
		case CSTEP_COMPILEINIT:
		fclose (projectFile);
		numStringsFound = countElements (allSourceStrings);
		numFilesFound = countElements (allFileHandles);
		setCompileStep (CSTEP_COMPILE, numProcessed);
		if (! startFunction (protoFunction ("_globalInitFunction", ""), 0, globalSpace, "_globalInitFunction", true, false, "-"))
			return errorBox (ERRORTYPE_INTERNALERROR, "Couldn't create global variable initialisation code segment", NULL, NULL);
		break;

		case CSTEP_COMPILE:
		if (data1 >= numProcessed)
		{
			setCompileStep (CSTEP_AFTERCOMPILE, 1);
			break;
		}
		
//		percRect (data1, P_BOTTOM);
		if (! realWork (data1, globalVarNames, globalSpace))
			return false;
		data1 ++;
		break;

		case CSTEP_AFTERCOMPILE:

		outputHalfCode (globalSpace, SLU_LOAD_GLOBAL, "init");
		outputDoneCode (globalSpace, SLU_CALLIT, 0);
		finishFunctionNew (globalSpace, NULL);
	
//		setWindowInt (COM_NUM_FUNC,		countElements (functionNames) - 1);
//		setWindowInt (COM_NUM_OBJ,		countElements (objectTypeNames));
//		setWindowInt (COM_NUM_RES, 		countElements (allFileHandles));
//		setWindowInt (COM_NUM_GLOB,		countElements (globalVarNames));
//		setWindowInt (COM_NUM_STRINGS,	countElements (allSourceStrings));

		checkUsedInit (CHECKUSED_FUNCTIONS, countElements (functionNames));
		setUsed(CHECKUSED_FUNCTIONS, 0);

		checkUsedInit (CHECKUSED_GLOBALS, countElements (globalVarNames));
		
		stringArray * silenceChecker = allFileHandles;
		while (silenceChecker) {
			if (audioFile (silenceChecker -> string)) silent = false;
			silenceChecker = silenceChecker -> next;
		}
		projectFile = openFinalFile (".sl~", "wb");
		if (! projectFile) return errorBox (ERRORTYPE_SYSTEMERROR, "Can't open output file for writing", NULL, NULL);
	
		FILE * textFile = getRegSetting ("compilerWriteStrings") ?
			openFinalFile (" text.doc", "wt") : NULL;
	
		writeFinalData (projectFile);
	
		// ADD ICON ------------------------------------
		if (settings.customIcon) {
//			setWindowText (COM_PROGTEXT, "Adding custom icon");
//			setWindowText (COM_FILENAME, settings.customIcon);
			convertTGA (settings.customIcon);
//			setWindowText (COM_ITEMTEXT, "");
			fputc (1, projectFile);
			if (! dumpFileInto (projectFile, settings.customIcon)) {
				fclose (projectFile);
				return errorBox (ERRORTYPE_PROJECTERROR, "Error adding custom icon (file not found or not a valid TGA file)", settings.customIcon, NULL);
			}
		} else {
			fputc (0, projectFile);
		}
		//----------------------------------------------
	
		put2bytes (countElements (globalVarNames), projectFile);

		if (! saveStrings (projectFile, textFile, allSourceStrings)) {
			fclose (projectFile);
			return errorBox (ERRORTYPE_SYSTEMERROR, "Can't save string bank(s)", NULL, NULL);
		}

		if (! gotoTempDirectory ()) {
			fclose (projectFile);
			return false;
		}
		tempIndex = fopen ("SLUDGE1.tmp", "wb");
		tempData = fopen ("SLUDGE2.tmp", "wb");

		allTheFunctionNamesTemp = functionNames;
		iSize = countElements (functionNames) * 4 + ftell (projectFile) + 4;
	
		setCompileStep (CSTEP_LINKSCRIPTS, countElements(functionNames));
		break;
		
		case CSTEP_LINKSCRIPTS:
		if (data1 < countElements(functionNames))
		{
//			percRect (data1, P_BOTTOM);
			if (! runLinker (tempData, tempIndex, data1, globalVarNames, iSize, allSourceStrings))
				return false;
			allTheFunctionNamesTemp = allTheFunctionNamesTemp -> next;
			data1 ++;
		}
		else
		{
			setCompileStep (CSTEP_AFTERLINKSCRIPTS, 1);
		}
		break;
		
		case CSTEP_AFTERLINKSCRIPTS:
		put4bytes (ftell (tempIndex) + ftell (tempData), projectFile);
		fclose (tempIndex);
		fclose (tempData);
		dumpFileInto (projectFile, "SLUDGE1.tmp");
		dumpFileInto (projectFile, "SLUDGE2.tmp");
		setCompileStep (CSTEP_LINKOBJECTS, tot);
		tot = countElements (objectTypeNames);

		tempIndex = fopen ("SLUDGE1.tmp", "wb");
		tempData = fopen ("SLUDGE2.tmp", "wb");

		iSize = tot * 4 + ftell (projectFile) + 4;
		setCompileStep (CSTEP_LINKOBJECTS, tot);

		break;
		
		case CSTEP_LINKOBJECTS:
		if (data1 < tot)
		{
//			percRect (data1, P_BOTTOM);
			if (! linkObjectFile (tempData, tempIndex, data1, iSize)) {
				fclose (projectFile);
				return false;
			}
			data1 ++;
		}
		else
		{
			setCompileStep (CSTEP_AFTERLINKOBJECTS, 0);
		}
		break;
		
		case CSTEP_AFTERLINKOBJECTS:
		put4bytes (ftell (tempIndex) + ftell (tempData), projectFile);
		fclose (tempIndex);
		fclose (tempData);
		dumpFileInto (projectFile, "SLUDGE1.tmp");
		dumpFileInto (projectFile, "SLUDGE2.tmp");
		unlink ("SLUDGE1.tmp");
		unlink ("SLUDGE2.tmp");

		extern stringArray * globalVarFileOrigins;
		extern stringArray * functionFiles;

		warnAboutUnused (CHECKUSED_FUNCTIONS, functionNames, "Function ", functionFiles);
		warnAboutUnused (CHECKUSED_GLOBALS, globalVarNames, "Global variable ", globalVarFileOrigins);

		setCompileStep (CSTEP_DUMPFILES, 1);
		break;
		
		case CSTEP_DUMPFILES:
		if (! dumpFiles (projectFile, allFileHandles)) {
			fclose (projectFile);
			return false;
		}
		fclose (projectFile);
		gotoSourceDirectory ();
		char * fromName = joinStrings (settings.finalFile, ".sl~");
		char * toName = joinStrings (settings.finalFile, settings.forceSilent ? " (silent).slg" : ".slg");
		unlink (toName);
		
		if (rename (fromName, toName))
		{
			errorBox (ERRORTYPE_SYSTEMERROR, "Couldn't rename the compiled game file... it's been left with the name", fromName, NULL);
		}
		else
		{

			// Run the compiled program (if we were told to)
			if (runMe)
			{
				runCompiledGame ();
			}
			else
			{
//				EnableWindow (GetDlgItem (compWin, IDOK), true);
			}
		}
		setCompileStep (CSTEP_DONE, 0);
		break;
	}

	return true;
}



bool compileEverything () {
	
	// 			runMe = true;

	
	if (!loadedFile) return false;
	
	initBuiltInFunc ();

	compileStep = CSTEP_INIT;
	while (compileStep < CSTEP_DONE) {
		fprintf(stderr, "Doing: %s.\n", stageName[compileStep]);
		if (! doSingleCompileStep ())
		{
			fprintf(stderr, "Error");
			setCompileStep (CSTEP_ERROR, 1);
			gotoSourceDirectory ();
			char * killName = joinStrings (settings.finalFile, ".sl~");
			unlink (killName);
			delete killName;
		}
	}
	fprintf(stderr, "Compiling done.\n");
			
	return true;		
}
