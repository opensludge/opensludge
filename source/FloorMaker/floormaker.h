/*
 *  FloorMaker.h
 *  Sludge Dev Kit
 *
 *  Created by Rikard Peterson on 2009-07-27.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif	
	

struct vertexList {
	int x, y;
	struct vertexList * next;
};

struct polyList {
	struct vertexList * firstVertex;
	struct polyList * next;
};

bool loadFloorFromFile (char * name, struct polyList **firstPoly);
bool saveFloorToFile (char * filename, struct polyList **firstPoly);

void noFloor (struct polyList **firstPoly);
int addVertex (int x, int y, struct polyList *firstPoly);
bool moveVertices (int x1, int y1, int x2, int y2, struct polyList *firstPoly);
void killVertex (int x, int y, struct polyList **firstPoly);

struct polyList * addPoly (struct polyList *firstPoly);

void splitPoly (int x1, int y1, int x2, int y2, struct polyList **firstPoly);
void splitLine (int x1, int y1, int x2, int y2, struct polyList *firstPoly);

void drawFloor (struct polyList * pL, float r, float g, float b);

bool snapToClosest (int *x, int *y, struct polyList * firstPoly);

#ifdef __cplusplus
}
#endif	
	
