#include "allfiles.h"

#include <string.h>

#include "backdrop.h"
#include "sprites.h"
#include "sludger.h"
#include "objtypes.h"
#include "region.h"
#include "sprbanks.h"
#include "people.h"
#include "talk.h"
#include "sound.h"
#include "fonttext.h"
#include "newfatal.h"
#include "stringy.h"
#include "moreio.h"

extern int fontHeight, cameraX, cameraY, speechMode;
extern float cameraZoom;
speechStruct * speech;
ponderingStruct * pondering = NULL;
float speechSpeed = 1;

void initSpeech () {
	speech = new speechStruct;
	if (checkNew (speech)) {
		speech -> currentTalker = NULL;
		speech -> allSpeech = NULL;
		speech -> speechY = 0;
		speech -> lastFile = -1;
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

void killAllPonderings () {
	ponderingStruct *killPonder;
	speechLine * killMe;
	speechStruct *s;
	while (pondering) {
		killPonder = pondering;
		pondering = pondering->next;
		s = pondering->speech;
			
		while (s -> allSpeech) {
			killMe = s -> allSpeech;
			s -> allSpeech = s -> allSpeech -> next;
			delete killMe -> textLine;
			delete killMe;
		}		
		delete s;
		delete killPonder;
	}	
}

#define TF_max(a, b) ((a > b) ? a : b)
#define TF_min(a, b) ((a > b) ? b : a)

inline void setObjFontColour (objectType * t, speechStruct *speech) {
	setFontColour (speech -> talkCol, t -> r, t -> g, t -> b);
}

void addSpeechLine (char * theLine, int x, int & offset, speechStruct *speech) {
	int halfWidth = (stringWidth (theLine) >> 1)/cameraZoom;
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
	} else if ((xx2 >= ((float)winWidth/cameraZoom) - 5) && (offset > (((float)winWidth/cameraZoom) - 5 - xx2))) {
		offset = ((float)winWidth/cameraZoom) - 5 - xx2;
	}
}

int isThereAnySpeechGoingOn () {
	return speech -> allSpeech ? speech -> lookWhosTalking : -1;
}

int wrapSpeechXY (char * theText, int x, int y, int wrap, int sampleFile, speechStruct *speech) {
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
		theText[a] = 0;
		addSpeechLine (theText, x, offset, speech);
		theText[a] = ' ';
		theText += a + 1;
		y -= fontHeight/cameraZoom;
	}
	addSpeechLine (theText, x, offset, speech);
	y -= fontHeight/cameraZoom;

	if (y < 0) speech -> speechY -= y;
	else if (speech -> speechY > cameraY + (float)(winHeight - fontHeight/3)/cameraZoom) speech -> speechY = cameraY + (float)(winHeight - fontHeight/3)/cameraZoom;

	if (offset) {
		speechLine * viewLine = speech -> allSpeech;
		while (viewLine) {
			viewLine -> x += offset;
			viewLine = viewLine -> next;
		}
	}

	return speechTime;
}

int wrapSpeechPerson (char * theText, onScreenPerson & thePerson, int sampleFile, bool animPerson, speechStruct *speech) {
	int i = wrapSpeechXY (theText, thePerson.x - cameraX, thePerson.y - cameraY - (thePerson.scale * (thePerson.height - thePerson.floaty)) - thePerson.thisType -> speechGap, thePerson.thisType -> wrapSpeech, sampleFile, speech);
	if (animPerson) {
		makeTalker (thePerson);
		speech -> currentTalker = & thePerson;
	}
	return i;
}

int wrapSpeech (char * theText, int objT, int sampleFile, bool animPerson) {
	int i;
	
	wrapPondering((char *) "", objT);
	
	speech -> lookWhosTalking = objT;
	onScreenPerson * thisPerson = findPerson (objT);
	if (thisPerson) {
		setObjFontColour (thisPerson -> thisType, speech);
		i = wrapSpeechPerson (theText, * thisPerson, sampleFile, animPerson, speech);
	} else {
		screenRegion * thisRegion = getRegionForObject (objT);
		if (thisRegion) {
			setObjFontColour (thisRegion -> thisType, speech);
			i = wrapSpeechXY (theText, ((thisRegion -> x1 + thisRegion -> x2) >> 1) - cameraX, thisRegion -> y1 - thisRegion -> thisType -> speechGap - cameraY, thisRegion -> thisType -> wrapSpeech, sampleFile, speech);
		} else {
			objectType * temp = findObjectType (objT);
			setObjFontColour (temp, speech);
			i = wrapSpeechXY (theText, winWidth >> 1, 10, temp -> wrapSpeech, sampleFile, speech);
		}
	}
	return i;
}

void wrapPondering (char * theText, int objT) {
	// First check if the person already is pondering
	ponderingStruct *p = pondering;
	ponderingStruct *p2 = pondering;
	speechStruct *s;
	while (p) {
		s = p->speech;
		if (s->lookWhosTalking == objT) {
			speechLine * killMe;
			
			while (s -> allSpeech) {
				killMe = s -> allSpeech;
				s -> allSpeech = s -> allSpeech -> next;
				delete killMe -> textLine;
				delete killMe;
			}
			
			delete s;
			p2->next = p->next;
			if (p == pondering) pondering = NULL;
			delete p;
			break;
		} else {
			p2 = p;
			p = p->next;
		}
	}
	
	if (! strlen (theText))
		return;
	
	// Add the new pondering
	p = pondering;
	
	pondering = new ponderingStruct;
	checkNew (pondering);
	pondering -> next = p;
	pondering -> speech = new speechStruct;
	
	s = pondering -> speech;
	
	if (checkNew (s)) {
		s -> currentTalker = NULL;
		s -> allSpeech = NULL;
		s -> speechY = 0;
		s -> lastFile = -1;
	}
	
	s -> lookWhosTalking = objT;
	onScreenPerson * thisPerson = findPerson (objT);
	if (thisPerson) {
		setObjFontColour (thisPerson -> thisType, s);
		wrapSpeechPerson (theText, * thisPerson, -1, false, s);
	} else {
		screenRegion * thisRegion = getRegionForObject (objT);
		if (thisRegion) {
			setObjFontColour (thisRegion -> thisType, s);
			wrapSpeechXY (theText, ((thisRegion -> x1 + thisRegion -> x2) >> 1) - cameraX, thisRegion -> y1 - thisRegion -> thisType -> speechGap - cameraY, thisRegion -> thisType -> wrapSpeech, -1, s);
		} else {
			objectType * temp = findObjectType (objT);
			setObjFontColour (temp, s);
			wrapSpeechXY (theText, winWidth >> 1, 10, temp -> wrapSpeech, -1, s);
		}
	}
}

void viewSpeech () {
	int viewY;
	speechLine * viewLine;
	
	// Ponderings
	ponderingStruct *p = pondering;
	while (p) {
		viewY = p -> speech -> speechY;
		viewLine = p-> speech -> allSpeech;
		fixFont (p -> speech -> talkCol);
		while (viewLine) {
			drawString (viewLine -> textLine, viewLine -> x, viewY, p->speech -> talkCol);
			viewY -= fontHeight / cameraZoom;
			viewLine = viewLine -> next;
		}
		p = p -> next;
	}
	
	// And the normal speech
	viewY = speech -> speechY;
	viewLine = speech -> allSpeech;
	fixFont (speech -> talkCol);
	while (viewLine) {
		drawString (viewLine -> textLine, viewLine -> x, viewY, speech -> talkCol);
		viewY -= fontHeight / cameraZoom;
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
	speech -> currentTalker = NULL;
	killAllSpeech ();
	byte r = fgetc (fp);
	byte g = fgetc (fp);
	byte b = fgetc (fp);
	setFontColour (sS -> talkCol, r, g, b);

	speechSpeed = getFloat (fp);
	
	// Read y co-ordinate
	sS -> speechY = get2bytes (fp);

	// Read which character's talking
	sS -> lookWhosTalking = get2bytes (fp);

	if (fgetc (fp)) {
		sS -> currentTalker = findPerson (get2bytes (fp));
	} else {
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
		newOne -> x	= get2bytes (fp);
		newOne -> next = NULL;
		(* viewLine) = newOne;
		viewLine = & (newOne -> next);
	}
	return true;
}

void savePonderings (FILE *fp) {
	ponderingStruct *p = pondering;
	
	while (p) {
		fputc (1, fp);	
		saveSpeech(p->speech, fp);
		p = p->next;
	}
	fputc (0, fp);	
}

bool loadPonderings(FILE *fp) {
	ponderingStruct *p;
	speechStruct *s;

	killAllPonderings();
	while (fgetc (fp)) {
		p = pondering;
		pondering = new ponderingStruct;
		checkNew (pondering);
		pondering -> next = p;
		pondering -> speech = new speechStruct;
		
		s = pondering -> speech;
		
		if (checkNew (s)) {
			s -> currentTalker = NULL;
			s -> allSpeech = NULL;
			s -> speechY = 0;
			s -> lastFile = -1;
		}
		if (! loadSpeech (s, fp))
			return false;
		
	}
	return true;
}
