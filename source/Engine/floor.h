struct floorPolygon {
	int numVertices;
	int * vertexID;
};

struct flor {
	int originalNum;
	POINT * vertex;
	int numPolygons;
	floorPolygon * polygon;
	int * * matrix;
};

BOOL initFloor ();
void setFloorNull ();
BOOL setFloor (int fileNum);
void drawFloor ();
int inFloor (int x, int y);
BOOL getMatchingCorners (floorPolygon &, floorPolygon &, int &, int &);
BOOL closestPointOnLine (int & closestX, int & closestY, int x1, int y1, int x2, int y2, int xP, int yP);