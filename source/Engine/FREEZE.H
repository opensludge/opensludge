struct frozenStuffStruct {
	onScreenPerson * allPeople;
	screenRegion * allScreenRegions;
	unsigned short int * * backDropImage;
	unsigned short int * * lightMapImage;
	unsigned short int * * zBufferImage;
	parallaxLayer * parallaxStuff;
	int lightMapNumber, zBufferNumber;
	speechStruct * speech;
	statusStuff * frozenStatus;
	eventHandlers * currentEvents;
	personaAnimation * mouseCursorAnim;
	int mouseCursorFrameNum;
	int cameraX, cameraY, sceneWidth, sceneHeight;

	frozenStuffStruct * next;
};

BOOL freeze ();
void unfreeze (BOOL killImage = TRUE);
int howFrozen ();