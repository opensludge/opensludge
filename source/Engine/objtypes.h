struct combination {
	int withObj, funcNum;
};

struct objectType {
	char * screenName;
	int objectNum;
	objectType * next;
	unsigned char r, g, b;
	int numCom;
	int speechGap, walkSpeed, wrapSpeech, spinSpeed;
	unsigned short int flags;
	combination * allCombis;
};

bool initObjectTypes ();
objectType * findObjectType (int i);
objectType * loadObjectType (int i);
int getCombinationFunction (int a, int b);
void removeObjectType (objectType * oT);
void saveObjectRef (objectType * r, FILE * fp);
objectType * loadObjectRef (FILE * fp);
