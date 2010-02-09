#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "ALLFILES.H"
#include "VARIABLE.H"
#include "NEWFATAL.H"
#include "MOREIO.H"

#define LOAD_ERROR "Can't load custom data...\n\n"

unsigned short saveEncoding = false;
char encode1 = 0;
char encode2 = 0;

extern char * gamePath;

/*
void loadSaveDebug (char * com) {
	FILE * ffpp = fopen ("debuggy.txt", "at");
	fprintf (ffpp, "%s\n", com);
	fclose (ffpp);
}

void loadSaveDebug (char com) {
	FILE * ffpp = fopen ("debuggy.txt", "at");
	fprintf (ffpp, "%c\n", com);
	fclose (ffpp);
}

void loadSaveDebug (int com) {
	FILE * ffpp = fopen ("debuggy.txt", "at");
	fprintf (ffpp, "%d\n", com);
	fclose (ffpp);
}
*/

void writeStringEncoded (const char * s, FILE * fp) {
	int a, len = strlen (s);

	put2bytes (len, fp);
//	loadSaveDebug ("WRITE: length");
//	loadSaveDebug (len);
	for (a = 0; a < len; a ++) {
//		loadSaveDebug ("WRITE: s[a]");
//		loadSaveDebug (s[a]);
//		loadSaveDebug ((int) s[a] ^ encode1);
		fputc (s[a] ^ encode1, fp);
		encode1 += encode2;
	}
}

char * readStringEncoded (FILE * fp) {
	int a, len = get2bytes (fp);
	char * s = new char[len + 1];
	if (! checkNew (s)) return NULL;
//	loadSaveDebug ("READ: length");
//	loadSaveDebug (len);
	for (a = 0; a < len; a ++) {
		s[a] = (char) (fgetc (fp) ^ encode1);
//		loadSaveDebug ("READ: s[a]");
//		loadSaveDebug (s[a]);
		encode1 += encode2;
	}
	s[len] = NULL;
	return s;
}

char * readTextPlain (FILE * fp) {
	int32_t startPos;

	int stringSize = 0;
	bool keepGoing = true;
	char gotChar;
	char * reply;

	startPos = ftell (fp);
	//fgetpos (fp, & startPos);

	while (keepGoing) {
		gotChar = (char) fgetc (fp);
		if ((gotChar == '\n') || (feof (fp))) {
			keepGoing = false;
		} else {
			stringSize ++;
		}
	}

	if ((stringSize == 0) && (feof (fp))) {
		return NULL;
	} else {
		//fsetpos (fp, &startPos);
		fseek (fp, startPos, SEEK_SET);
		reply = new char[stringSize + 1];
		if (reply == NULL) return NULL;
		fread (reply, stringSize, 1, fp);
		fgetc (fp); // Skip the newline character
		reply[stringSize] = NULL;
	}

	return reply;
}

bool fileToStack (char * filename, stackHandler * sH) {
	variable stringVar;
	stringVar.varType = SVT_NULL;
	const char * checker = saveEncoding ? "[Custom data (encoded)]\r\n" : "[Custom data (ASCII)]\n";

	FILE * fp = fopen (filename, "rb");
	if (! fp) {
		char currentDir[1000];
#ifdef _MSC_VER
		if (! _getcwd (currentDir, 998)) {
#else
		if (! getcwd (currentDir, 998)) {
#endif
			fprintf(stderr, "Can't get current directory.\n");
		}

#ifdef _MSC_VER
		_chdir (gamePath);
#else
		chdir (gamePath);
#endif
		fp = fopen (filename, "rb");
#ifdef _MSC_VER
		_chdir (currentDir);
#else
		chdir (currentDir);
#endif
		if (! fp) {
			return fatal ("No such file", filename);
		}
	}

	encode1 = (unsigned char) saveEncoding & 255;
	encode2 = (unsigned char) (saveEncoding >> 8);

	while (* checker) {
		if (fgetc (fp) != * checker) {
			fclose (fp);
			return fatal (LOAD_ERROR "This isn't a SLUDGE custom data file:", filename);
		}
		checker ++;
	}

	if (saveEncoding) {
//		loadSaveDebug ("\nCHECKING ENCODING IS THE SAME\n");
		char * checker = readStringEncoded (fp);
		if (strcmp (checker, "UN£LOåCKED")) {
			fclose (fp);
			return fatal (LOAD_ERROR "The current file encoding setting does not match the encoding setting used when this file was created:", filename);
		}
		delete checker;
		checker = NULL;
	}


	for (;;) {
		if (saveEncoding) {
			char i = fgetc (fp) ^ encode1;

			if (feof (fp)) break;
//			loadSaveDebug ("READ: type");
//			loadSaveDebug (i);
			switch (i) {
				case 0:
				{
					char * g = readStringEncoded (fp);
					makeTextVar (stringVar, g);
					delete g;
				}
				break;

				case 1:
				setVariable (stringVar, SVT_INT, get4bytes (fp));
				break;

				case 2:
				setVariable (stringVar, SVT_INT, fgetc (fp));
				break;

				default:
				fatal (LOAD_ERROR "Corrupt custom data file:", filename);
				fclose (fp);
				return false;
			}
		} else {
			char * line = readTextPlain (fp);
			if (! line) break;
			makeTextVar (stringVar, line);
		}

		if (sH -> first == NULL) {
			// Adds to the TOP of the array... oops!
			if (! addVarToStackQuick (stringVar, sH -> first)) return false;
			sH -> last = sH -> first;
		} else {
			// Adds to the END of the array... much better
			if (! addVarToStackQuick (stringVar, sH -> last -> next)) return false;
			sH -> last = sH -> last -> next;
		}
	}
	fclose (fp);
	return true;
}

bool stackToFile (char * filename, const variable & from) {
	FILE * fp = fopen (filename, saveEncoding ? "wb" : "wt");
	if (! fp) return fatal ("Can't create file", filename);

	variableStack * hereWeAre = from.varData.theStack -> first;

	encode1 = (unsigned char) saveEncoding & 255;
	encode2 = (unsigned char) (saveEncoding >> 8);

	if (saveEncoding) {
		fprintf (fp, "[Custom data (encoded)]\r\n");
		writeStringEncoded ("UN£LOåCKED", fp);
	} else {
		fprintf (fp, "[Custom data (ASCII)]\n");
	}

	while (hereWeAre) {
		if (saveEncoding) {
			switch (hereWeAre -> thisVar.varType) {
				case SVT_STRING:
				fputc (encode1, fp);
				writeStringEncoded (hereWeAre -> thisVar.varData.theString, fp);
				break;

				case SVT_INT:
				// Small enough to be stored as a char
				if (hereWeAre -> thisVar.varData.intValue >= 0 && hereWeAre -> thisVar.varData.intValue < 256) {
					fputc (2 ^ encode1, fp);
					fputc (hereWeAre -> thisVar.varData.intValue, fp);
				} else {
					fputc (1 ^ encode1, fp);
					put4bytes (hereWeAre -> thisVar.varData.intValue, fp);
				}
				break;

				default:
				fatal ("Can't create an encoded custom data file containing anything other than numbers and strings", filename);
				fclose (fp);
				return false;
			}
		} else {
			char * makeSureItsText = getTextFromAnyVar (hereWeAre -> thisVar);
			if (makeSureItsText == NULL) break;
			fprintf (fp, "%s\n", makeSureItsText);
			delete makeSureItsText;
		}

		hereWeAre = hereWeAre -> next;
	}
//	fprintf (fp, "Done!\n");
	fclose (fp);
	return true;
}
