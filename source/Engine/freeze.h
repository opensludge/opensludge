struct frozenStuffStruct {
	onScreenPerson * allPeople;
	screenRegion * allScreenRegions;
	GLubyte * backdropTexture;
	GLuint backdropTextureName;
	GLuint lightMapTextureName;
	GLubyte * lightMapTexture;
	GLubyte * zBufferImage;
	int zPanels;
	parallaxLayer * parallaxStuff;
	int lightMapNumber, zBufferNumber;
	speechStruct * speech;
	statusStuff * frozenStatus;
	eventHandlers * currentEvents;
	personaAnimation * mouseCursorAnim;
	int mouseCursorFrameNum;
	int cameraX, cameraY, sceneWidth, sceneHeight;
	float cameraZoom;

	frozenStuffStruct * next;
};

bool freeze ();
void unfreeze (bool killImage = true);
int howFrozen ();
