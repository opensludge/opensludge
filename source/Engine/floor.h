#ifdef _WIN32
#include "windef.h"
#endif

struct floorPolygon {
	int numVertices;
	int * vertexID;
};

#ifndef _WIN32
struct POINT {
	int x; int y;
};
#endif

struct flor {
	int originalNum;
	POINT * vertex;
	int numPolygons;
	floorPolygon * polygon;
	int * * matrix;
};

bool initFloor ();
void setFloorNull ();
bool setFloor (int fileNum);
void drawFloor ();
int inFloor (int x, int y);
bool getMatchingCorners (floorPolygon &, floorPolygon &, int &, int &);
bool closestPointOnLine (int & closestX, int & closestY, int x1, int y1, int x2, int y2, int xP, int yP);
