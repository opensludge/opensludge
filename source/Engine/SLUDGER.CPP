#include "allfiles.h"
#include "SDL.h"

#ifdef __APPLE__
#import <Cocoa/Cocoa.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <iconv.h>

#include "glee.h"
//#include <SDL_opengl.h>

#ifdef _MSC_VER
#include <shellapi.h>
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>

#include <libpng/png.h>

#include "version.h"
#include "sludger.h"
#include "backdrop.h"
#include "cursors.h"
#include "colours.h"
#include "objtypes.h"
#include "region.h"
#include "sprites.h"
#include "sprbanks.h"
#include "people.h"
#include "talk.h"
#include "newfatal.h"
#include "stringy.h"
#include "moreio.h"
#include "statusba.h"
#include "builtin.h"
#include "fonttext.h"
#include "freeze.h"
#include "floor.h"
#include "zbuffer.h"
#include "sound.h"
#include "loadsave.h"
#include "fileset.h"
#include "transition.h"
#include "language.h"
#include "specialSettings.h"
#include "Graphics.h"

#include "debug.h"
#ifdef _WIN32
#include <shellapi.h>
#include <shlobj.h> // For SHGetFolderPath
#endif

int showSetupWindow();

extern personaAnimation * mouseCursorAnim;
extern spritePalette pastePalette;
extern int dialogValue, sceneWidth, sceneHeight;
extern char * launchMe;
extern variable * launchResult;

int numBIFNames = 0;
char * * allBIFNames = NULL;
int numUserFunc = 0;
char * * allUserFunc = NULL;
int numResourceNames = 0;
char * * allResourceNames = NULL;
int selectedLanguage = 0;
int languageNum = -1;

int gameVersion;
int specialSettings;
FILETIME fileTime;
//extern byte frameRate;
extern int desiredfps;
bool captureAllKeys = false;

unsigned char brightnessLevel = 255;

eventHandlers mainHandlers;
eventHandlers * currentEvents = & mainHandlers;

extern HWND hMainWindow;
extern screenRegion * overRegion;
extern speechStruct * speech;
extern statusStuff * nowStatus;
extern loadedFunction * saverFunc;

loadedFunction * allRunningFunctions = NULL;
screenRegion * lastRegion = NULL;
variableStack * noStack = NULL;
char * loadNow = NULL;
inputType input;
variable * globalVars;
int numGlobals;

char * sludgeText[] = { "?????", "RETURN", "BRANCH", "BR_ZERO", "SET_GLOBAL",
						"SET_LOCAL", "LOAD_GLOBAL", "LOAD_LOCAL",
						"PLUS", "MINUS", "MULT", "DIVIDE",
						"AND", "OR", "EQUALS", "NOT_EQ", "MODULUS", "LOAD_VALUE",
						"LOAD_BUILT", "LOAD_FUNC", "CALLIT", "LOAD_STRING", "LOAD_FILE",
						"LOAD_OBJTYPE", "NOT", "LOAD_NULL", "STACK_PUSH",
						"LESSTHAN", "MORETHAN", "NEGATIVE", "U", "LESS_EQUAL", "MORE_EQUAL",
						"INC_LOCAL", "DEC_LOCAL", "INC_GLOBAL", "DEC_GLOBAL", "INDEXSET", "INDEXGET",
						"INC_INDEX", "DEC_INDEX", "QUICK_PUSH"};

void loadHandlers (FILE * fp) {
	currentEvents -> leftMouseFunction		= get2bytes (fp);
	currentEvents -> leftMouseUpFunction	= get2bytes (fp);
	currentEvents -> rightMouseFunction		= get2bytes (fp);
	currentEvents -> rightMouseUpFunction	= get2bytes (fp);
	currentEvents -> moveMouseFunction		= get2bytes (fp);
	currentEvents -> focusFunction			= get2bytes (fp);
	currentEvents -> spaceFunction			= get2bytes (fp);
}

void saveHandlers (FILE * fp) {
	put2bytes (currentEvents -> leftMouseFunction,		fp);
	put2bytes (currentEvents -> leftMouseUpFunction,	fp);
	put2bytes (currentEvents -> rightMouseFunction,		fp);
	put2bytes (currentEvents -> rightMouseUpFunction,	fp);
	put2bytes (currentEvents -> moveMouseFunction,		fp);
	put2bytes (currentEvents -> focusFunction,			fp);
	put2bytes (currentEvents -> spaceFunction,			fp);
}

FILE * openAndVerify (char * filename, char extra1, char extra2, char * er, int & fileVersion) {
	FILE * fp = fopen (filename, "rb");
	if (! fp) {
		fatal ("Can't open file", filename);
		return false;
	}
	bool headerBad = false;
	if (fgetc (fp) != 'S') headerBad = true;
	if (fgetc (fp) != 'L') headerBad = true;
	if (fgetc (fp) != 'U') headerBad = true;
	if (fgetc (fp) != 'D') headerBad = true;
	if (fgetc (fp) != extra1) headerBad = true;
	if (fgetc (fp) != extra2) headerBad = true;
	if (headerBad) {
		fatal (er, filename);
		return NULL;
	}
	fgetc (fp);
	while (fgetc(fp)) {;}

	int majVersion = fgetc (fp);
	int minVersion = fgetc (fp);
	fileVersion = majVersion * 256 + minVersion;

	char txtVer[120];

	if (fileVersion > WHOLE_VERSION) {
		sprintf (txtVer, ERROR_VERSION_TOO_LOW_2, majVersion, minVersion);
		fatal (ERROR_VERSION_TOO_LOW_1, txtVer);
		return NULL;
	} else if (fileVersion < MINIM_VERSION) {
		sprintf (txtVer, ERROR_VERSION_TOO_HIGH_2, majVersion, minVersion);
		fatal (ERROR_VERSION_TOO_HIGH_1, txtVer);
		return NULL;
	}
	return fp;
}

extern settingsStruct gameSettings;

bool initSludge (char * filename) {
	int a = 0;
	mouseCursorAnim = makeNullAnim ();
	createFontPalette (pastePalette);

	FILE * fp = openAndVerify (filename, 'G', 'E', ERROR_BAD_HEADER, gameVersion);
	if (! fp) return false;

	if (fgetc (fp)) {
		numBIFNames = get2bytes (fp);
		allBIFNames = new char * [numBIFNames];
		for (int fn = 0; fn < numBIFNames; fn ++) {
			allBIFNames[fn] = readString (fp);
		}
		numUserFunc = get2bytes (fp);
		allUserFunc = new char * [numUserFunc];
		for (int fn = 0; fn < numUserFunc; fn ++) {
			allUserFunc[fn] = readString (fp);
		}
		if (gameVersion >= VERSION(1,3)) {
			numResourceNames = get2bytes (fp);
			allResourceNames = new char * [numResourceNames];
			for (int fn = 0; fn < numResourceNames; fn ++) {
				allResourceNames[fn] = readString (fp);
			}
		}
	}
	winWidth = get2bytes (fp);
	winHeight = get2bytes (fp);
	specialSettings = fgetc (fp);

	desiredfps = 1000/fgetc (fp);

	delete readString (fp); // Unused - was used for registration purposes.
	fread (& fileTime, sizeof (FILETIME), 1, fp);


	char * dataFol = (gameVersion >= VERSION(1,3)) ? readString(fp) : joinStrings ("", "");

	gameSettings.numLanguages = (gameVersion >= VERSION(1,3)) ? (fgetc (fp)) : 0;
	makeLanguageTable (fp);

	bool useAAFromIni = true;
	if (gameVersion >= VERSION(1,6))
	{
		useAAFromIni = fgetc(fp);
		aaLoad(maxAntiAliasSettings, fp);
	}

	char * checker = readString (fp);

	if (strcmp (checker, "okSoFar")) return fatal (ERROR_BAD_HEADER, filename);
	delete checker;
	checker = NULL;


	if (fgetc (fp)) {
		// There is an icon - read it!
		int n;
		int w, h;

		long file_pointer = ftell (fp);

		png_structp png_ptr;
		png_infop info_ptr, end_info;


		int fileIsPNG = true;

		// Is this a PNG file?

		char tmp[10];
		fread(tmp, 1, 8, fp);
		if (png_sig_cmp((png_byte *) tmp, 0, 8)) {
			// No, it's old-school HSI
			fileIsPNG = false;
			fseek(fp, file_pointer, SEEK_SET);

			w = get2bytes (fp);
			h = get2bytes (fp);
		} else {
			// Read the PNG header

			png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
			if (!png_ptr) {
				return false;
			}

			info_ptr = png_create_info_struct(png_ptr);
			if (!info_ptr) {
				png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
				return false;
			}

			end_info = png_create_info_struct(png_ptr);
			if (!end_info) {
				png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
				return false;
			}
			png_init_io(png_ptr, fp);		// Tell libpng which file to read
			png_set_sig_bytes(png_ptr, 8);	// 8 bytes already read

			png_read_info(png_ptr, info_ptr);

			png_uint_32 width, height;
			int bit_depth, color_type, interlace_type, compression_type, filter_method;
			png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

			w = width;
			h = height;

			if (bit_depth < 8) png_set_packing(png_ptr);
			png_set_expand(png_ptr);
			if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png_ptr);
			if (bit_depth == 16) png_set_strip_16(png_ptr);

			png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);

			png_read_update_info(png_ptr, info_ptr);
			png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

		}


		if (SDL_Surface * programIcon = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000)) {
			signed long transCol = 63519;
			SDL_LockSurface(programIcon);
			Uint8 *p = (Uint8 *) programIcon->pixels;

			if (fileIsPNG) {
				unsigned char * row_pointers[h];
				for (int i = 0; i<h; i++)
					row_pointers[i] = p + 4*i*w;

				png_read_image(png_ptr, (png_byte **) row_pointers);
				png_read_end(png_ptr, NULL);
				png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
			} else {

				for (int t2 = 0; t2 < h; t2 ++) {
					int t1 = 0;
					while (t1 < w) {
						unsigned short c = (unsigned short) get2bytes (fp);
						if (c & 32) {
							n = fgetc (fp) + 1;
							c -= 32;
						} else {
							n = 1;
						}
						while (n --) {
							*p++ = (Uint8) redValue(c);
							*p++ = (Uint8) greenValue(c);
							*p++ = (Uint8) blueValue(c);
							*p++ = (Uint8) (c == transCol) ? 0 : 255;

							t1++;
						}
					}
				}
			}
			SDL_UnlockSurface (programIcon);
			SDL_WM_SetIcon(programIcon, NULL);
			SDL_FreeSurface(programIcon);
		}
	}

 	numGlobals = get2bytes (fp);

	globalVars = new variable[numGlobals];
	if (! checkNew (globalVars)) return false;
	for (a = 0; a < numGlobals; a ++) initVarNew (globalVars[a]);

	// Get the original (untranslated) name of the game and convert it to Unicode.
	// We use this to find saved preferences and saved games.
	setFileIndices (fp, gameSettings.numLanguages, 0);
	char * gameNameWin = getNumberedString(1);
	char * gameName = new char[ 1024];
	char **tmp1 = (char **) &gameNameWin;
	char **tmp2 = &gameName;
	char * nameOrig = gameNameWin;
	char * gameNameOrig = gameName;

	iconv_t convert = iconv_open ("UTF-8", "CP1252");
	size_t len1 = strlen(gameNameWin)+1;
	size_t len2 = 1023;
	iconv (convert, (const char **) tmp1, &len1, tmp2, &len2);
	iconv_close (convert);

	gameName = encodeFilename (gameNameOrig);

	delete nameOrig;
	delete gameNameOrig;

#ifdef __APPLE__
	char appsupport_path[1024];
	FSRef foundRef;

	// Find the Application Support Folder  (or should it be kPreferencesFolderType?) and go there
	FSFindFolder (kUserDomain, kApplicationSupportFolderType , kDontCreateFolder, &foundRef);
	FSRefMakePath( &foundRef, (Uint8 *) appsupport_path, 1024);
	chdir (appsupport_path);
#endif
#ifdef _WIN32
	TCHAR szAppData[MAX_PATH];
	/*hr = */SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szAppData);
	_chdir(szAppData);
#endif

#ifdef _WIN32
	mkdir (gameName);
#else
	mkdir (gameName, 0000777);
#endif
#ifdef _MSC_VER
	if (_chdir (gameName)) return fatal ("This game's preference folder is inaccessible!\nI can't access the following directory (maybe there's a file with the same name, or maybe it's read-protected):", gameName);
#else
	if (chdir (gameName)) return fatal ("This game's preference folder is inaccessible!\nI can't access the following directory (maybe there's a file with the same name, or maybe it's read-protected):", gameName);
#endif

	// Get user settings
	readIniFile (filename);
	if (! showSetupWindow()) return 0;
	saveIniFile (filename);

	if (useAAFromIni && gameSettings.antiAlias >= 0)
		maxAntiAliasSettings.useMe = (gameSettings.antiAlias != 0);

	// Now set file indices properly to the chosen language.
	languageNum = getLanguageForFileB ();
	if (languageNum < 0) return fatal ("Can't find the translation data specified!");
	setFileIndices (NULL, gameSettings.numLanguages, languageNum);

	if (dataFol[0]) {
		char *dataFolder = encodeFilename(dataFol);
#ifdef _WIN32
		mkdir (dataFolder);
#else
		mkdir (dataFolder, 0000777);
#endif
#ifdef _MSC_VER
		if (_chdir (dataFolder)) return fatal ("This game's data folder is inaccessible!\nI can't access the following directory (maybe there's a file with the same name, or maybe it's read-protected):", dataFolder);
#else
		if (chdir (dataFolder)) return fatal ("This game's data folder is inaccessible!\nI can't access the following directory (maybe there's a file with the same name, or maybe it's read-protected):", dataFolder);
#endif
		delete dataFolder;
	}

 	positionStatus (10, winHeight - 15);

	return true;
}

extern int cameraX, cameraY;

bool checkColourChange (bool reset) {
	static GLuint oldPixel;
	static GLuint pixel;

	glReadPixels(viewportOffsetX+input.mouseX*viewportWidth/winWidth, viewportOffsetY+(winHeight - input.mouseY)*viewportHeight/winHeight, 1, 1, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, &pixel);

	if (reset || oldPixel != pixel) {
		oldPixel = pixel;
		return true;
	}
	return false;
}

void sludgeDisplay () {

	glDepthMask (GL_TRUE);
	//glClearColor(0.5, 0.5, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen
	glDepthMask (GL_FALSE);
/*
	glDisable (GL_TEXTURE_2D);

	glColor4i(100, 100, 255, 255);
	glBegin(GL_QUADS);
	glVertex3f(0.0, 0.0, 0);
	glVertex3f(800.0, 0.0, 0);
	glVertex3f(800.0, 800.0, 0);
	glVertex3f(0.0, 800.0, 0);
	glEnd();*/


	drawBackDrop ();				// Draw the room

	drawZBuffer(cameraX, cameraY, false);

	checkColourChange (true);

	glEnable(GL_DEPTH_TEST);
	drawPeople ();					// Then add any moving characters...
	glDisable(GL_DEPTH_TEST);
	viewSpeech ();					// ...and anything being said
	drawStatusBar ();
	displayCursor ();

	if (brightnessLevel < 255) fixBrightness ();	// This is for transitionLevel special effects

	glFlush();
	SDL_GL_SwapBuffers();

}

void pauseFunction (loadedFunction * fun) {
	loadedFunction * * huntAndDestroy = & allRunningFunctions;
	while (* huntAndDestroy) {
		if (fun == * huntAndDestroy) {
			(* huntAndDestroy) = (* huntAndDestroy) -> next;
		} else {
			huntAndDestroy = & (* huntAndDestroy) -> next;
		}
	}
}

void restartFunction (loadedFunction * fun) {
	fun -> next = allRunningFunctions;
	allRunningFunctions = fun;
}

void killSpeechTimers () {
	loadedFunction * thisFunction = allRunningFunctions;

	while (thisFunction) {
		if (thisFunction -> freezerLevel == 0 && thisFunction -> isSpeech && thisFunction -> timeLeft) {
			thisFunction -> timeLeft = 0;
			thisFunction -> isSpeech = false;
		}
		thisFunction = thisFunction -> next;
	}

	killAllSpeech ();
}

void completeTimers () {
	loadedFunction * thisFunction = allRunningFunctions;

	while (thisFunction) {
		if (thisFunction -> freezerLevel == 0) thisFunction -> timeLeft = 0;
		thisFunction = thisFunction -> next;
	}
}

void finishFunction (loadedFunction * fun) {
	int a;

	pauseFunction (fun);
	if (fun -> stack) fatal (ERROR_NON_EMPTY_STACK);
	delete fun -> compiledLines;
	for (a = 0; a < fun -> numLocals; a ++) unlinkVar (fun -> localVars[a]);
	delete fun -> localVars;
	unlinkVar (fun -> reg);
	delete fun;
	fun = NULL;
}

void abortFunction (loadedFunction * fun) {
	int a;

	pauseFunction (fun);
	while (fun -> stack) trimStack (fun -> stack);
	delete fun -> compiledLines;
	for (a = 0; a < fun -> numLocals; a ++) unlinkVar (fun -> localVars[a]);
	delete fun -> localVars;
	unlinkVar (fun -> reg);
	if (fun -> calledBy) abortFunction (fun -> calledBy);
	delete fun;
	fun = NULL;
}

int cancelAFunction (int funcNum, loadedFunction * myself, bool & killedMyself) {
	int n = 0;
	killedMyself = false;

	loadedFunction * fun = allRunningFunctions;
	while (fun) {
		if (fun -> originalNumber == funcNum) {
			fun -> cancelMe = true;
			n ++;
			if (fun == myself) killedMyself = true;
		}
		fun = fun -> next;
	}
	return n;
}

void freezeSubs () {
	loadedFunction * thisFunction = allRunningFunctions;

	while (thisFunction) {
		if (thisFunction -> unfreezable) {
//			MessageBox (NULL, "Trying to freeze an unfreezable function!", "SLUDGE debugging bollocks!", MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL | MB_SETFOREGROUND);
		} else {
			thisFunction -> freezerLevel ++;
		}
		thisFunction = thisFunction -> next;
	}
}

void unfreezeSubs () {
	loadedFunction * thisFunction = allRunningFunctions;

	while (thisFunction) {
		if (thisFunction -> freezerLevel) thisFunction -> freezerLevel --;
		thisFunction = thisFunction -> next;
	}
}

//FILE * debuggy2 = fopen ("ou.txt", "wt");
//extern persona * hackPersona;

bool continueFunction (loadedFunction * fun) {
	bool keepLooping = true;
	bool advanceNow;
	unsigned int param;
	sludgeCommand com;

	if (fun -> cancelMe) {
		abortFunction (fun);
		return true;
	}

//	if (numBIFNames) newDebug ("*** Function:", allUserFunc[fun -> originalNumber]);

	//debugOut ("SLUDGER: continueFunction\n");

	while (keepLooping) {
		advanceNow = true;
		param = fun -> compiledLines[fun -> runThisLine].param;
		com = fun -> compiledLines[fun -> runThisLine].theCommand;
//		fprintf (stderr, "com: %d param: %d (%s)\n", com, param,
//				(com < numSludgeCommands) ? sludgeText[com] : ERROR_UNKNOWN_MCODE); fflush(stderr);

		if (numBIFNames) {
			setFatalInfo (
				(fun -> originalNumber < numUserFunc) ? allUserFunc[fun -> originalNumber] : "Unknown user function",
				(com < numSludgeCommands) ? sludgeText[com] : ERROR_UNKNOWN_MCODE);
//			newDebug (
//				(com < numSludgeCommands) ? sludgeText[com] : "Unknown SLUDGE machine code",
//				param);
		}

		//debugOut ("SLUDGER: continueFunction - in da loop: %s\n", sludgeText[com]);

		switch (com) {
			case SLU_RETURN:
			if (fun -> calledBy) {
				loadedFunction * returnTo = fun -> calledBy;
				if (fun -> returnSomething) copyVariable (fun -> reg, returnTo -> reg);
				finishFunction (fun);
				fun = returnTo;
				restartFunction (fun);
			} else {
				finishFunction (fun);
				advanceNow = false;		// So we don't do anything else with "fun"
				keepLooping = false;	// So we drop out of the loop
			}
			break;

			case SLU_CALLIT:
			switch (fun -> reg.varType) {
				case SVT_FUNC:
				pauseFunction (fun);
				if (numBIFNames) setFatalInfo (
					(fun -> originalNumber < numUserFunc) ? allUserFunc[fun -> originalNumber] : "Unknown user function",
					(fun -> reg.varData.intValue < numUserFunc) ? allUserFunc[fun -> reg.varData.intValue] : "Unknown user function");

				if (! startNewFunctionNum (fun -> reg.varData.intValue, param, fun, fun -> stack)) return false;
				fun = allRunningFunctions;
				advanceNow = false;		// So we don't do anything else with "fun"
				break;

				case SVT_BUILT:
					{
					builtReturn br = callBuiltIn (fun -> reg.varData.intValue, param, fun);
				    //fprintf (stderr, "Function returned. \n");    fflush (stderr);

				//debugOut ("built - %i\n", br);

				switch (br) {
					case BR_NOCOMMENT:
					return false;

					case BR_PAUSE:
					pauseFunction (fun);
					// No break!

					case BR_KEEP_AND_PAUSE:
					keepLooping = false;
					break;

					case BR_ALREADY_GONE:
					keepLooping = false;
					advanceNow = false;
					break;

					case BR_CALLAFUNC:
					{
						int i = fun -> reg.varData.intValue;
						setVariable (fun -> reg, SVT_INT, 1);
						pauseFunction (fun);
						if (numBIFNames) setFatalInfo (
							(fun -> originalNumber < numUserFunc) ? allUserFunc[fun -> originalNumber] : "Unknown user function",
							(i < numUserFunc) ? allUserFunc[i] : "Unknown user function");
						if (! startNewFunctionNum (i, 0, fun, noStack, false)) return false;
						fun = allRunningFunctions;
						advanceNow = false;		// So we don't do anything else with "fun"
					}
					break;

				}
				}
				break;

				default:
				return fatal (ERROR_CALL_NONFUNCTION);
			}
			break;

			// These all grab things and shove 'em into the register

			case SLU_LOAD_NULL:
			setVariable (fun -> reg, SVT_NULL, 0);
			break;

			case SLU_LOAD_FILE:
			setVariable (fun -> reg, SVT_FILE, param);
			break;

			case SLU_LOAD_VALUE:
			setVariable (fun -> reg, SVT_INT, param);
			break;

			case SLU_LOAD_LOCAL:
			if (! copyVariable (fun -> localVars[param], fun -> reg)) return false;
			break;

			case SLU_AND:
			setVariable (fun -> reg, SVT_INT, getBoolean (fun -> reg) && getBoolean (fun -> stack -> thisVar));
			trimStack (fun -> stack);
			break;

			case SLU_OR:
			setVariable (fun -> reg, SVT_INT, getBoolean (fun -> reg) || getBoolean (fun -> stack -> thisVar));
			trimStack (fun -> stack);
			break;

			case SLU_LOAD_FUNC:
			setVariable (fun -> reg, SVT_FUNC, param);
			break;

			case SLU_LOAD_BUILT:
			setVariable (fun -> reg, SVT_BUILT, param);
			break;

			case SLU_LOAD_OBJTYPE:
			setVariable (fun -> reg, SVT_OBJTYPE, param);
			break;

			case SLU_UNREG:
			if (dialogValue != 1) fatal (ERROR_HACKER);
			break;

			case SLU_LOAD_STRING:
				if (! loadStringToVar (fun -> reg, param)) {
					return false;
				}
			break;

			case SLU_INDEXGET:
			case SLU_INCREMENT_INDEX:
			case SLU_DECREMENT_INDEX:
			switch (fun -> stack -> thisVar.varType) {
				case SVT_NULL:
				if (com == SLU_INDEXGET) {
					setVariable (fun -> reg, SVT_NULL, 0);
					trimStack (fun -> stack);
				} else {
					return fatal (ERROR_INCDEC_UNKNOWN);
				}
				break;

				case SVT_FASTARRAY:
				case SVT_STACK:
				if (fun -> stack -> thisVar.varData.theStack -> first == NULL) {
					return fatal (ERROR_INDEX_EMPTY);
				} else {
					int ii;
					if (! getValueType (ii, SVT_INT, fun -> reg)) return false;
					variable * grab = (fun -> stack -> thisVar.varType == SVT_FASTARRAY) ?
						fastArrayGetByIndex (fun -> stack -> thisVar.varData.fastArray, ii)
							:
						stackGetByIndex (fun -> stack -> thisVar.varData.theStack -> first, ii);

					trimStack (fun -> stack);

					if (! grab) {
						setVariable (fun -> reg, SVT_NULL, 0);
					} else {
						int ii;
						switch (com) {
							case SLU_INCREMENT_INDEX:
							if (! getValueType (ii, SVT_INT, * grab)) return false;
							setVariable (fun -> reg, SVT_INT, ii);
							grab -> varData.intValue = ii + 1;
							break;

							case SLU_DECREMENT_INDEX:
							if (! getValueType (ii, SVT_INT, * grab)) return false;
							setVariable (fun -> reg, SVT_INT, ii);
							grab -> varData.intValue = ii - 1;
							break;

							default:
							if (! copyVariable (* grab, fun -> reg)) return false;
						}
					}
				}
				break;

				default:
				return fatal (ERROR_INDEX_NONSTACK);
			}
			break;

			case SLU_INDEXSET:
			switch (fun -> stack -> thisVar.varType) {
				case SVT_STACK:
				if (fun -> stack -> thisVar.varData.theStack -> first == NULL) {
					return fatal (ERROR_INDEX_EMPTY);
				} else {
					int ii;
					if (! getValueType (ii, SVT_INT, fun -> reg)) return false;
					if (! stackSetByIndex (fun -> stack -> thisVar.varData.theStack -> first, ii, fun -> stack -> next -> thisVar)) {
						return false;
					}
					trimStack (fun -> stack);
					trimStack (fun -> stack);
				}
				break;

				case SVT_FASTARRAY:
				{
					int ii;
					if (! getValueType (ii, SVT_INT, fun -> reg)) return false;
					variable * v = fastArrayGetByIndex (fun -> stack -> thisVar.varData.fastArray, ii);
					if (v == NULL) return fatal ("Not within bounds of fast array.");
					if (! copyVariable (fun -> stack -> next -> thisVar, * v)) return false;
					trimStack (fun -> stack);
					trimStack (fun -> stack);
				}
				break;

				default:
				return fatal (ERROR_INDEX_NONSTACK);
			}
			break;

			// What can we do with the register? Well, we can copy it into a local
			// variable, a global or onto the stack...

			case SLU_INCREMENT_LOCAL:
			{
				int ii;
				if (! getValueType (ii, SVT_INT, fun -> localVars[param])) return false;
				setVariable (fun -> reg, SVT_INT, ii);
				setVariable (fun -> localVars[param], SVT_INT, ii + 1);
			}
			break;

			case SLU_INCREMENT_GLOBAL:
			{
				int ii;
				if (! getValueType (ii, SVT_INT, globalVars[param])) return false;
				setVariable (fun -> reg, SVT_INT, ii);
				setVariable (globalVars[param], SVT_INT, ii + 1);
			}
			break;

			case SLU_DECREMENT_LOCAL:
			{
				int ii;
				if (! getValueType (ii, SVT_INT, fun -> localVars[param])) return false;
				setVariable (fun -> reg, SVT_INT, ii);
				setVariable (fun -> localVars[param], SVT_INT, ii - 1);
			}
			break;

			case SLU_DECREMENT_GLOBAL:
			{
				int ii;
				if (! getValueType (ii, SVT_INT, globalVars[param])) return false;
				setVariable (fun -> reg, SVT_INT, ii);
				setVariable (globalVars[param], SVT_INT, ii - 1);
			}
			break;

			case SLU_SET_LOCAL:
			if (! copyVariable (fun -> reg, fun -> localVars[param])) return false;
			break;

			case SLU_SET_GLOBAL:
//			newDebug ("  Copying TO global variable", param);
//			newDebug ("  Global type at the moment", globalVars[param].varType);
			if (! copyVariable (fun -> reg, globalVars[param])) return false;
//			newDebug ("  New type", globalVars[param].varType);
			break;

			case SLU_LOAD_GLOBAL:
//			newDebug ("  Copying FROM global variable", param);
//			newDebug ("  Global type at the moment", globalVars[param].varType);
			if (! copyVariable (globalVars[param], fun -> reg)) return false;
			break;

			case SLU_STACK_PUSH:
			if (! addVarToStack (fun -> reg, fun -> stack)) return false;
			break;

			case SLU_QUICK_PUSH:
			if (! addVarToStackQuick (fun -> reg, fun -> stack)) return false;
			break;

			case SLU_NOT:
			setVariable (fun -> reg, SVT_INT, ! getBoolean (fun -> reg));
			break;

			case SLU_BR_ZERO:
			if (! getBoolean (fun -> reg)) {
				advanceNow = false;
				fun -> runThisLine = param;
			}
			break;

			case SLU_BRANCH:
			advanceNow = false;
			fun -> runThisLine = param;
			break;

			case SLU_NEGATIVE:
			{
				int i;
				if (! getValueType (i, SVT_INT, fun -> reg)) return false;
				setVariable (fun -> reg, SVT_INT, -i);
			}
			break;

			// All these things rely on there being somet' on the stack

			case SLU_MULT:
			case SLU_PLUS:
			case SLU_MINUS:
			case SLU_MODULUS:
			case SLU_DIVIDE:
			case SLU_EQUALS:
			case SLU_NOT_EQ:
			case SLU_LESSTHAN:
			case SLU_MORETHAN:
			case SLU_LESS_EQUAL:
			case SLU_MORE_EQUAL:
			if (fun -> stack) {
				int firstValue, secondValue;

				switch (com) {
					case SLU_PLUS:
					addVariablesInSecond (fun -> stack -> thisVar, fun -> reg);
					trimStack (fun -> stack);
					break;

					case SLU_EQUALS:
					compareVariablesInSecond (fun -> stack -> thisVar, fun -> reg);
					trimStack (fun -> stack);
					break;

					case SLU_NOT_EQ:
					compareVariablesInSecond (fun -> stack -> thisVar, fun -> reg);
					trimStack (fun -> stack);
	               fun -> reg.varData.intValue = ! fun -> reg.varData.intValue;
					break;

					default:
					if (! getValueType (firstValue, SVT_INT, fun -> stack -> thisVar)) return false;
					if (! getValueType (secondValue, SVT_INT, fun -> reg)) return false;
					trimStack (fun -> stack);

					switch (com) {
						case SLU_MULT:
						setVariable (fun -> reg, SVT_INT, firstValue * secondValue);
						break;

						case SLU_MINUS:
						setVariable (fun -> reg, SVT_INT, firstValue - secondValue);
						break;

						case SLU_MODULUS:
						setVariable (fun -> reg, SVT_INT, firstValue % secondValue);
						break;

						case SLU_DIVIDE:
						setVariable (fun -> reg, SVT_INT, firstValue / secondValue);
						break;

						case SLU_LESSTHAN:
						setVariable (fun -> reg, SVT_INT, firstValue < secondValue);
						break;

						case SLU_MORETHAN:
						setVariable (fun -> reg, SVT_INT, firstValue > secondValue);
						break;

						case SLU_LESS_EQUAL:
						setVariable (fun -> reg, SVT_INT, firstValue <= secondValue);
						break;

						case SLU_MORE_EQUAL:
						setVariable (fun -> reg, SVT_INT, firstValue >= secondValue);
						break;
					}
				}
			} else {
				return fatal (ERROR_NOSTACK);
			}
			break;

			default:
			return fatal (ERROR_UNKNOWN_CODE);
		}

		//debugOut ("AdvanceNow is %i\n", advanceNow);
		if (advanceNow) fun -> runThisLine ++;
//		fprintf (stderr, ".\n"); fflush (stderr);

	}
//	fprintf (debuggy, "I think I've dropped out of this thread successfully!\n");
	return true;
}


bool runSludge () {
	loadedFunction * thisFunction = allRunningFunctions;
	loadedFunction * nextFunction;

#if BUILTINDEBUG
	builtInDebugTick();
#endif

//	debugSounds ();
	while (thisFunction) {
		nextFunction = thisFunction -> next;
		//debugOut ("SLUDGER: runSludge in the loop\n");

		if (! thisFunction -> freezerLevel) {
			if (thisFunction -> timeLeft) {
//				if (numBIFNames) newDebug ("*** Paused function:", allUserFunc[thisFunction -> originalNumber]);
//				newDebug ("Time left...", thisFunction -> timeLeft);
				if (thisFunction -> timeLeft < 0) {
					if (! stillPlayingSound (findInSoundCache (speech -> lastFile))) {
						thisFunction -> timeLeft = 0;
					}
				} else if (! -- (thisFunction -> timeLeft)) {
				}
			} else {
				if (thisFunction -> isSpeech) {
					thisFunction -> isSpeech = false;
					killAllSpeech ();
				}
				//debugOut ("SLUDGER: runSludge before\n");
				if (! continueFunction (thisFunction)) return false;
				//debugOut ("SLUDGER: runSludge after\n");
			}
		}

		thisFunction = nextFunction;
	}

	if (loadNow) {
		if (loadNow[0] == ':') {
			saveGame (loadNow + 1);
			setVariable (saverFunc -> reg, SVT_INT, 1);
		} else {
			if (! loadGame (loadNow)) return false;
		}
		delete loadNow;
		loadNow = NULL;
	}

	return true;
}

bool loadFunctionCode (loadedFunction * newFunc) {
	unsigned int numLines, numLinesRead;
	int a;

	if (! openSubSlice (newFunc -> originalNumber)) return false;


	newFunc -> unfreezable	= fgetc (bigDataFile);
	numLines				= get2bytes (bigDataFile);
	newFunc -> numArgs		= get2bytes (bigDataFile);
	newFunc -> numLocals	= get2bytes (bigDataFile);
	newFunc -> compiledLines = new lineOfCode[numLines];
	if (! checkNew (newFunc -> compiledLines)) return false;

	for (numLinesRead = 0; numLinesRead < numLines; numLinesRead ++) {
		newFunc -> compiledLines[numLinesRead].theCommand = (sludgeCommand) fgetc (bigDataFile);
		newFunc -> compiledLines[numLinesRead].param = get2bytes (bigDataFile);
	}

	finishAccess ();

	// Now we need to reserve memory for the local variables

	newFunc -> localVars = new variable[newFunc -> numLocals];
	if (! checkNew (newFunc -> localVars)) return false;
	for (a = 0; a < newFunc -> numLocals; a ++) {
		initVarNew (newFunc -> localVars[a]);
	}
	return true;
}

int startNewFunctionNum (unsigned int funcNum, unsigned int numParamsExpected, loadedFunction * calledBy, variableStack * & vStack, bool returnSommet) {
	loadedFunction * newFunc = new loadedFunction;
	checkNew (newFunc);
	newFunc -> originalNumber = funcNum;

	loadFunctionCode (newFunc);

	if (newFunc -> numArgs != numParamsExpected) return fatal ("Wrong number of parameters!");
	if (newFunc -> numArgs > newFunc -> numLocals) return fatal ("More arguments than local variable space!");

	// Now, lets copy the parameters from the calling function's stack...

	while (numParamsExpected) {
		numParamsExpected --;
		if (vStack == NULL) return fatal ("Corrupted file! The stack's empty and there were still parameters expected");
		copyVariable (vStack -> thisVar, newFunc -> localVars[numParamsExpected]);
		trimStack (vStack);
	}

	newFunc -> cancelMe = false;
	newFunc -> timeLeft = 0;
	newFunc -> returnSomething = returnSommet;
	newFunc -> calledBy = calledBy;
	newFunc -> stack = NULL;
	newFunc -> freezerLevel = 0;
	newFunc -> runThisLine = 0;
	newFunc -> isSpeech = 0;
	initVarNew (newFunc -> reg);

	restartFunction (newFunc);
	return 1;
}

int lastFramesPerSecond = -1;
int thisFramesPerSecond = -1;
int lastSeconds = 0;

bool handleInput () {
	static int l = 0;
	static Uint32 theTime;

	theTime = SDL_GetTicks() / 1000;
	if (lastSeconds != theTime) {
		lastSeconds = theTime;
		lastFramesPerSecond = thisFramesPerSecond;
		thisFramesPerSecond = 1;
	} else {
		thisFramesPerSecond ++;
	}
//	lastFramesPerSecond = theTime.wSecond;

	if (launchMe) {
		if (l) {
			// Still paused because of spawned thingy...
		} else {
			l = 1;
			unsigned long retVal = 0;
#ifdef _WIN32
			retVal = (unsigned long) ShellExecute (hMainWindow, "open",
							  									 launchMe, NULL, "C:\\",
							  									 SW_SHOWNORMAL);
#else
			if (launchMe[0] == 'h' &&
				launchMe[1] == 't' &&
				launchMe[2] == 't' &&
				launchMe[3] == 'p' &&
				launchMe[4] == ':') {

				// IT'S A WEBSITE!
				[[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString: [NSString stringWithUTF8String: launchMe]]];

			} else {
				[[NSWorkspace sharedWorkspace] openFile: [NSString stringWithUTF8String: launchMe]];
			}
#endif
			setVariable (* launchResult, SVT_INT, retVal > 31);
			launchMe = NULL;
			launchResult = NULL;
		}
		return true;
	} else {
		 l = 0;
	}

	if (! overRegion) getOverRegion ();
//	input.checkRegion = false;

	if (input.justMoved) {
		if (currentEvents -> moveMouseFunction) {
			if (! startNewFunctionNum (currentEvents -> moveMouseFunction, 0, NULL, noStack)) return false;
		}
	}
	input.justMoved = false;
//	if (! runSludge ()) return false;

	if (lastRegion != overRegion && currentEvents -> focusFunction) {
		variableStack * tempStack = new variableStack;

		if (! checkNew (tempStack)) return false;
		initVarNew (tempStack -> thisVar);
		if (overRegion) {
			setVariable (tempStack -> thisVar, SVT_OBJTYPE, overRegion -> thisType -> objectNum);
		} else {
			setVariable (tempStack -> thisVar, SVT_INT, 0);
		}
		tempStack -> next = NULL;
		if (! startNewFunctionNum (currentEvents -> focusFunction, 1, NULL, tempStack)) return false;
	}
	if (input.leftRelease && currentEvents -> leftMouseUpFunction)  {
		if (! startNewFunctionNum (currentEvents -> leftMouseUpFunction, 0, NULL, noStack)) return false;
	}
	if (input.rightRelease && currentEvents -> rightMouseUpFunction) {
		if (! startNewFunctionNum (currentEvents -> rightMouseUpFunction, 0, NULL, noStack)) return false;
	}
	if (input.leftClick && currentEvents -> leftMouseFunction)
		if (! startNewFunctionNum (currentEvents -> leftMouseFunction, 0, NULL, noStack)) return false;
	if (input.rightClick && currentEvents -> rightMouseFunction) {
		if (! startNewFunctionNum (currentEvents -> rightMouseFunction, 0, NULL, noStack)) return false;
	}
	if (input.keyPressed && currentEvents -> spaceFunction) {
		char * tempString = NULL;
		switch (input.keyPressed) {
			case 127:	tempString = copyString ("BACKSPACE");	break;
			case 9:		tempString = copyString ("TAB");		break;
			case 13:	tempString = copyString ("ENTER");		break;
			case 27:	tempString = copyString ("ESCAPE");		break;
			/*
			case 1112:	tempString = copyString ("ALT+F1");		break;
			case 1113:	tempString = copyString ("ALT+F2");		break;
			case 1114:	tempString = copyString ("ALT+F3");		break;
			case 1115:	tempString = copyString ("ALT+F4");		break;
			case 1116:	tempString = copyString ("ALT+F5");		break;
			case 1117:	tempString = copyString ("ALT+F6");		break;
			case 1118:	tempString = copyString ("ALT+F7");		break;
			case 1119:	tempString = copyString ("ALT+F8");		break;
			case 1120:	tempString = copyString ("ALT+F9");		break;
			case 1121:	tempString = copyString ("ALT+F10");	break;
			case 1122:	tempString = copyString ("ALT+F11");	break;
			case 1123:	tempString = copyString ("ALT+F12");	break;

			case 2019:	tempString = copyString ("PAUSE");		break;
			*/
			case 63276:	tempString = copyString ("PAGE UP");	break;
			case 63277:	tempString = copyString ("PAGE DOWN");	break;
			case 63275:	tempString = copyString ("END");		break;
			case 63273:	tempString = copyString ("HOME");		break;
			case 63234:	tempString = copyString ("LEFT");		break;
			case 63232:	tempString = copyString ("UP");			break;
			case 63235:	tempString = copyString ("RIGHT");		break;
			case 63233:	tempString = copyString ("DOWN");		break;
				/*
			case 2045:	tempString = copyString ("INSERT");		break;
			case 2046:	tempString = copyString ("DELETE");		break;
				*/
			case 63236:	tempString = copyString ("F1");			break;
			case 63237:	tempString = copyString ("F2");			break;
			case 63238:	tempString = copyString ("F3");			break;
			case 63239:	tempString = copyString ("F4");			break;
			case 63240:	tempString = copyString ("F5");			break;
			case 63241:	tempString = copyString ("F6");			break;
			case 63242:	tempString = copyString ("F7");			break;
			case 63243:	tempString = copyString ("F8");			break;
			case 63244:	tempString = copyString ("F9");			break;
			case 63245:	tempString = copyString ("F10");		break;
			case 63246:	tempString = copyString ("F11");		break;
			case 63247:	tempString = copyString ("F12");		break;

			default:
			if (input.keyPressed >= 256) {
				//if (captureAllKeys) {
					tempString = copyString ("ABCDEF");
					sprintf (tempString, "%i", input.keyPressed);
				//}
			} else {
				tempString = copyString (" ");
				tempString[0] = input.keyPressed;
			}
		}

		fprintf (stderr, "tempString: %s\n", tempString);
	
		if (tempString) {
			variableStack * tempStack = new variableStack;
			if (! checkNew (tempStack)) return false;
			initVarNew (tempStack -> thisVar);
			makeTextVar (tempStack -> thisVar, tempString);
			delete tempString;
			tempString = NULL;
			tempStack -> next = NULL;
			if (! startNewFunctionNum (currentEvents -> spaceFunction, 1, NULL, tempStack)) return false;
		}
	}
	input.rightClick = false;
	input.leftClick = false;
	input.rightRelease = false;
	input.leftRelease = false;
	input.keyPressed = 0;
	lastRegion = overRegion;
	return runSludge ();
//	return true;
}
