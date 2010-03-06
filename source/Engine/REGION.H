struct screenRegion {
	int x1, y1, x2, y2, sX, sY, di;
	objectType * thisType;
	screenRegion * next;
};

bool addScreenRegion (int x1, int y1, int x2, int y2, int, int, int, int objectNum);
void getOverRegion ();
screenRegion * getRegionForObject (int obj);
void removeScreenRegion (int objectNum);
void killAllRegions ();
void loadRegions (FILE *);
void saveRegions (FILE *);
void showBoxes ();

