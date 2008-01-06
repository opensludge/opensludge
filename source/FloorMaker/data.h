struct vertexList {
	int x, y;
	vertexList * next;
};

struct polyList {
	vertexList * firstVertex;
	polyList * next;
};

void drawSoFar (unsigned short adder);
int addVertex (int x, int y);
polyList * addPoly ();
BOOL snapToClosest (int & x, int & y);
BOOL loadFromFile (char * name);
BOOL polyIsComplete ();
void splitPoly (int x1, int y1, int x2, int y2);
void splitLine (int x1, int y1, int x2, int y2);
BOOL moveVertices (int x1, int y1, int x2, int y2);
BOOL saveToFile (char * filename);
void noFloor ();
void killVertex (int x, int y);