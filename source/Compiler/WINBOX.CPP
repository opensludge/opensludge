#ifdef WIN32

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "wincomp.h"
#include "winbox.h"
#include "winterfa.h"
#include "messbox.h"
#include "splitter.h"
#include "preproc.h"
#include "function.h"
#include "realproc.h"
#include "registry.h"
#include "settings.h"
#include "linker.h"
#include "objtype.h"
#include "moreio.h"
#include "dumpfiles.h"
#include "percbar.h"
#include "wintext.h"
#include "allknown.h"
#include "backdrop.h"
#include "translation.h"
#include "checkUsed.h"

HWND compWin=NULL;
HWND warningWindowH=NULL;

extern HINSTANCE inst;
extern char * windowName;
extern char * quitMessage;
extern char * customIcon;
extern bool runMe;
extern bool forceSilent;
extern char * finalFile;

extern stringArray * functionNames;
extern char * sourceDirectory;

extern stringArray * objectTypeNames;
char * theFile;
extern stringArray * allFileHandles = NULL;

extern int numStringsFound;
extern int numFilesFound;

extern bool silent;

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
static compilationSpace globalSpace;
static int data1 = 0, numProcessed = 0;
static stringArray * allSourceStrings = NULL;
static stringArray * globalVarNames = NULL;
static stringArray * allTheFunctionNamesTemp = NULL;
static FILE * projectFile;
static unsigned long iSize;
static FILE * tempIndex;
static FILE * tempData;
static int tot;

void setCompileStep (int a, int totalBits)
{
	compileStep = a;
	data1 = 0;

//	errorBox (ERRORTYPE_SYSTEMERROR, "Step", stageName[a], NULL);

	setWindowText (COM_PROJTEXT, windowName);
	setWindowText (COM_PROGTEXT, stageName[a]);

	if (a <= CSTEP_DONE)
	{
		setWindowText (COM_FILENAME, "");
		setWindowText (COM_ITEMTEXT, "");
	
		clearRect (CSTEP_DONE, P_TOP);
		percRect (a, P_TOP);
		clearRect (totalBits, P_BOTTOM);
	}
}

void runCompiledGame (HWND h) {
	char * wholePath = new char[strlen (sourceDirectory) + strlen (finalFile) + 20];
	if (wholePath)
	{
		sprintf (wholePath, "%s\\%s%s", sourceDirectory, finalFile, forceSilent ? " (silent).slg" : ".slg");
//		errorBox (ERRORTYPE_SYSTEMERROR, "Execute", wholePath, NULL);
		unsigned long reply = (unsigned long) ShellExecute (h, "open", wholePath, NULL, NULL, SW_SHOWNORMAL);
//		errorBox (ERRORTYPE_SYSTEMERROR, "Got reply", wholePath, NULL);
		
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
		}
		delete wholePath;
	}
}

bool doSingleCompileStep () {
	switch (compileStep)
	{
		case CSTEP_INIT:
		setGlobPointer (& globalVarNames);

		if (! getSourceDirFromName (theFile)) return errorBox (ERRORTYPE_INTERNALERROR, "Error initialising!", NULL, NULL);

		projectFile = fopen (theFile, "rt");
		if (! projectFile) return errorBox (ERRORTYPE_SYSTEMERROR, "Can't read project file", theFile, NULL);
		if (! readSettings (projectFile)) return false;
	
		addToStringArray (allSourceStrings, "");
		addToStringArray (allSourceStrings, windowName);
		addToStringArray (allSourceStrings, quitMessage);
		setCompileStep (CSTEP_PARSE, 1);
		break;

		case CSTEP_PARSE:
		{
			char * tx = readText (projectFile);
			if (! tx) {
				setCompileStep (CSTEP_COMPILEINIT, 1);
			} else if (tx[0] && tx[0] != '[') {
				setWindowText (COM_FILENAME, tx);
				char * compareMe = tx + (strlen (tx) - 4);
				char * lowExt = compareMe;
				while (*lowExt) {
					*lowExt = tolower (*lowExt);
					lowExt ++;
				}
				if (strcmp (compareMe, ".sld") == 0) {
					doDefines (tx, allSourceStrings, allFileHandles);
				} else if (strcmp (compareMe, ".slu") == 0) {
					if (! preProcess (tx, numProcessed ++, allSourceStrings, allFileHandles)) return false;
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
		
		percRect (data1, P_BOTTOM);
		if (! realWork (data1, globalVarNames, globalSpace))
			return false;
//		percRect (a + 1, P_BOTTOM);
		data1 ++;
		break;

		case CSTEP_AFTERCOMPILE:

		outputHalfCode (globalSpace, SLU_LOAD_GLOBAL, "init");
		outputDoneCode (globalSpace, SLU_CALLIT, 0);
		finishFunctionNew (globalSpace, NULL);
	
		setWindowInt (COM_NUM_FUNC,		countElements (functionNames) - 1);
		setWindowInt (COM_NUM_OBJ,		countElements (objectTypeNames));
		setWindowInt (COM_NUM_RES, 		countElements (allFileHandles));
		setWindowInt (COM_NUM_GLOB,		countElements (globalVarNames));
		setWindowInt (COM_NUM_STRINGS,	countElements (allSourceStrings));

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
	
		writeFinalData (projectFile, registered);
	
		// ADD ICON ------------------------------------
		if (customIcon[0]) {
			setWindowText (COM_PROGTEXT, "Adding custom icon");
			setWindowText (COM_FILENAME, customIcon);
			convertTGA (customIcon);
			setWindowText (COM_ITEMTEXT, "");
			fputc (1, projectFile);
			if (! dumpFileInto (projectFile, customIcon)) {
				fclose (projectFile);
				return errorBox (ERRORTYPE_PROJECTERROR, "Error adding custom icon (file not found or not a valid TGA file)", NULL, NULL);
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
			percRect (data1, P_BOTTOM);
			if (! runLinker (tempData, tempIndex, data1, globalVarNames, iSize, allSourceStrings))
				return false;
			allTheFunctionNamesTemp = allTheFunctionNamesTemp -> next;
//		percRect (a + 1, P_BOTTOM);		
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
			percRect (data1, P_BOTTOM);
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
		char * fromName = joinStrings (finalFile, ".sl~");
		char * toName = joinStrings (finalFile, forceSilent ? " (silent).slg" : ".slg");
		unlink (toName);
		
//		errorBox (ERRORTYPE_SYSTEMERROR, "autorun", "DUMPFILES", NULL);
		if (rename (fromName, toName))
		{
			errorBox (ERRORTYPE_SYSTEMERROR, "Couldn't rename the compiled game file... it's been left with the name", fromName, NULL);
		}
		else
		{

			// Run the compiled program (if we were told to)

			if (runMe)
			{
//				errorBox (ERRORTYPE_SYSTEMERROR, "autorun", "ON", NULL);
				runCompiledGame (compWin);
			}
			else
			{
//				errorBox (ERRORTYPE_SYSTEMERROR, "autorun", "OFF", NULL);
				EnableWindow (GetDlgItem (compWin, IDOK), true);
			}
		}
		setCompileStep (CSTEP_DONE, 0);
		break;
	}

	return true;
}

bool APIENTRY warningBoxFunc (HWND h, UINT m, WPARAM w, LPARAM l) {
	switch (m) {
		case WM_COMMAND:
		switch (LOWORD (w)) {
			case IDCANCEL:
			return 1;
			
			case ID_WARNINGLIST:
			if (HIWORD(w) == LBN_DBLCLK)
			{
				int n = SendMessage (GetDlgItem (h, ID_WARNINGLIST), LB_GETCURSEL, 0, 0);
				if (n != LB_ERR)
					userClickedErrorLine(n);
			}
			return 1;
		}
		
//		case LBN_DBLCLK:
//		
//		break;
		
		case WM_INITDIALOG:
		warningWindowH = h;
		break;
	}
	
	return 0;
}

bool APIENTRY dialogComp (HWND h, UINT m, WPARAM w, LPARAM l) {
//	static OPENFILENAME ofn;
//	static char path[MAX_PATH];

	switch (m) {
		case WM_COMMAND:
		switch (LOWORD (w)) {
			case IDCANCEL:
			DestroyWindow (h);
			return 1;
			
			case IDOK:
			runCompiledGame (h);
			return 1;
		}
		break;
/*
		case WM_USER + 1:
		
			EnableWindow (GetDlgItem (h, IDOK), true);

			// Sort out file stuff

			delete fromName;
			delete toName;
		}
		return 1;
*/
		case WM_INITDIALOG:
//		MessageBox (NULL, "Init dialog", er, MB_OK | MB_SETFOREGROUND);
		compWin = h;
		SetClassLong(h, GCL_HICON, (LONG) LoadIcon(inst, MAKEINTRESOURCE(IDI_ICON1)));
//		GetCurrentDirectory (MAX_PATH, path);
//		memset (&ofn, 0, sizeof (ofn));
//		ofn.lStructSize = sizeof (ofn);
//		ofn.hwndOwner = h;
//		ofn.hInstance = inst;
//		ofn.nMaxFile = MAX_PATH;
//		ofn.lpstrInitialDir = path;
//		ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;
		SetForegroundWindow (h);	// handle of window
		SetTimer(h,1,20,NULL);
		EnableWindow (GetDlgItem (h, IDCANCEL), true);
		return 1;

		case WM_CLOSE:
		DestroyWindow (h);
		return 1;

		case WM_PAINT:
		repaintPercBars ();
//		if (doneOnce == false) {
//			doneOnce = true;
//			PostMessage (compWin, WM_USER + 1, 526, 0);
//		}
		break;
		
		case WM_TIMER:
		if (compileStep < CSTEP_DONE)
		if (! doSingleCompileStep ())
		{
			setCompileStep (CSTEP_ERROR, 1);
			gotoSourceDirectory ();
			char * killName = joinStrings (finalFile, ".sl~");
			unlink (killName);
			delete killName;
		}
		break;

		case WM_DESTROY:
		KillTimer(h,1);
		PostQuitMessage (0);
		return 1;

//		default:
//		return (DefWindowProc(h, m, w, l));
	}
	return 0;
}

char * myNameIs;

void compileWindow (char * file, char * name, char * code) {
	MSG msg;
	theFile = file;

	myNameIs = name;

	initBuiltInFunc ();

	if (! CreateDialog (inst, MAKEINTRESOURCE(DIALOG_1), NULL, dialogComp)) {
		MessageBox (NULL, "Can't create dialogue box... ack!", "SLUDGE Compiler Error", MB_OK | MB_SETFOREGROUND);
		return;
	}

	CreateDialog(inst, MAKEINTRESOURCE(WARNINGDIALOG), NULL, warningBoxFunc);

	while (1) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
			if (!GetMessage(&msg, NULL, 0, 0))
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else
			WaitMessage();
	}
}

#endif