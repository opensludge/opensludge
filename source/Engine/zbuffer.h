struct zBufferData {
	unsigned short * * map;
//	int width, height;
//	BOOL loaded;
	int originalNum;
};

BOOL setZBuffer (int y);
void noZBuffer ();