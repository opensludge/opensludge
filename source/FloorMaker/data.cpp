#include <stdlib.h>
#include <stdio.h>
#include "data.h"

#define CLOSENESS 8

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

bool snapToClosest (int & x, int & y) {
	polyList * pL =  firstPoly;
	if (x < 4) x = 0;
	if (y < 4) y = 0;
	while (pL) {
		vertexList * vL = pL -> firstVertex;
		while (vL) {
			if (abs (vL -> x - x) < CLOSENESS && abs (vL -> y - y) < CLOSENESS) {
				x = vL -> x;
				y = vL -> y;
				return true;
			}
			vL = vL -> next;
		}
		pL = pL -> next;
	}
	return false;
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
		bool killedAlready = false;
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
					killedAlready = true;
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

bool moveVertices (int x1, int y1, int x2, int y2) {
	polyList * pL;
	vertexList * vL;
	bool got1, got2;

	// Check we're not doubling up...

	pL = firstPoly;
	while (pL) {
		got1 = false;
		got2 = false;
		vL = pL -> firstVertex;
		while (vL) {
			if (vL -> x == x1 && vL -> y == y1) got1 = true;
			if (vL -> x == x2 && vL -> y == y2) got2 = true;
			vL = vL -> next;
		}
		if (got1 && got2) return false;
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
	return true;
}

void splitPoly (int x1, int y1, int x2, int y2) {

	if (x1 == x2 && y1 == y2) return;

	vertexList * gotCorner1, * gotCorner2, * vTemp;
	bool swap;

	polyList * pL = firstPoly;
	while (pL) {
		gotCorner1 = NULL;
		gotCorner2 = NULL;
		swap = false;
		vertexList * vL = pL -> firstVertex;
		if (vL) {
			while (vL -> next) {
				if (vL -> x == x1 && vL -> y == y1) gotCorner1 = vL;
				if (vL -> x == x2 && vL -> y == y2) {
					gotCorner2 = vL;
					if (gotCorner1 == NULL) swap = true;
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

