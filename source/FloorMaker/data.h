

void drawSoFar (unsigned short adder);
int addVertex (int x, int y);
bool snapToClosest (int & x, int & y);
bool polyIsComplete ();
void splitPoly (int x1, int y1, int x2, int y2);
void splitLine (int x1, int y1, int x2, int y2);
bool moveVertices (int x1, int y1, int x2, int y2);
bool saveToFile (char * filename);
void killVertex (int x, int y);

