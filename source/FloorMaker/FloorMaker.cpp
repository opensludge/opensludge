/*
 *  FloorMaker.cpp
 *  Sludge Dev Kit
 *
 *  Created by Rikard Peterson on 2009-07-27.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include <OpenGL/gl.h>

#include "FloorMaker.h"

bool polyIsComplete (struct polyList *firstPoly) {
	if (firstPoly -> firstVertex == NULL) return false;
	if (firstPoly -> firstVertex -> next == NULL) return false;
	vertexList * newVertex = firstPoly -> firstVertex;
	while (newVertex -> next) newVertex = newVertex -> next;
	return firstPoly -> firstVertex -> x == newVertex -> x && firstPoly -> firstVertex -> y == newVertex -> y;
}

polyList * addPoly (struct polyList *firstPoly) {
	polyList * newPoly = new polyList;
	if (newPoly == NULL) return NULL;
	newPoly -> next = firstPoly;
	newPoly -> firstVertex = NULL;
	firstPoly = newPoly;
	return newPoly;
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
	
	// Wow! Now all we need to do is set the values and update the list!
	newVertex -> x = x;
	newVertex -> y = y;
	newVertex -> next = firstPoly -> firstVertex;
	firstPoly -> firstVertex = newVertex;
	
	// It worked, so...
	return 1;
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

void drawFloor (struct polyList * floor) {
	struct polyList * pL = floor;
	
	struct vertexList * vL;
	struct vertexList * drawnVertices = NULL;
	
	if (! floor) return;
	
	while (pL) {
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


bool saveFloorToFile (char * filename, struct polyList *firstPoly) {
	FILE * fp = fopen (filename, "wt");
	if (! fp) {
//		alert ("Can't open file for writing");
		return false;
	}
	polyList * pL =  firstPoly;
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

