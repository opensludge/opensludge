#include <windows.h>
#include <stdio.h>
#include "data.h"
#include "line.h"

#define CLOSENESS 8

extern int HORZ_RES, VERT_RES;
polyList * firstPoly = NULL;

// This is very naughty... prototype for a function in another file
void alert (char * txt);

void splitLine (int x1, int y1, int x2, int y2) {
	polyList * pL = firstPoly;
	while (pL) {
		vertexList * vL = pL -> firstVertex;
		if (vL) {
			while (vL -> next) {
				if ((vL -> x == x1 && vL -> y == y1 && vL -> next -> x == x2 && vL -> next -> y == y2) ||
					(vL -> x == x2 && vL -> y == y2 && vL -> next -> x == x1 && vL -> next -> y == y1)) {
					vertexList * newV = new vertexList;
					if (! newV) return;
					newV -> x = (x1 + x2) >> 1;
					newV -> y = (y1 + y2) >> 1;
					newV -> next = vL -> next;
					vL -> next = newV;
					break;
				}
				vL = vL -> next;
			}
			pL = pL -> next;
		}
	}
}

BOOL snapToClosest (int & x, int & y) {
	polyList * pL =  firstPoly;
	if (x < 4) x = 0;
	if (y < 4) y = 0;
	if (x > HORZ_RES - 5) x = HORZ_RES - 1;
	if (y > VERT_RES - 5) y = VERT_RES - 1;
	while (pL) {
		vertexList * vL = pL -> firstVertex;
		while (vL) {
			if (abs (vL -> x - x) < CLOSENESS && abs (vL -> y - y) < CLOSENESS) {
				x = vL -> x;
				y = vL -> y;
				return TRUE;
			}
			vL = vL -> next;
		}
		pL = pL -> next;
	}
	return FALSE;
}

void removePoly (polyList * killMe) {
	polyList * * hunt = & firstPoly;
	while (* hunt) {
		if (* hunt == killMe) {
			polyList * killer = (* hunt);
			* hunt = (* hunt) -> next;
			delete killer;
			if (! firstPoly) addPoly ();
			break;
		} else {
			hunt = & ((* hunt) -> next);
		}
	}
}

void killVertex (int x, int y) {
	polyList * pL =  firstPoly;
	while (pL) {
		vertexList * * changeMe = & (pL -> firstVertex);
		BOOL killedAlready = FALSE;
		while (* changeMe) {
			if ((* changeMe) -> x == x && (* changeMe) -> y == y) {
				if (killedAlready) {

					// It was the first and final (with more stuff in between), so fix the new ends instead of deleting
					(* changeMe) -> x = pL -> firstVertex -> x;
					(* changeMe) -> y = pL -> firstVertex -> y;
					changeMe = & ((* changeMe) -> next);
				} else {
				
					// It's just a normal situation
					vertexList * killMe = (* changeMe);
					(* changeMe) = (* changeMe) -> next;
					delete killMe;
					killedAlready = TRUE;
				}
			} else {
				changeMe = & ((* changeMe) -> next);
			}				
		}
		
		// If we're down to 0 corners, destroy the polygon
		
		polyList * killMe = ((pL -> firstVertex == NULL) ? pL : NULL);
		
		// Or 1 corner, for that matter
		
		if (! killMe) {
			if (pL -> firstVertex -> next == NULL) {
				delete pL -> firstVertex;
				killMe = pL;
			}
		}
		
		// Or 2 the same...
		
		if (! killMe) {
			if (pL -> firstVertex -> x == pL -> firstVertex -> next -> x &&
				pL -> firstVertex -> y == pL -> firstVertex -> next -> y) {
				delete pL -> firstVertex -> next;
				delete pL -> firstVertex;
				killMe = pL;
			}
		}

		// OK, we're done! Next polygon please!

		pL = pL -> next;
		if (killMe) removePoly (killMe);
	}
}

BOOL moveVertices (int x1, int y1, int x2, int y2) {
	polyList * pL;
	vertexList * vL;
	BOOL got1, got2;

	// Check we're not doubling up...

	pL = firstPoly;
	while (pL) {
		got1 = FALSE;
		got2 = FALSE;
		vL = pL -> firstVertex;
		while (vL) {
			if (vL -> x == x1 && vL -> y == y1) got1 = TRUE;
			if (vL -> x == x2 && vL -> y == y2) got2 = TRUE;
			vL = vL -> next;
		}
		if (got1 && got2) return FALSE;
		pL = pL -> next;
	}
	
	// Now more all appropriate points...
	
	pL = firstPoly;
	while (pL) {
		vL = pL -> firstVertex;
		while (vL) {
			if (vL -> x == x1 && vL -> y == y1) {
				vL -> x = x2;
				vL -> y = y2;
			}
			vL = vL -> next;
		}
		pL = pL -> next;
	}
	return TRUE;
}

polyList * addPoly () {
	polyList * newPoly = new polyList;
	if (newPoly == NULL) return NULL;
	newPoly -> next = firstPoly;
	newPoly -> firstVertex = NULL;
	firstPoly = newPoly;
	return newPoly;
}

void noFloor () {
	while (firstPoly) {
		while (firstPoly -> firstVertex) {
			vertexList * killMe = firstPoly -> firstVertex;
			firstPoly -> firstVertex = killMe -> next;
			delete killMe;
		}
		polyList * killPoly = firstPoly;
		firstPoly = firstPoly -> next;
		delete killPoly;
	}
	addPoly ();
}

BOOL polyIsComplete () {
	if (firstPoly -> firstVertex == NULL) return FALSE;
	if (firstPoly -> firstVertex -> next == NULL) return FALSE;
	vertexList * newVertex = firstPoly -> firstVertex;
	while (newVertex -> next) newVertex = newVertex -> next;
	return firstPoly -> firstVertex -> x == newVertex -> x && firstPoly -> firstVertex -> y == newVertex -> y;
}

int addVertex (int x, int y) {

	// Let's return 2 if the floor's complete...
	if (polyIsComplete ()) return 2;
	
	// Let's return 3 if the chosen point is already used here...
	vertexList * newVertex = firstPoly -> firstVertex;
	while (newVertex) {
		if (newVertex -> next && x == newVertex -> x && y == newVertex -> y) return 3;
		newVertex = newVertex -> next;
	}

	// Let's return 0 if we can't create a new vertexLest thingy...
	newVertex = new vertexList;
	if (newVertex == NULL) return 0;
	
	// Wow! Now all we need to do is set the values and update the list!
	newVertex -> x = x;
	newVertex -> y = y;
	newVertex -> next = firstPoly -> firstVertex;
	firstPoly -> firstVertex = newVertex;
	
	// It worked, so...
	return 1;
}

extern BOOL markVertices;

void drawSoFar (unsigned short adder) {
	polyList * pL = firstPoly;
	vertexList * vL;
	vertexList * drawnVertices = NULL;

	while (pL) {
		vL = pL -> firstVertex;
		while (vL) {
			
			// Draw the line from here to the next point
		
			if (vL -> next) {
				drawLine (vL -> x, vL -> y, vL -> next -> x, vL -> next -> y, adder);
			}
			
			// Do we want to draw crosses at the corners?
			
			if (markVertices) {
				vertexList * newV = drawnVertices;
				BOOL alreadyDrawn = FALSE;

				while (newV) {
					if (newV -> x == vL -> x && newV -> y == vL -> y) break;
					newV = newV -> next;
				}
				
				// We haven't drawn this cross already

				if (! newV) {
				
					// Remember we've drawn it
				
					newV = new vertexList;
					if (newV) {
						newV -> next = drawnVertices;
						drawnVertices = newV;
						newV -> x = vL -> x;
						newV -> y = vL -> y;
					}
					
					// Draw it
					
					drawLine (vL -> x - 5, vL -> y - 5, vL -> x + 5, vL -> y + 5, adder);
					drawLine (vL -> x + 5, vL -> y - 5, vL -> x - 5, vL -> y + 5, adder);
				}
			}
			vL = vL -> next;
		}
		pL = pL -> next;
	}
	
	if (! polyIsComplete () && firstPoly -> firstVertex) {
		vL = firstPoly -> firstVertex;
		int x = vL -> x;
		int y = vL -> y;
		drawLine (x - 5, y - 5, x + 5, y - 5, adder);
		drawLine (x + 5, y - 5, x + 5, y + 5, adder);
		drawLine (x + 5, y + 5, x - 5, y + 5, adder);
		drawLine (x - 5, y + 5, x - 5, y - 5, adder);
		
		while (vL -> next) vL = vL -> next;
		x = vL -> x;
		y = vL -> y;
		drawLine (x - 7, y, x, y - 7, adder);
		drawLine (x, y - 7, x + 7, y, adder);
		drawLine (x + 7, y, x, y + 7, adder);
		drawLine (x, y + 7, x - 7, y, adder);
	}

	while (drawnVertices) {
		vertexList * k = drawnVertices;
		drawnVertices = drawnVertices -> next;
		delete k;
	}
}

BOOL saveToFile (char * filename) {
	FILE * fp = fopen (filename, "wt");
	if (! fp) {
		alert ("Can't open file for writing");
		return FALSE;
	}
	polyList * pL =  firstPoly;
	while (pL) {
		vertexList * vL = pL -> firstVertex;
		if (vL) {
			fprintf (fp, "* ");
			while (vL -> next) {
				fprintf (fp, "%i, %i%s", vL -> x, vL -> y, vL -> next -> next ? "; " : "\n");
				vL = vL -> next;
			}
		}
		pL = pL -> next;
	}
	fclose (fp);
	return TRUE;
}

BOOL loadFromFile (char * name) {
	BOOL adding = FALSE;
	int numGot = 0, gotX, firstX, firstY;

	FILE * fp = fopen (name, "rb");
	if (! fp) return FALSE;
	char c;
	
	noFloor ();
	delete firstPoly;
	firstPoly = NULL;

	while (! feof (fp)) {

		c = fgetc (fp);
		if (feof (fp)) c = '\n';

		switch (c) {
			case '*':
			adding = TRUE;
			firstX = -1;
			addPoly ();
			numGot = 0;
			break;
			
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			if (adding) numGot = numGot * 10 + (c - '0');
			break;
			
			case ',':
			if (adding) {
				gotX = numGot;
				numGot = 0;
			}
			break;
			
			case '\n':
			case ';':
			if (adding) {
				addVertex (gotX, numGot);
				if (firstX == -1) {
					firstX = gotX;
					firstY = numGot;
				}
				if (c == '\n') {
					addVertex (firstX, firstY);
					adding = FALSE;
				}
				numGot = 0;
			}
			break;
			
			default:
			break;
		}
	}
	fclose (fp);
	return TRUE;
}

void splitPoly (int x1, int y1, int x2, int y2) {

	if (x1 == x2 && y1 == y2) return;

	vertexList * gotCorner1, * gotCorner2, * vTemp;
	BOOL swap;

	polyList * pL = firstPoly;
	while (pL) {
		gotCorner1 = NULL;
		gotCorner2 = NULL;
		swap = FALSE;
		vertexList * vL = pL -> firstVertex;
		if (vL) {
			while (vL -> next) {
				if (vL -> x == x1 && vL -> y == y1) gotCorner1 = vL;
				if (vL -> x == x2 && vL -> y == y2) {
					gotCorner2 = vL;
					if (gotCorner1 == NULL) swap = TRUE;
				}
				vL = vL -> next;
			}
			if (gotCorner1 && gotCorner2) {
				if (swap) {
					vTemp = gotCorner2;
					gotCorner2 = gotCorner1;
					gotCorner1 = vTemp;
				}

				polyList * p2 = addPoly ();

				vTemp = gotCorner1 -> next;
				gotCorner1 -> next = gotCorner2;
				p2 -> firstVertex = vTemp;
				while (vTemp -> next != gotCorner2) {
					vTemp = vTemp -> next;
				}
				vTemp -> next = NULL;
			
				addVertex (gotCorner1 -> x, gotCorner1 -> y);
				addVertex (gotCorner2 -> x, gotCorner2 -> y);
				addVertex (vTemp -> x, vTemp -> y);
			}
		}
		pL = pL -> next;
	}
}