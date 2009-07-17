#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "Splitter.hpp"
#include "percbar.h"
#include "wintext.h"
#include "winterfa.h"
#include "settings.h"
#include "HSI.h"
#include "messbox.h"
#include "moreio.h"
#include "floor.h"
#include "translation.h"
#include "dumpfiles.h"

bool dumpFileInto (FILE * writer, char * thisFile) {
	int a;
	FILE * reader = fopen (thisFile, "rb");

	if (! reader) return false;
	for (;;) {
		a = fgetc (reader);
		if (a == EOF) break;
		fputc (a, writer);
	}
	fclose (reader);
	return true;
}

int getFileType (char * filename) {
	int reply = FILETYPE_UNKNOWN;
	
	if (strlen (filename) >= 4) {
		char * compareMe = copyString(filename + (strlen (filename) - 4));
		for (int ii = 0; ii < 4; ii ++)
		{
			if (compareMe[ii] >= 'A' && compareMe[ii] <= 'Z')
				compareMe[ii] += 'a' - 'A';
		}

		if (strcmp (compareMe, ".tga") == 0) {
			reply = FILETYPE_TGA;
		} else if (strcmp (compareMe, ".flo") == 0) {
			reply = FILETYPE_FLOOR;
		} else if ( strcmp (compareMe, ".mid") == 0 ||
					strcmp (compareMe, ".rmi") == 0) {
			reply = FILETYPE_MIDI;
		} else if ( strcmp (compareMe, ".wav") == 0 ||
					strcmp (compareMe, ".mp3") == 0 ||
					strcmp (compareMe, ".ogg") == 0 ||
					strcmp (compareMe + 1, ".xm") == 0 ||
					strcmp (compareMe + 1, ".it") == 0 ||
					strcmp (compareMe, ".s3m") == 0 ||
					strcmp (compareMe, ".mod") == 0 ||
					strcmp (compareMe, ".mo3") == 0 ||
					strcmp (compareMe, ".avi") == 0) {
			reply = FILETYPE_AUDIO;
		} else if (	strcmp (compareMe, ".duc") == 0 ||
					strcmp (compareMe, ".zbu") == 0) {
			reply = FILETYPE_RAW;
		}
		delete compareMe;
	}

	return reply;
}

bool dumpFiles (FILE * mainFile, stringArray * & theSA) {

	// Display first...

//	setWindowText (COM_PROGTEXT, "Attaching sprites, images and audio files");
//	setWindowText (COM_ITEMTEXT, "");
	clearRect (countElements (theSA), P_BOTTOM);

	// Now the hard work
	unsigned long remainingIndexSize = countElements (theSA) * 4 - 4;
	unsigned long filesize;
	int i = 0, ch;
	gotoTempDirectory ();

	FILE * inFile, * outFile = fopen ("alldata.big", "wb");
	if (outFile == NULL)
		return errorBox (ERRORTYPE_SYSTEMERROR, "Can't write temporary file", "alldata.big", NULL);
	gotoSourceDirectory ();
	
	bool killAfterAdd;
	while (theSA) {
		setWindowText (COM_FILENAME, theSA -> string);
		
		killAfterAdd = false;
		if (! theSA -> string[0]) {
			// Nothing - used in forceSilent mode to replace audio	
			put4bytes (0, outFile);

			// Save the distance from here to the data in the index...
			put4bytes (remainingIndexSize, mainFile);
		} else {
			switch (getFileType (theSA->string)) {
				case FILETYPE_TGA:
				convertTGA (theSA -> string);
				killAfterAdd = programSettings.compilerKillImages;
				fprintf (stderr, "KillAfterAdd: %d (%s)", killAfterAdd, theSA -> string);
				break;
				
				case FILETYPE_FLOOR:
				convertFloor (theSA -> string);
				killAfterAdd = true;
				break;
				
				case FILETYPE_MIDI:
				errorBox (ERRORTYPE_PROJECTWARNING, "The current version of the SLUDGE engine cannot play MIDI files such as", theSA->string, NULL);
				break;
				
				case FILETYPE_UNKNOWN:
				return errorBox (ERRORTYPE_PROJECTERROR, "Tried to include a file which is not supported by SLUDGE.\n\nSupported music types: .XM, .MOD, .S3M, .IT, .MID, .RMI\nSupported sampled sound types: .WAV, .MP3, .OGG\nSupported graphic types: .TGA\nSLUDGE-specific types: .FLO, .ZBU, .DUC\n\nThe file you tried to include was:", theSA -> string, NULL);
			}
			
			inFile = fopen (theSA -> string, "rb");
			if (inFile == NULL) return errorBox (ERRORTYPE_PROJECTERROR, "Can't read resource file", theSA -> string, NULL);
			fseek (inFile, 0, 2);
			filesize = ftell (inFile);
			fseek (inFile, 0, 0);
			
			// Save the size at the start of the data...
			put4bytes (filesize, outFile);
			
			// Save the distance from here to the data in the index...
			put4bytes (remainingIndexSize, mainFile);
			
			remainingIndexSize += filesize;
			
			while (filesize --) {
				fputc (fgetc (inFile), outFile);
			}
			
			fclose (inFile);
			if (killAfterAdd) unlink (theSA -> string);
		}
		destroyFirst (theSA);
		percRect (++ i, P_BOTTOM);
	}
	
	fclose (outFile);
	gotoTempDirectory ();
	inFile = fopen ("alldata.big", "rb");
	if (! inFile) return errorBox (ERRORTYPE_SYSTEMERROR, "Can't read the file I just wrote", "alldata.big", NULL);

	setWindowText (COM_PROGTEXT, "Adding look-up table");
	for (;;) {
		ch = fgetc (inFile);
		if (feof (inFile)) break;
		fputc (ch, mainFile);
	}

	fclose (inFile);
	unlink ("alldata.big");

//	char showWhenDone[255];
//	sprintf (showWhenDone, "Added %d resources", howManyFiles);
//	addComment (showWhenDone);
	
	return true;
}

bool saveStrings (FILE * mainFile, FILE * textFile, stringArray * theSA) {
	stringArray * wholeStringThing = theSA;
	
	FILE * projectFile, * indexFile;
	int indexSize = countElements (theSA) * 4 + ftell (mainFile) + 4;

//	errorBox ("Number of unique strings: ", countElements (theSA));
	if (! gotoTempDirectory ()) return false;
	projectFile = fopen ("txtdata.tmp", "wb");
	indexFile = fopen ("txtindex.tmp", "wb");
//	textFile = fopen ("alltext.txt", "wt");
	if (! (projectFile && indexFile)) return false;

	while (theSA) {
//		printf ("Writing string %s at position %i\n", theSA -> string, (int) (ftell (projectFile) + indexSize));
		put4bytes ((ftell (projectFile) + indexSize), indexFile);
		writeString (theSA -> string, projectFile);
		if (textFile) fprintf (textFile, "%s\n", theSA -> string);
		theSA = theSA -> next;
	}

	put4bytes (ftell (projectFile) + indexSize, mainFile);

//	addComment ("Size of string file: ", ftell (projectFile) + indexSize);
	fclose (projectFile);
	fclose (indexFile);
	if (textFile) fclose (textFile);

	if (! gotoTempDirectory ()) return false;
	if (dumpFileInto (mainFile, "txtindex.tmp") && dumpFileInto (mainFile, "txtdata.tmp")) {
		unlink ("txtindex.tmp");
		unlink ("txtdata.tmp");
		return addAllTranslationData (wholeStringThing, mainFile);
	} else {
		return false;
	}
}
