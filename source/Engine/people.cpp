#include "allfiles.h"
#include <math.h>
#include <stdlib.h>

#include "version.h"
#include "sprites.h"
#include "sprites_aa.h"
#include "sprbanks.h"
#include "sludger.h"
#include "objtypes.h"
#include "region.h"
#include "people.h"
#include "talk.h"
#include "newfatal.h"
#include "variable.h"
//#include "memwatch.h"
#include "moreio.h"
#include "loadsave.h"
#include "floor.h"
#include "zbuffer.h"
#include "sound.h"
#include "debug.h"

#define ANGLEFIX (180.0 / 3.14157)
#define ANI_STAND 0
#define ANI_WALK 1
#define ANI_TALK 2

extern speechStruct * speech;

extern variableStack * noStack;

extern int ssgVersion;

extern int cameraX, cameraY;
screenRegion personRegion;
extern screenRegion * lastRegion;
extern flor * currentFloor;

extern inputType input;
onScreenPerson * allPeople = NULL;
short int scaleHorizon = 75;
short int scaleDivide = 150;
extern screenRegion * allScreenRegions;

//persona * hackPersona = NULL;

#define TF_max(a, b) ((a > b) ? a : b)

inline int TF_abs (int a) {
	return (a > 0) ? a : -a;
}

void setFrames (onScreenPerson & m, int a) {
	m.myAnim = m.myPersona -> animation[(a * m.myPersona -> numDirections) + m.direction];
}

personaAnimation * createPersonaAnim (int num, variableStack * & stacky) {
	personaAnimation * newP = new personaAnimation;
//	db ("\nCreating new animation from built-in function");
	checkNew (newP);
//	adding (newP);

	newP -> numFrames = num;
	newP -> frames = new animFrame[num];
	checkNew (newP -> frames);
//	db ("Making room for frames");
//	adding (newP -> frames);

	int a = num, frameNum, howMany;

	while (a) {
		a --;
		newP -> frames[a].noise = 0;
		if (stacky -> thisVar.varType == SVT_FILE) {
			newP -> frames[a].noise = stacky -> thisVar.varData.intValue;
		} else if (stacky -> thisVar.varType == SVT_FUNC) {
			newP -> frames[a].noise = - stacky -> thisVar.varData.intValue;
		} else if (stacky -> thisVar.varType == SVT_STACK) {
			getValueType (frameNum, SVT_INT, stacky -> thisVar.varData.theStack -> first -> thisVar);
			getValueType (howMany, SVT_INT, stacky -> thisVar.varData.theStack -> first -> next -> thisVar);
		} else {
			getValueType (frameNum, SVT_INT, stacky -> thisVar);
			howMany = 1;
		}
		trimStack (stacky);
		newP -> frames[a].frameNum = frameNum;
		newP -> frames[a].howMany = howMany;
	}

	return newP;
}

personaAnimation * makeNullAnim () {
	personaAnimation * newAnim	= new personaAnimation;
//	db ("\nCreating NULL animation");
//	adding (newAnim);

	newAnim -> theSprites		= NULL;
	newAnim -> numFrames		= 0;
	newAnim -> frames			= NULL;
	return newAnim;
}

personaAnimation * copyAnim (personaAnimation * orig) {
	int num = orig -> numFrames;

	personaAnimation * newAnim	= new personaAnimation;
//	db ("\nCopying animation");
//	adding (newAnim);

	// Copy the easy bits...
	newAnim -> theSprites		= orig -> theSprites;
	newAnim -> numFrames		= num;

	if (num) {

		// Argh! Frames! We need a whole NEW array of animFrame structures...

		newAnim -> frames = new animFrame[num];
//		db ("Copying frames");
//		adding (newAnim -> frames);

		for (int a = 0; a < num; a ++) {
			newAnim -> frames[a].frameNum = orig -> frames[a].frameNum;
			newAnim -> frames[a].howMany = orig -> frames[a].howMany;
			newAnim -> frames[a].noise = orig -> frames[a].noise;
		}
	} else {
		newAnim -> frames = NULL;
	}

	return newAnim;
}

void deleteAnim (personaAnimation * orig) {
/*	FILE * debu = fopen ("debuTURN.txt", "at");
	if (orig) {
		fprintf (debu, "\nDeleting an animation with %i frames...\n", orig -> numFrames);
	} else {
		fprintf (debu, "\nDeleting a NULL animation! Ouch!");
	}
	fclose (debu);
*/

	if (orig)
	{
		if (orig -> numFrames) {
	//		deleting (orig -> frames);
			delete orig -> frames;
		}
	//	deleting (orig);
		delete orig;
		orig = NULL;
	}
}

void turnMeAngle (onScreenPerson * thisPerson, int direc) {
	int d = thisPerson -> myPersona -> numDirections;
	thisPerson -> angle = direc;
	direc += (180 / d) + 180 + thisPerson -> angleOffset;
	while (direc >= 360) direc -= 360;
	thisPerson -> direction = (direc * d) / 360;
}

bool initPeople () {
	personRegion.sX = 0;
	personRegion.sY = 0;
	personRegion.di = -1;
	allScreenRegions = NULL;

	return true;
}

void spinStep (onScreenPerson * thisPerson) {
	int diff = (thisPerson -> angle + 360) - thisPerson -> wantAngle;
	int eachSlice = thisPerson -> spinSpeed ? thisPerson -> spinSpeed : (360 / thisPerson -> myPersona -> numDirections);
	while (diff > 180) {
		diff -= 360;
	}

	if (diff >= eachSlice) {
		turnMeAngle (thisPerson, thisPerson -> angle - eachSlice);
	} else if (diff <= - eachSlice) {
		turnMeAngle (thisPerson, thisPerson -> angle + eachSlice);
	} else {
		turnMeAngle (thisPerson, thisPerson -> wantAngle);
		thisPerson -> spinning = false;
	}
}

void rethinkAngle (onScreenPerson * thisPerson) {
	int d = thisPerson -> myPersona -> numDirections;
	int direc = thisPerson -> angle + (180 / d) + 180 + thisPerson -> angleOffset;
	while (direc >= 360) direc -= 360;
	thisPerson -> direction = (direc * d) / 360;
}

bool turnPersonToFace (int thisNum, int direc) {
	onScreenPerson * thisPerson = findPerson (thisNum);
	if (thisPerson) {
		if (thisPerson -> continueAfterWalking) abortFunction (thisPerson -> continueAfterWalking);
		thisPerson -> continueAfterWalking = NULL;
		thisPerson -> walking = false;
		thisPerson -> spinning = false;
		turnMeAngle (thisPerson, direc);
		setFrames (* thisPerson, (thisPerson == speech->currentTalker) ? ANI_TALK : ANI_STAND);
		return true;
	}
	return false;
}

bool setPersonExtra (int thisNum, int extra) {
	onScreenPerson * thisPerson = findPerson (thisNum);
	if (thisPerson) {
		thisPerson -> extra = extra;
		if (extra & EXTRA_NOSCALE) thisPerson -> scale = 1;
		return true;
	}
	return false;
}

void setScale (short int h, short int d) {
	scaleHorizon = h;
	scaleDivide = d;
}

void moveAndScale (onScreenPerson & me, float x, float y) {
	me.x = x;
	me.y = y;
	if (! (me.extra & EXTRA_NOSCALE) && scaleDivide) me.scale = (me.y - scaleHorizon) / scaleDivide;
}

onScreenPerson * findPerson (int v) {
	onScreenPerson * thisPerson = allPeople;
	while (thisPerson) {
		if (v == thisPerson -> thisType -> objectNum) break;
		thisPerson = thisPerson -> next;
	}
	return thisPerson;
}

void movePerson (int x, int y, int objNum) {
	onScreenPerson * moveMe = findPerson (objNum);
	if (moveMe) moveAndScale (* moveMe, x, y);
}

void setShown (bool h, int ob) {
	onScreenPerson * moveMe = findPerson (ob);
	if (moveMe) moveMe -> show = h;
}

void setDrawMode (int h, int ob) {
	onScreenPerson * moveMe = findPerson (ob);
	if (moveMe) moveMe -> drawMode = h;
}

extern screenRegion * overRegion;

void shufflePeople () {
	onScreenPerson * * thisReference = & allPeople;
	onScreenPerson * A, * B;

	if (! allPeople) return;

	while ((* thisReference) -> next) {
		float y1 = (* thisReference) -> y;
		if ((* thisReference) -> extra & EXTRA_FRONT) y1 += 1000;

		float y2 = (* thisReference) -> next -> y;
		if ((* thisReference) -> next -> extra & EXTRA_FRONT) y2 += 1000;

		if (y1 > y2) {
			A = (* thisReference);
			B = (* thisReference) -> next;
			A -> next = B -> next;
			B -> next = A;
			(* thisReference) = B;
		} else {
			thisReference = & ((* thisReference) -> next);
		}
	}
}

void fixPeople (int oldX, int oldY) {
	shufflePeople ();

	onScreenPerson * thisPerson = allPeople;
	personaAnimation * myAnim;

	while (thisPerson) {
		if (thisPerson -> show) {
			myAnim = thisPerson -> myAnim;
			if (myAnim != thisPerson -> lastUsedAnim) {
				thisPerson -> lastUsedAnim = myAnim;
				thisPerson -> frameNum = 0;
				thisPerson -> frameTick = myAnim -> frames[0].howMany;
			}
			int fNumSign = myAnim -> frames[thisPerson -> frameNum].frameNum;
			int m = fNumSign < 0;
			int fNum = abs (fNumSign);

			if (fNum >= myAnim -> theSprites -> bank.total) {
				fNum = 0;
				m = 2 - m;
			}
			if (m != 2) {
				int meX, meY;
				if (thisPerson -> extra & EXTRA_FIXTOSCREEN) {
					meX = thisPerson -> x;
					meY = thisPerson -> y;
				} else {
					meX = thisPerson -> x - oldX;
					meY = thisPerson -> y - oldY;
				}
				fixScaleSprite (meX, meY, myAnim -> theSprites -> bank.sprites[fNum], myAnim -> theSprites -> bank.myPalette, thisPerson -> scale, thisPerson -> drawMode, thisPerson -> floaty, ! (thisPerson -> extra & EXTRA_NOZB), !(thisPerson -> extra & EXTRA_NOLITE), oldX, oldY, m, & thisPerson->aaSettings);
			}
		}
		thisPerson = thisPerson -> next;
	}
}

void drawPeople () {
	shufflePeople ();

	onScreenPerson * thisPerson = allPeople;
	personaAnimation * myAnim = NULL;
	overRegion = NULL;

	while (thisPerson) {
		if (thisPerson -> show) {
			myAnim = thisPerson -> myAnim;
			if (myAnim != thisPerson -> lastUsedAnim) {
				thisPerson -> lastUsedAnim = myAnim;
				thisPerson -> frameNum = 0;
				thisPerson -> frameTick = myAnim -> frames[0].howMany;
				if (myAnim -> frames[thisPerson -> frameNum].noise > 0) {
					startSound(myAnim -> frames[thisPerson -> frameNum].noise, false);
					thisPerson -> frameNum ++;
					thisPerson -> frameNum %= thisPerson -> myAnim -> numFrames;
					thisPerson -> frameTick = thisPerson -> myAnim -> frames[thisPerson -> frameNum].howMany;
				} else if (myAnim -> frames[thisPerson -> frameNum].noise) {
					startNewFunctionNum (- myAnim -> frames[thisPerson -> frameNum].noise, 0, NULL, noStack);
					thisPerson -> frameNum ++;
					thisPerson -> frameNum %= thisPerson -> myAnim -> numFrames;
					thisPerson -> frameTick = thisPerson -> myAnim -> frames[thisPerson -> frameNum].howMany;
				}
			}
			int fNumSign = myAnim -> frames[thisPerson -> frameNum].frameNum;
			int m = fNumSign < 0;
			int fNum = abs (fNumSign);
			if (fNum >= myAnim -> theSprites -> bank.total) {
				fNum = 0;
				m = 2 - m;
			}
			if (m != 2) {
				bool r = false;
				float drawAtX = thisPerson->x;
				float drawAtY = thisPerson->y;
				if (! (thisPerson -> extra & EXTRA_FIXTOSCREEN))
				{
					drawAtX -= cameraX;
					drawAtY -= cameraY;
				}
				r = scaleSprite (drawAtX, drawAtY, myAnim->theSprites->bank.sprites[fNum], myAnim -> theSprites -> bank.myPalette, thisPerson -> scale, thisPerson -> drawMode, thisPerson -> floaty, ! (thisPerson -> extra & EXTRA_NOZB), ! (thisPerson -> extra & EXTRA_NOLITE), m, thisPerson -> extra & EXTRA_RECTANGULAR, & thisPerson->aaSettings);
				if (r) {
					if (thisPerson -> thisType -> screenName[0]) {
						if (personRegion.thisType != thisPerson -> thisType) lastRegion = NULL;
						personRegion.thisType = thisPerson -> thisType;
						overRegion = & personRegion;
					}
				}
			}
		}
		if (! -- thisPerson -> frameTick) {
			thisPerson -> frameNum ++;
			thisPerson -> frameNum %= thisPerson -> myAnim -> numFrames;
			thisPerson -> frameTick = thisPerson -> myAnim -> frames[thisPerson -> frameNum].howMany;
			if (thisPerson -> show && myAnim && myAnim -> frames) {
				if (myAnim -> frames[thisPerson -> frameNum].noise > 0) {
					startSound(myAnim -> frames[thisPerson -> frameNum].noise, false);
					thisPerson -> frameNum ++;
					thisPerson -> frameNum %= thisPerson -> myAnim -> numFrames;
					thisPerson -> frameTick = thisPerson -> myAnim -> frames[thisPerson -> frameNum].howMany;
				} else if (myAnim -> frames[thisPerson -> frameNum].noise) {
					startNewFunctionNum (- myAnim -> frames[thisPerson -> frameNum].noise, 0, NULL, noStack);
					thisPerson -> frameNum ++;
					thisPerson -> frameNum %= thisPerson -> myAnim -> numFrames;
					thisPerson -> frameTick = thisPerson -> myAnim -> frames[thisPerson -> frameNum].howMany;
				}
			}
		}
		thisPerson = thisPerson -> next;
	}
}

void makeTalker (onScreenPerson & me) {
	setFrames (me, ANI_TALK);
}

void makeSilent (onScreenPerson & me) {
	setFrames (me, ANI_STAND);
}

bool handleClosestPoint (int & setX, int & setY, int & setPoly) {
	int gotX = 320, gotY = 200, gotPoly = -1, i, j, xTest1, yTest1,
		xTest2, yTest2, closestX, closestY, oldJ, currentDistance = 0xFFFFF,
		thisDistance;

//	FILE * dbug = fopen ("debug_closest.txt", "at");
//	fprintf (dbug, "\nGetting closest point to %i, %i\n", setX, setY);

	for (i = 0; i < currentFloor -> numPolygons; i ++) {
		oldJ = currentFloor -> polygon[i].numVertices - 1;
		for (j = 0; j < currentFloor -> polygon[i].numVertices; j ++) {
//			fprintf (dbug, "Polygon %i, line %i... ", i, j);
			xTest1 = currentFloor -> vertex[currentFloor -> polygon[i].vertexID[j]].x;
			yTest1 = currentFloor -> vertex[currentFloor -> polygon[i].vertexID[j]].y;
			xTest2 = currentFloor -> vertex[currentFloor -> polygon[i].vertexID[oldJ]].x;
			yTest2 = currentFloor -> vertex[currentFloor -> polygon[i].vertexID[oldJ]].y;
			closestPointOnLine (closestX, closestY, xTest1, yTest1, xTest2, yTest2, setX, setY);
//			fprintf (dbug, "closest point is %i, %i... ", closestX, closestY);
			xTest1 = setX - closestX;
			yTest1 = setY - closestY;
			thisDistance = xTest1 * xTest1 + yTest1 * yTest1;
//			fprintf (dbug, "Distance squared %i\n", thisDistance);

			if (thisDistance < currentDistance) {
//				fprintf (dbug, "** We have a new winner! **\n");

				currentDistance = thisDistance;
				gotX = closestX;
				gotY = closestY;
				gotPoly = i;
			}
			oldJ = j;
		}
	}
//	fclose (dbug);

	if (gotPoly == -1) return false;
	setX = gotX;
	setY = gotY;
	setPoly = gotPoly;

	return true;
}

bool doBorderStuff (onScreenPerson * moveMe) {
	if (moveMe -> inPoly == moveMe -> walkToPoly) {
		moveMe -> inPoly = -1;
		moveMe -> thisStepX = moveMe -> walkToX;
		moveMe -> thisStepY = moveMe -> walkToY;
	} else {
		// The section in which we need to be next...
		int newPoly = currentFloor -> matrix[moveMe -> inPoly][moveMe -> walkToPoly];
		if (newPoly == -1) return false;

		// Grab the index of the second matching corner...
		int ID, ID2;
		if (! getMatchingCorners (currentFloor -> polygon[moveMe -> inPoly], currentFloor -> polygon[newPoly], ID, ID2))
			return fatal ("Not a valid floor plan!");

		// Remember that we're walking to the new polygon...
		moveMe -> inPoly = newPoly;

		// Calculate the destination position on the coincidantal line...
		int x1 = moveMe -> x, y1 = moveMe -> y;
		int x2 = moveMe -> walkToX, y2 = moveMe -> walkToY;
		int x3 = currentFloor -> vertex[ID].x, y3 = currentFloor -> vertex[ID].y;
		int x4 = currentFloor -> vertex[ID2].x, y4 = currentFloor -> vertex[ID2].y;
//		drawLine (x1, y1, x2, y2);
//		drawLine (x3, y3, x4, y4);

		int xAB = x1 - x2;
		int yAB = y1 - y2;
		int xCD = x4 - x3;
		int yCD = y4 - y3;

		double m = (yAB * (x3 - x1) - xAB * (y3 - y1));
		m /= ((xAB * yCD) - (yAB * xCD));

		if (m > 0 && m < 1) {
			moveMe -> thisStepX = x3 + m * xCD;
			moveMe -> thisStepY = y3 + m * yCD;
		} else {
			int dx13 = x1-x3, dx14 = x1-x4, dx23 = x2-x3, dx24 = x2-x4;
			int dy13 = y1-y3, dy14 = y1-y4, dy23 = y2-y3, dy24 = y2-y4;

			dx13 *= dx13; dx14 *= dx14; dx23 *= dx23; dx24 *= dx24;
			dy13 *= dy13; dy14 *= dy14; dy23 *= dy23; dy24 *= dy24;

			if (sqrt((double) dx13 + dy13) + sqrt((double) dx23 + dy23) <
				sqrt((double) dx14 + dy14) + sqrt((double) dx24 + dy24)) {
				moveMe -> thisStepX = x3;
				moveMe -> thisStepY = y3;
			} else {
				moveMe -> thisStepX = x4;
				moveMe -> thisStepY = y4;
			}
		}
	}

	float yDiff = moveMe -> thisStepY - moveMe -> y;
	float xDiff = moveMe -> x - moveMe -> thisStepX;
	if (xDiff || yDiff) {
		moveMe -> wantAngle = 180 + ANGLEFIX * atan2(xDiff, yDiff * 2);
		moveMe -> spinning = true;
	}

	setFrames (* moveMe, ANI_WALK);
	return true;
}

bool walkMe (onScreenPerson * thisPerson, bool move = true) {
	float xDiff, yDiff, maxDiff, s;

	for (;;) {
		xDiff = thisPerson -> thisStepX - thisPerson -> x;
		yDiff = (thisPerson -> thisStepY - thisPerson -> y) * 2;
		s = thisPerson -> scale * thisPerson -> walkSpeed;
		if (s < 0.2) s = 0.2;

		maxDiff = (TF_abs (xDiff) >= TF_abs (yDiff)) ? TF_abs (xDiff) : TF_abs (yDiff);

		if (TF_abs (maxDiff) > s) {
			if (thisPerson -> spinning) {
//				db ("IN WALKME (MIDDLE):");
				spinStep (thisPerson);
				setFrames (* thisPerson, ANI_WALK);
			}
			s = maxDiff / s;
			if (move)
				moveAndScale (* thisPerson,
							  thisPerson -> x + xDiff / s,
							  thisPerson -> y + yDiff / (s * 2));
			return true;
		}

		if (thisPerson -> inPoly == -1) {
			if (thisPerson -> directionWhenDoneWalking != -1) {
				thisPerson -> wantAngle = thisPerson -> directionWhenDoneWalking;
				thisPerson -> spinning = true;
//				db ("IN WALKME (INPOLY == -1):");
				spinStep (thisPerson);
			}
			break;
		}
		if (! doBorderStuff (thisPerson)) break;
	}

	thisPerson -> walking = false;
	setFrames (* thisPerson, ANI_STAND);
	moveAndScale (* thisPerson,
				  thisPerson -> walkToX,
				  thisPerson -> walkToY);
	return false;
}

bool makeWalkingPerson (int x, int y, int objNum, loadedFunction * func, int di) {
	if (x == 0 && y == 0) return false;
	if (currentFloor -> numPolygons == 0) return false;
	onScreenPerson * moveMe = findPerson (objNum);
	if (! moveMe) return false;

	if (moveMe -> continueAfterWalking) abortFunction (moveMe -> continueAfterWalking);
	moveMe -> continueAfterWalking = NULL;
	moveMe -> walking = true;
	moveMe -> directionWhenDoneWalking = di;

	moveMe -> walkToX = x;
	moveMe -> walkToY = y;
	moveMe -> walkToPoly = inFloor (x, y);
	if (moveMe -> walkToPoly == -1) {
		if (! handleClosestPoint (moveMe -> walkToX, moveMe -> walkToY, moveMe -> walkToPoly)) return false;
	}

	moveMe -> inPoly = inFloor (moveMe -> x, moveMe -> y);
	if (moveMe -> inPoly == -1) {
		int xxx = moveMe -> x, yyy = moveMe -> y;
		if (! handleClosestPoint (xxx, yyy, moveMe -> inPoly)) return false;
	}

	doBorderStuff (moveMe);
	if (walkMe (moveMe, false) || moveMe -> spinning) {
		moveMe -> continueAfterWalking = func;
		return true;
	} else {
		return false;
	}
}

bool stopPerson (int o) {
	onScreenPerson * moveMe = findPerson (o);
	if (moveMe)
		if (moveMe -> continueAfterWalking) {
			abortFunction (moveMe -> continueAfterWalking);
			moveMe -> continueAfterWalking = NULL;
			moveMe -> walking = false;
			moveMe -> spinning = false;
			setFrames (* moveMe, ANI_STAND);
			return true;
		}
	return false;
}

bool forceWalkingPerson (int x, int y, int objNum, loadedFunction * func, int di) {
	if (x == 0 && y == 0) return false;
	onScreenPerson * moveMe = findPerson (objNum);
	if (! moveMe) return false;

	if (moveMe -> continueAfterWalking) abortFunction (moveMe -> continueAfterWalking);
	moveMe -> walking = true;
	moveMe -> continueAfterWalking = NULL;
	moveMe -> directionWhenDoneWalking = di;

	moveMe -> walkToX = x;
	moveMe -> walkToY = y;

	// Let's pretend the start and end points are both in the same
	// polygon (which one isn't important)
	moveMe -> inPoly = 0;
	moveMe -> walkToPoly = 0;

	doBorderStuff (moveMe);
	if (walkMe (moveMe) || moveMe -> spinning) {
		moveMe -> continueAfterWalking = func;
		return true;
	} else {
		return false;
	}
}

void jumpPerson (int x, int y, int objNum) {
	if (x == 0 && y == 0) return;
	onScreenPerson * moveMe = findPerson (objNum);
	if (! moveMe) return;
	if (moveMe -> continueAfterWalking) abortFunction (moveMe -> continueAfterWalking);
	moveMe -> continueAfterWalking = NULL;
	moveMe -> walking = false;
	moveMe -> spinning = false;
	moveAndScale (* moveMe, x, y);
}

bool floatCharacter (int f, int objNum) {
	onScreenPerson * moveMe = findPerson (objNum);
	if (! moveMe) return false;
	moveMe -> floaty = f;
	return true;
}

bool setCharacterWalkSpeed (int f, int objNum) {
	if (f <= 0) return false;
	onScreenPerson * moveMe = findPerson (objNum);
	if (! moveMe) return false;
	moveMe -> walkSpeed = f;
	return true;
}

void walkAllPeople () {
	onScreenPerson * thisPerson = allPeople;

//	db ("");

	while (thisPerson) {
		if (thisPerson -> walking) {
			walkMe (thisPerson);
		} else if (thisPerson -> spinning) {
//			db ("FROM WALKALLPEOPLE:");
			spinStep (thisPerson);
			setFrames (* thisPerson, ANI_STAND);
		}
		if ((! thisPerson -> walking) && (! thisPerson -> spinning) && thisPerson -> continueAfterWalking) {
			restartFunction (thisPerson -> continueAfterWalking);
			thisPerson -> continueAfterWalking = NULL;
		}
		thisPerson = thisPerson -> next;
	}
}

bool addPerson (int x, int y, int objNum, persona * p) {
	onScreenPerson * newPerson = new onScreenPerson;
	if (! checkNew (newPerson)) return false;

//	debug ("addPerson start");

	// EASY STUFF
	newPerson -> thisType = loadObjectType (objNum);
	newPerson -> scale = 1;
	newPerson -> extra = 0;
	newPerson -> continueAfterWalking = NULL;
	moveAndScale (* newPerson, x, y);
	newPerson -> frameNum = 0;
	newPerson -> walkToX = x;
	newPerson -> walkToY = y;
	newPerson -> walking = false;
	newPerson -> spinning = false;
	newPerson -> show = true;
	newPerson -> direction = 0;
	newPerson -> angle = 180;
	newPerson -> wantAngle = 180;
	newPerson -> angleOffset = 0;
	newPerson -> floaty = 0;
	newPerson -> walkSpeed = newPerson -> thisType -> walkSpeed;
	newPerson -> myAnim = NULL;
	newPerson -> spinSpeed = newPerson -> thisType -> spinSpeed;
	newPerson -> drawMode = 0;
	newPerson -> myPersona = p;

	aaCopy (& newPerson->aaSettings, & newPerson->thisType->antiAliasingSettings);

	setFrames (* newPerson, ANI_STAND);

	// HEIGHT (BASED ON 1st FRAME OF 1st ANIMATION... INC. SPECIAL CASES)
	int fNumSigned = p -> animation[0] -> frames[0].frameNum;
	int fNum = abs (fNumSigned);
	if (fNum >= p -> animation[0] -> theSprites -> bank.total) {
		if (fNumSigned < 0) {
			newPerson -> height = 5;
		} else {
			newPerson -> height = p -> animation[0] -> theSprites -> bank.sprites[0].yhot + 5;
		}
	} else {
		newPerson -> height = p -> animation[0] -> theSprites -> bank.sprites[fNum].yhot + 5;
	}

	// NOW ADD IT IN THE RIGHT PLACE
	onScreenPerson * * changethat = & allPeople;

	while (((* changethat) != NULL) && ((* changethat) -> y < y))
		changethat = & ((* changethat) -> next);

	newPerson -> next = (* changethat);
	(* changethat) = newPerson;

	return (bool) (newPerson -> thisType != NULL);
}

int timeForAnim (personaAnimation * fram) {
	int total = 0;
	for (int a = 0; a < fram -> numFrames; a ++) {
		total += fram -> frames[a].howMany;
	}
	return total;
}

void animatePerson (int obj, personaAnimation * fram) {	// Set a new SINGLE animation
	onScreenPerson * moveMe = findPerson (obj);
	if (moveMe) {
		if (moveMe -> continueAfterWalking) abortFunction (moveMe -> continueAfterWalking);
		moveMe -> continueAfterWalking = NULL;
		moveMe -> walking = false;
		moveMe -> spinning = false;
		moveMe -> myAnim = fram;
	}
}

void animatePerson (int obj, persona * per) {			// Set a new costume
	onScreenPerson * moveMe = findPerson (obj);
	if (moveMe) {
	//	if (moveMe -> continueAfterWalking) abortFunction (moveMe -> continueAfterWalking);
	//	moveMe -> continueAfterWalking = NULL;
	//	moveMe -> walking = false;
		moveMe -> spinning = false;
		moveMe -> myPersona = per;
		rethinkAngle (moveMe);
		if (moveMe-> walking) {
			setFrames (* moveMe, ANI_WALK);
		} else {
			setFrames (* moveMe, ANI_STAND);
		}
	}
}

void killAllPeople () {
	onScreenPerson * killPeople;
	while (allPeople) {
		if (allPeople -> continueAfterWalking) abortFunction (allPeople -> continueAfterWalking);
		allPeople -> continueAfterWalking = NULL;
		killPeople = allPeople;
		allPeople = allPeople -> next;
		removeObjectType (killPeople -> thisType);
		delete killPeople;
	}
}

void killMostPeople () {
	onScreenPerson * killPeople;
	onScreenPerson * * lookyHere = & allPeople;

	while (* lookyHere) {
		if ((* lookyHere) -> extra & EXTRA_NOREMOVE) {
			lookyHere = & (* lookyHere) -> next;
		} else {
			killPeople = (* lookyHere);

			// Change last pointer to NEXT in the list instead
			(* lookyHere) = killPeople -> next;

			// Gone from the list... now free some memory
			if (killPeople -> continueAfterWalking) abortFunction (killPeople -> continueAfterWalking);
			killPeople -> continueAfterWalking = NULL;
			removeObjectType (killPeople -> thisType);
			delete killPeople;
		}
	}
}

void removeOneCharacter (int i) {
	onScreenPerson * p = findPerson (i);

	if (p) {
		if (overRegion == &personRegion && overRegion->thisType == p->thisType) {
			overRegion = NULL;
		}

		if (p -> continueAfterWalking) abortFunction (p -> continueAfterWalking);
		p -> continueAfterWalking = NULL;
		onScreenPerson * * killPeople;

		for (killPeople = & allPeople;
			* killPeople != p;
			killPeople = & ((* killPeople) -> next)) {;}

		* killPeople = p -> next;
		removeObjectType (p -> thisType);
		delete p;
	}
}

bool saveAnim (personaAnimation * p, FILE * fp) {
	put2bytes (p -> numFrames, fp);
	if (p -> numFrames) {
		put4bytes (p -> theSprites -> ID, fp);

		for (int a = 0; a < p -> numFrames; a ++) {
			put4bytes (p -> frames[a].frameNum, fp);
			put4bytes (p -> frames[a].howMany, fp);
			put4bytes (p -> frames[a].noise, fp);
		}
	}
	return true;
}

bool loadAnim (personaAnimation * p, FILE * fp) {
	p -> numFrames = get2bytes (fp);

	if (p -> numFrames) {
		int a = get4bytes (fp);
		p -> frames = new animFrame[p -> numFrames];
		if (! checkNew (p -> frames)) return false;
		p -> theSprites = loadBankForAnim (a);

		for (a = 0; a < p -> numFrames; a ++) {
			p -> frames[a].frameNum = get4bytes (fp);
			p -> frames[a].howMany = get4bytes (fp);
			if (ssgVersion >= VERSION(2,0)) {
				p -> frames[a].noise = get4bytes (fp);
			} else {
				p -> frames[a].noise = 0;
			}
		}
	} else {
		p -> theSprites = NULL;
		p -> frames = NULL;
	}
	return true;
}
/*
void debugCostume (char * message, persona * cossy) {
	FILE * db = fopen ("debuTURN.txt", "at");
	fprintf (db, "  %s costume with %i directions...\n", message, cossy -> numDirections);
	for (int a = 0; a < cossy -> numDirections * 3; a ++) {
		fprintf (db, "      %i frames:", cossy -> animation[a] -> numFrames);
		for (int b = 0; b < cossy -> animation[a] -> numFrames; b ++) {
			fprintf (db, " %i", cossy -> animation[a] -> frames[b]);
		}
		fprintf (db, "\n");

	}
	fclose (db);
}
*/
bool saveCostume (persona * cossy, FILE * fp) {
	int a;
	put2bytes (cossy -> numDirections, fp);
	for (a = 0; a < cossy -> numDirections * 3; a ++) {
		if (! saveAnim (cossy -> animation[a], fp)) return false;
	}
//	debugCostume ("Saved", cossy);
	return true;
}

bool loadCostume (persona * cossy, FILE * fp) {
	int a;
	cossy -> numDirections = get2bytes (fp);
	cossy -> animation = new personaAnimation * [cossy -> numDirections * 3];
	if (! checkNew (cossy -> animation)) return false;
	for (a = 0; a < cossy -> numDirections * 3; a ++) {
		cossy -> animation[a] = new personaAnimation;
		if (! checkNew (cossy -> animation[a])) return false;

		if (! loadAnim (cossy -> animation[a], fp)) return false;
	}
//	debugCostume ("Loaded", cossy);
	return true;
}

bool savePeople (FILE * fp) {
	onScreenPerson * me = allPeople;
	int countPeople = 0, a;

	putSigned (scaleHorizon, fp);
	putSigned (scaleDivide, fp);

	while (me) {
		countPeople ++;
		me = me -> next;
	}

	put2bytes (countPeople, fp);

	me = allPeople;
	for (a = 0; a < countPeople; a ++) {

		putFloat (me -> x, fp);
		putFloat (me -> y, fp);

		saveCostume (me -> myPersona, fp);
		saveAnim (me -> myAnim, fp);
		fputc (me -> myAnim == me -> lastUsedAnim, fp);

		putFloat (me -> scale, fp);

		put2bytes (me -> extra, fp);
		put2bytes (me -> height, fp);
		put2bytes (me -> walkToX, fp);
		put2bytes (me -> walkToY, fp);
		put2bytes (me -> thisStepX, fp);
		put2bytes (me -> thisStepY, fp);
		put2bytes (me -> frameNum, fp);
		put2bytes (me -> frameTick, fp);
		put2bytes (me -> walkSpeed, fp);
		put2bytes (me -> spinSpeed, fp);
		putSigned (me -> floaty, fp);
		fputc (me -> show, fp);
		fputc (me -> walking, fp);
		fputc (me -> spinning, fp);
		if (me -> continueAfterWalking) {
			fputc (1, fp);
			saveFunction (me -> continueAfterWalking, fp);
		} else {
			fputc (0, fp);
		}
		put2bytes (me -> direction, fp);
		put2bytes (me -> angle, fp);
		put2bytes (me -> angleOffset, fp);
		put2bytes (me -> wantAngle, fp);
		putSigned (me -> directionWhenDoneWalking, fp);
		putSigned (me -> inPoly, fp);
		putSigned (me -> walkToPoly, fp);
		put2bytes (me -> drawMode, fp);

		saveObjectRef (me -> thisType, fp);

		// Anti-aliasing settings
		aaSave (me -> aaSettings, fp);

		me = me -> next;
	}
	return true;
}

bool loadPeople (FILE * fp) {
	onScreenPerson * * pointy = & allPeople;
	onScreenPerson * me;

	scaleHorizon = getSigned (fp);
	scaleDivide = getSigned (fp);

	int countPeople = get2bytes (fp);
	int a;

	allPeople = NULL;
	for (a = 0; a < countPeople; a ++) {
		me = new onScreenPerson;
		if (! checkNew (me)) return false;

		me -> myPersona = new persona;
		if (! checkNew (me -> myPersona)) return false;

		me -> myAnim = new personaAnimation;
		if (! checkNew (me -> myAnim)) return false;

		me -> x = getFloat (fp);
		me -> y = getFloat (fp);

		loadCostume (me -> myPersona, fp);
		loadAnim (me -> myAnim, fp);

		me -> lastUsedAnim = fgetc (fp) ? me -> myAnim : NULL;

		me -> scale = getFloat (fp);

		me -> extra = get2bytes (fp);
		me -> height = get2bytes (fp);
		me -> walkToX = get2bytes (fp);
		me -> walkToY = get2bytes (fp);
		me -> thisStepX = get2bytes (fp);
		me -> thisStepY = get2bytes (fp);
		me -> frameNum = get2bytes (fp);
		me -> frameTick = get2bytes (fp);
		me -> walkSpeed = get2bytes (fp);
		me -> spinSpeed = get2bytes (fp);
		me -> floaty = getSigned (fp);
		me -> show = fgetc (fp);
		me -> walking = fgetc (fp);
		me -> spinning = fgetc (fp);
		if (fgetc (fp)) {
			me -> continueAfterWalking = loadFunction (fp);
			if (! me -> continueAfterWalking) return false;
		} else {
			me -> continueAfterWalking = NULL;
		}
		me -> direction = get2bytes(fp);
		me -> angle = get2bytes(fp);
		if (ssgVersion >= VERSION(2,0)) {
			me -> angleOffset = get2bytes(fp);
		} else {
			me -> angleOffset = 0;
		}
		me -> wantAngle = get2bytes(fp);
		me -> directionWhenDoneWalking = getSigned(fp);
		me -> inPoly = getSigned(fp);
		me -> walkToPoly = getSigned(fp);
		me -> drawMode = get2bytes(fp);
		me -> thisType = loadObjectRef (fp);

		// Anti-aliasing settings
		if (ssgVersion >= VERSION(1,6))
		{
			aaLoad (me -> aaSettings, fp);
		} else {
			aaCopy (& me->aaSettings, &me->thisType->antiAliasingSettings);
		}

		me -> next = NULL;
		* pointy = me;
		pointy = & (me -> next);
	}
//	db ("End of loadPeople");
	return true;
}
