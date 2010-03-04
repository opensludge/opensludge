#include "ALLFILES.H"

#include <string.h>

#include "BACKDROP.H"
#include "sprites.h"
#include "SLUDGER.H"
#include "OBJTYPES.H"
#include "REGION.H"
#include "SPRBANKS.H"
#include "PEOPLE.H"
#include "TALK.H"
#include "sound.h"
#include "FONTTEXT.H"
#include "NEWFATAL.H"
#include "STRINGY.H"
#include "MOREIO.H"

extern int fontHeight, cameraX, cameraY, speechMode;
speechStruct * speech;
float speechSpeed = 1;

void initSpeech () {
	speech = new speechStruct;
	if (checkNew (speech)) {
		speech -> currentTalker = NULL;
		speech -> allSpeech = NULL;
		speech -> speechY = 0;
		speech -> lastFile = -1;
		createFontPalette (speech -> talkCol);
	}
}

void killAllSpeech () {
	if (speech -> lastFile != -1) {
		huntKillSound (speech -> lastFile);
		speech -> lastFile = -1;
	}

	if (speech -> currentTalker) {
		makeSilent (* (speech -> currentTalker));
		speech -> currentTalker = NULL;
	}
	
	speechLine * killMe;
	
	while (speech -> allSpeech) {
		killMe = speech -> allSpeech;
		speech -> allSpeech = speech -> allSpeech -> next;
		delete killMe -> textLine;
		delete killMe;
	}
}

#define TF_max(a, b) ((a > b) ? a : b)
#define TF_min(a, b) ((a > b) ? b : a)

inline void setObjFontColour (objectType * t) {
	setFontColour (speech -> talkCol, t -> r, t -> g, t -> b);
}

void addSpeechLine (char * theLine, int x, int & offset) {
	int halfWidth = stringWidth (theLine) >> 1;
	int xx1 = x - (halfWidth);
	int xx2 = x + (halfWidth);
	speechLine * newLine = new speechLine;
	checkNew (newLine);

	newLine -> next = speech -> allSpeech;
	newLine -> textLine = copyString (theLine);
	newLine -> x = xx1;
	speech -> allSpeech = newLine;
	if ((xx1 < 5) && (offset < (5 - xx1))) {
		offset = 5 - xx1;
	} else if ((xx2 >= winWidth - 5) && (offset > (winWidth - 5 - xx2))) {
		offset = winWidth - 5 - xx2;
	}
}

int isThereAnySpeechGoingOn () {
	return speech -> allSpeech ? speech -> lookWhosTalking : -1;
}

int wrapSpeechXY (char * theText, int x, int y, int wrap, int sampleFile) {
	int a, offset = 0;
	killAllSpeech ();

	int speechTime = (strlen (theText) + 20) * speechSpeed;
	if (speechTime < 1) speechTime = 1;

	if (sampleFile != -1) {
		if (speechMode >= 1) {
			if (startSound (sampleFile, false)) {
				speechTime = -10;
				speech -> lastFile = sampleFile;
				if (speechMode == 2) return -10;
			}
		}
	}
	speech -> speechY = y;

	while (strlen (theText) > wrap) {
		a = wrap;
		while (theText[a] != ' ') {
			a --;
			if (a == 0) {
				a = wrap;
				break;
			}
		}
		theText[a] = NULL;
		addSpeechLine (theText, x, offset);
		theText[a] = ' ';
		theText += a + 1;
		y -= fontHeight;
	}
	addSpeechLine (theText, x, offset);
	y -= fontHeight;

	if (y < 0) speech -> speechY -= y;
	else if (speech -> speechY > cameraY + winHeight - fontHeight/3) speech -> speechY = cameraY + winHeight - fontHeight/3;

	if (offset) {
		speechLine * viewLine = speech -> allSpeech;
		while (viewLine) {
			viewLine -> x += offset;
			viewLine = viewLine -> next;
		}
	}

	return speechTime;
}

int wrapSpeechPerson (char * theText, onScreenPerson & thePerson, int sampleFile, bool animPerson) {
	int i = wrapSpeechXY (theText, thePerson.x - cameraX, thePerson.y - cameraY - (thePerson.scale * (thePerson.height - thePerson.floaty)) - thePerson.thisType -> speechGap, thePerson.thisType -> wrapSpeech, sampleFile);
	if (animPerson) {
		makeTalker (thePerson);
		speech -> currentTalker = & thePerson;
	}
	return i;
}

int wrapSpeech (char * theText, int objT, int sampleFile, bool animPerson) {
	int i;
	speech -> lookWhosTalking = objT;
	onScreenPerson * thisPerson = findPerson (objT);
	if (thisPerson) {
		setObjFontColour (thisPerson -> thisType);
		i = wrapSpeechPerson (theText, * thisPerson, sampleFile, animPerson);
	} else {
		screenRegion * thisRegion = getRegionForObject (objT);
		if (thisRegion) {
			setObjFontColour (thisRegion -> thisType);
			i = wrapSpeechXY (theText, ((thisRegion -> x1 + thisRegion -> x2) >> 1) - cameraX, thisRegion -> y1 - thisRegion -> thisType -> speechGap - cameraY, thisRegion -> thisType -> wrapSpeech, sampleFile);
		} else {
			objectType * temp = findObjectType (objT);
			setObjFontColour (temp);
			i = wrapSpeechXY (theText, winWidth >> 1, 10, temp -> wrapSpeech, sampleFile);
		}
	}
	return i;
}

void viewSpeech () {
	int viewY = speech -> speechY;
	speechLine * viewLine = speech -> allSpeech;
	fixFont (speech -> talkCol);
	while (viewLine) {
		pasteString (viewLine -> textLine, viewLine -> x, viewY, speech -> talkCol);
		viewY -= fontHeight;
		viewLine = viewLine -> next;
	}
}

void saveSpeech (speechStruct * sS, FILE * fp) {
	speechLine * viewLine = sS -> allSpeech;

	fputc (sS -> talkCol.originalRed, fp);
	fputc (sS -> talkCol.originalGreen, fp);
	fputc (sS -> talkCol.originalBlue, fp);
	
	putFloat (speechSpeed, fp);
	
		// Write y co-ordinate
		put2bytes (sS -> speechY, fp);
		
		// Write which character's talking
		put2bytes (sS -> lookWhosTalking, fp);		
		if (sS -> currentTalker) {
			fputc (1, fp);
			put2bytes (sS -> currentTalker -> thisType -> objectNum, fp);
		} else {
			fputc (0, fp);
		}
		
		// Write what's being said
		while (viewLine) {
			fputc (1, fp);
			writeString (viewLine -> textLine, fp);
			put2bytes (viewLine -> x, fp);
			viewLine = viewLine -> next;
		}
		fputc (0, fp);
}

bool loadSpeech (speechStruct * sS, FILE * fp) {
//	debugOut ("About to kill all speech...");
	speech -> currentTalker = NULL;
	killAllSpeech ();
//	debugOut (" done!\n");
	byte r = fgetc (fp);
	byte g = fgetc (fp);
	byte b = fgetc (fp);
	setFontColour (sS -> talkCol, r, g, b);

	speechSpeed = getFloat (fp);
	
	// Read y co-ordinate
	sS -> speechY = get2bytes (fp);
//	debugOut ("SpeechY =", sS -> speechY);

	// Read which character's talking
	sS -> lookWhosTalking = get2bytes (fp);
//	debugOut ("lookWhosTalking =", sS -> lookWhosTalking);

	if (fgetc (fp)) {
//		debugOut ("That's a person! I'm looking for who it is!");
		sS -> currentTalker = findPerson (get2bytes (fp));
	} else {
//		debugOut ("That's NOT a person!");
		sS -> currentTalker = NULL;
	}
		
	// Read what's being said
	speechLine * * viewLine = & sS -> allSpeech;
	speechLine * newOne;
	speech -> lastFile = -1;
	while (fgetc (fp)) {
		newOne = new speechLine;
		if (! checkNew (newOne)) return false;
		newOne -> textLine = readString (fp);
//		debugOut ("TEXT: ", newOne -> textLine);
		newOne -> x	= get2bytes (fp);
//		debugOut ("X:    ", newOne -> x);
		newOne -> next = NULL;
		(* viewLine) = newOne;
		viewLine = & (newOne -> next);
	}

//	debugOut ("About to return...");
	return true;
}