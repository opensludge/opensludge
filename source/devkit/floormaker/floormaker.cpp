/*
 *  FloorMaker.cpp
 *  Sludge Dev Kit
 *
 *  Created by Rikard Peterson on 2009-07-27.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "floormaker.h"

bool polyIsComplete (struct polyList *firstPoly) {
	if (firstPoly -> firstVertex == NULL) return false;
	if (firstPoly -> firstVertex -> next == NULL) return false;
	vertexList * newVertex = firstPoly -> firstVertex;
	while (newVertex -> next) newVertex = newVertex -> next;
	return firstPoly -> firstVertex -> x == newVertex -> x && firstPoly -> firstVertex -> y == newVertex -> y;
}

int ccw (struct vertexList *p0, struct vertexList *p1, struct vertexList *p2) {
	int dx1, dx2, dy1, dy2;
	dx1 = p1->x - p0->x; dy1 = p1->y - p0->y;
	dx2 = p2->x - p0->x; dy2 = p2->y - p0->y;
	if (dx1*dy2 > dy1*dx2) return +1;
	if (dx1*dy2 < dy1*dx2) return -1;
	if ((dx1*dx2 < 0) || (dy1*dy2 < 0)) return -1;
	if ((dx1*dx1+dy1*dy1) < (dx2*dx2+dy2*dy2)) return +1;
	return 0;
}

bool intersect(struct vertexList *l1a, struct vertexList *l1b, struct vertexList *l2) {
	// Check the lines
	if (! l1a) return true;
	if (! l1b) return true;
	if (! l2) return true;
	if (! l2->next) return true;
	
	// Lines sharing a vertex doesn't count...
	if ((l1a->x == l2->x && l1a->y == l2->y) ||
		(l1b->x == l2->x && l1b->y == l2->y) ||
		(l1b->x == l2->next->x && l1b->y == l2->next->y) ||
		(l1a->x == l2->next->x && l1a->y == l2->next->y)) {
		return false;
	}
		
	// Check for intersection
	return ((ccw(l1a, l1b, l2) * ccw(l1a, l1b, l2->next)) <=0)
		&& ((ccw(l2, l2->next, l1a) * ccw(l2, l2->next, l1b)) <=0);
}

bool polyIsConvex (struct polyList *firstPoly) {
	if (firstPoly -> firstVertex == NULL) return false;
	if (firstPoly -> firstVertex -> next == NULL) return false;
	if (firstPoly -> firstVertex -> next -> next == NULL) return false;
	
	int dir = 0, dir1 = 0;
	
	vertexList * newVertex1 = firstPoly -> firstVertex;
	vertexList * newVertex2 = newVertex1 -> next;
	vertexList * newVertex3 = newVertex2 -> next;
	while (newVertex3) {
		dir1 = ccw(newVertex1, newVertex2, newVertex3);
		if (dir1) {
			if (dir && (dir != dir1)) {
				return false;
			}
			dir = dir1;
		}
		
		newVertex1 = newVertex2;
		newVertex2 = newVertex3;
		newVertex3 = newVertex3->next;
	}
	dir1 = ccw(newVertex1, newVertex2, newVertex3 = firstPoly->firstVertex->next);
	if (dir1) {
		if (dir && (dir != dir1)) {
			return false;
		}
		dir = dir1;
	}
	dir1 = ccw(newVertex2, newVertex3, newVertex3->next);
	if (dir1) {
		if (dir && (dir != dir1)) {
			return false;
		}
		dir = dir1;
	}
	return true;
}

int getPolyDirection (struct polyList *firstPoly) {
	if (firstPoly -> firstVertex == NULL) return false;
	if (firstPoly -> firstVertex -> next == NULL) return false;
	if (firstPoly -> firstVertex -> next -> next == NULL) return false;
	
	int dir = 0;
	
	vertexList * newVertex1 = firstPoly -> firstVertex;
	vertexList * newVertex2 = newVertex1 -> next;
	vertexList * newVertex3 = newVertex2 -> next;
	while (newVertex3) {
		dir += ccw(newVertex1, newVertex2, newVertex3);

		newVertex1 = newVertex2;
		newVertex2 = newVertex3;
		newVertex3 = newVertex3->next;
	}
	dir += ccw(newVertex1, newVertex2, newVertex3 = firstPoly->firstVertex->next);
	dir += ccw(newVertex2, newVertex3, newVertex3->next);
	if (dir > 0) return 1;
	if (dir < 0) return -1;
	return 0;
}

struct vertexList * findConcaveVertex (struct polyList *firstPoly, int dir) {
	if (firstPoly -> firstVertex == NULL) return NULL;
	if (firstPoly -> firstVertex -> next == NULL) return NULL;
	if (firstPoly -> firstVertex -> next -> next == NULL) return NULL;
	if (! dir) return NULL;
	
	int dir1 = 0;
	
	vertexList * newVertex1 = firstPoly -> firstVertex;
	vertexList * newVertex2 = newVertex1 -> next;
	vertexList * newVertex3 = newVertex2 -> next;
	vertexList * test;
	while (newVertex3) {
		dir1 = ccw(newVertex1, newVertex2, newVertex3);
		if (dir1) {
			if (dir != dir1) {
				test = (newVertex3->next) ? newVertex3->next : firstPoly -> firstVertex->next;
				dir1 = ccw(newVertex2, newVertex3, test);
				if (dir == dir1)
					return newVertex2;
			}
		}
		
		newVertex1 = newVertex2;
		newVertex2 = newVertex3;
		newVertex3 = newVertex3->next;
	}
	dir1 = ccw(newVertex1, newVertex2, newVertex3 = firstPoly->firstVertex->next);
	if (dir1) {
		if (dir != dir1) {
			test = (newVertex3->next) ? newVertex3->next : firstPoly -> firstVertex;
			dir1 = ccw(newVertex2, newVertex3, test);
			if (dir == dir1)
				return newVertex2;
		}
	}
	dir1 = ccw(newVertex2, newVertex3, newVertex3->next);
	if (dir1) {
		if (dir != dir1) {
			test = (newVertex3->next->next) ? newVertex3->next->next : firstPoly -> firstVertex;
			dir1 = ccw(newVertex3, newVertex3->next, test);
			if (dir == dir1)
				return newVertex3;
		}
	}
	return NULL;
}


polyList * addPoly (struct polyList *firstPoly) {
	polyList * newPoly = new polyList;
	if (newPoly == NULL) return NULL;
	newPoly -> next = firstPoly;
	newPoly -> firstVertex = NULL;
	firstPoly = newPoly;
	return newPoly;
}

#define CLOSENESS 6
bool snapToClosest (int *x1, int *y1, polyList * firstPoly) {
	int x = *x1;
	int y = *y1;
	polyList * pL =  firstPoly;
	if (x < 4) x = 0;
	if (y < 4) y = 0;
	while (pL) {
		vertexList * vL = pL -> firstVertex;
		while (vL) {
			if (abs (vL -> x - x) < CLOSENESS && abs (vL -> y - y) < CLOSENESS) {
				*x1 = vL -> x;
				*y1 = vL -> y;
				return true;
			}
			vL = vL -> next;
		}
		pL = pL -> next;
	}
	return false;
}

int addVertex (int x, int y, struct polyList *firstPoly) {
	
	// Let's return 2 if the floor's complete...
	if (polyIsComplete (firstPoly)) return 2;
	
	// Let's return 3 if the chosen point is already used here...
	vertexList * newVertex = firstPoly -> firstVertex;
	while (newVertex) {
		if (newVertex -> next && x == newVertex -> x && y == newVertex -> y) return 3;
		newVertex = newVertex -> next;
	}
	
	// Let's return 0 if we can't create a new vertexLest thingy...
	newVertex = new vertexList;
	if (newVertex == NULL) return 0;
	
	if (x<0) x =0 ;
	if (y<0) y =0;
	
	// Wow! Now all we need to do is set the values and update the list!
	newVertex -> x = x;
	newVertex -> y = y;
	newVertex -> next = firstPoly -> firstVertex;
	firstPoly -> firstVertex = newVertex;
	
	// It worked, so...
	return 1;
}

bool moveVertices (int x1, int y1, int x2, int y2, struct polyList *firstPoly) {
	polyList * pL;
	vertexList * vL;
	bool got1, got2;
	
	if (x2<0) x2 = 0;
	if (y2<0) y2 = 0;
	
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

void removePoly (polyList * killMe, struct polyList **firstPoly) {
	polyList * * hunt = firstPoly;
	while (* hunt) {
		if (* hunt == killMe) {
			polyList * killer = (* hunt);
			* hunt = (* hunt) -> next;
			delete killer;
			if (! (*firstPoly)) *firstPoly = addPoly (*firstPoly);
			break;
		} else {
			hunt = & ((* hunt) -> next);
		}
	}
}


void killVertex (int x, int y, struct polyList **firstPoly) {
	polyList * pL =  *firstPoly;
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
		if (killMe) removePoly (killMe, firstPoly);
	}
}

void splitLine (int x1, int y1, int x2, int y2, struct polyList *firstPoly) {
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




void splitPoly (int x1, int y1, int x2, int y2, struct polyList **firstPoly) {
	
	if (x1 == x2 && y1 == y2) return;
	
	vertexList * gotCorner1, * gotCorner2, * vTemp;
	bool swap;
	
	polyList * pL = *firstPoly;
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
				
				polyList * p2 = (*firstPoly) = addPoly (*firstPoly);
				
				vTemp = gotCorner1 -> next;
				gotCorner1 -> next = gotCorner2;
				p2 -> firstVertex = vTemp;
				while (vTemp -> next != gotCorner2) {
					vTemp = vTemp -> next;
				}
				vTemp -> next = NULL;
				
				addVertex (gotCorner1 -> x, gotCorner1 -> y, *firstPoly);
				addVertex (gotCorner2 -> x, gotCorner2 -> y, *firstPoly);
				addVertex (vTemp -> x, vTemp -> y, *firstPoly);
			}
		}
		pL = pL -> next;
	}
}




void noFloor (struct polyList **firstPoly) {
	while (*firstPoly) {
		while ((*firstPoly) -> firstVertex) {
			vertexList * killMe = (*firstPoly) -> firstVertex;
			(*firstPoly) -> firstVertex = killMe -> next;
			delete killMe;
		}
		polyList * killPoly = *firstPoly;
		*firstPoly = (*firstPoly) -> next;
		delete killPoly;
	}
	*firstPoly = addPoly (*firstPoly);
}


bool loadFloorFromFile (char * name, struct polyList **firstPoly) {
	bool adding = false;
	int numGot = 0, gotX, firstX, firstY;
	
	FILE * fp = fopen (name, "rb");
	if (! fp) return false;
	char c;
	
	noFloor (firstPoly);
	delete *firstPoly;
	*firstPoly = NULL;
	
	while (! feof (fp)) {
		
		c = fgetc (fp);
		if (feof (fp)) c = '\n';
		
		switch (c) {
			case '*':
				adding = true;
				firstX = -1;
				*firstPoly = addPoly (*firstPoly);
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
					addVertex (gotX, numGot, *firstPoly);
					if (firstX == -1) {
						firstX = gotX;
						firstY = numGot;
					}
					if (c == '\n') {
						addVertex (firstX, firstY, *firstPoly);
						adding = false;
					}
					numGot = 0;
				}
				break;
				
			default:
				break;
		}
	}
	fclose (fp);
	return true;
}

void drawFloor (struct polyList * floor, float r, float g, float b) {
	struct polyList * pL = floor;
	
	struct vertexList * vL;
	struct vertexList * drawnVertices = NULL;
	
	if (! floor) return;
	
	while (pL) {
		vL = pL -> firstVertex;
		if (polyIsConvex(pL)) {
			glColor4f(r, g, b, 0.25);	
			glBegin(GL_POLYGON);
			while (vL) {
				if (vL -> next) {
					glVertex3f(vL -> x, vL -> y, 0.0);
					glVertex3f(vL -> next -> x, vL -> next -> y, 0.0);
				}
				vL = vL -> next;
			}
			glEnd();
		} else {
			glColor4f(1.0, 0.0, 0.0, 0.25);	
			glBegin(GL_POLYGON);
			while (vL) {
				if (vL -> next) {
					glVertex3f(vL -> x, vL -> y, 0.0);
					glVertex3f(vL -> next -> x, vL -> next -> y, 0.0);
				}
				vL = vL -> next;
			}
			glEnd();
		}
			
		glColor4f(r, g, b, 1.0);	
		vL = pL -> firstVertex;
		while (vL) {
			
			// Draw the line from here to the next point
			if (vL -> next) {
				glBegin(GL_LINES);
				{
					glVertex3f(vL -> x, vL -> y, 0.0);
					glVertex3f(vL -> next -> x, vL -> next -> y, 0.0);
				}
				glEnd();
			}
			// Do we want to draw crosses at the corners?
			 
			if (1 /*markVertices*/) {
				vertexList * newV = drawnVertices;
				 
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
					glBegin(GL_LINES);
					{
						glVertex3f(vL -> x - 5, vL -> y - 5, 0.0);
						glVertex3f(vL -> x + 5, vL -> y + 5, 0.0);
						glVertex3f(vL -> x + 5, vL -> y - 5, 0.0);
						glVertex3f(vL -> x - 5, vL -> y + 5, 0.0);
					}
					glEnd();
				}
			}
			vL = vL -> next;
		}
		pL = pL -> next;
	}
	vL = floor -> firstVertex;
	if (! polyIsComplete (floor) && vL) {
		int xx = vL -> x;
		int yy = vL -> y;
		glBegin(GL_QUADS);
		{
			glVertex3f(xx-5, yy-5, 0.0);
			glVertex3f(xx+5, yy-5, 0.0);
			glVertex3f(xx+5, yy+5, 0.0);
			glVertex3f(xx-5, yy+5, 0.0);
		}
		glEnd();
		
		while (vL -> next) vL = vL -> next;
		xx = vL -> x;
		yy = vL -> y;
		glBegin(GL_QUADS);
		{
			glVertex3f(xx-7, yy, 0.0);
			glVertex3f(xx, yy-7, 0.0);
			glVertex3f(xx+7, yy, 0.0);
			glVertex3f(xx, yy+7, 0.0);
		}
		glEnd();
	}
	while (drawnVertices) {
		struct vertexList * k = drawnVertices;
		drawnVertices = drawnVertices -> next;
		delete k;
	}
}


bool saveFloorToFile (char * filename, struct polyList **firstPoly) {
	FILE * fp = fopen (filename, "wt");
	if (! fp) {
//		alert ("Can't open file for writing");
		return false;
	}

	int dir;
	bool fixFloor = false;
	polyList * pL =  *firstPoly;
	while (pL) {
		if (! polyIsConvex (pL)) {
			if (fixFloor || askAQuestion("Error in floor.", "I found a polygon in the floor that isn't convex. That will most likely cause problems. Do you want me to try and fix it for you?")) {
				fixFloor = true;
				// Fix it!
				if (! (dir = getPolyDirection(pL))) {
					errorBox ("Error", "I can't fix the error. The floor is too complicated, but I'll save it anyway.");
					pL = NULL;
					break;
				}
				
				vertexList * vL1 = findConcaveVertex (pL, dir);
				if (vL1) {
					vertexList * vL2;
					if (vL1->next) vL2 = vL1->next; else vL2 = pL->firstVertex->next;
					if (vL2->next) vL2 = vL2->next; else vL2 = pL->firstVertex->next;
					
					// Check that the line doesn't cross any other lines
					vertexList * vL = pL -> firstVertex;
					bool doIt = true;
					while (vL -> next) {
						if (intersect (vL1, vL2, vL)) {
							vL2 = (vL2->next) ? vL2->next : pL -> firstVertex;
							if (vL2 == vL1) {
								doIt = false;
								break;
							} else {
								vL = pL -> firstVertex;
							}
						} else {
							vL = vL -> next;
						}
					}
						
					if (doIt) {
						splitPoly (vL1->x, vL1->y, vL2->x, vL2->y, firstPoly);
						pL = *firstPoly;
						continue;
					}
				}
			} else {
				break;
			}
		}
		if (pL) pL = pL -> next;
	}
	
	pL =  *firstPoly;
	while (pL) {
		vertexList * vL = pL -> firstVertex;
		if (vL) {
			fprintf (fp, "* ");
			while (vL -> next) {
				if (vL->x < 0) vL->x = 0;
				if (vL->y < 0) vL->y = 0;
				fprintf (fp, "%i, %i%s", vL -> x, vL -> y, vL -> next -> next ? "; " : "\n");
				vL = vL -> next;
			}
		}
		pL = pL -> next;
	}
	fclose (fp);
	return true;
}

