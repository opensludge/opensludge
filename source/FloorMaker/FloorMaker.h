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

void noFloor (struct polyList **firstPoly);
struct polyList * addPoly (struct polyList *firstPoly);

void drawFloor (struct polyList * pL);


#ifdef __cplusplus
}
#endif	
	