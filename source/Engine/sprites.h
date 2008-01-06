struct sprite {
	int width, height, xhot, yhot;
	byte * data;
};

struct spritePalette {
	unsigned short int * pal;
	byte originalRed, originalGreen, originalBlue, total;
};

struct spriteBank {
	int total;
	sprite * sprites;
	spritePalette myPalette;
};

struct aaSettingsStruct;

void forgetSpriteBank (spriteBank & forgetme);
BOOL loadSpriteBank (char * filename, spriteBank & loadhere);
BOOL loadSpriteBank (int fileNum, spriteBank & loadhere);

void fontSprite		(int x1, int y1, sprite & single, const spritePalette & fontPal);
void flipFontSprite	(int x1, int y1, sprite & single, const spritePalette & fontPal);

BOOL scaleSprite (int x1, int y1, sprite & single, const spritePalette & fontPal, float, unsigned int, int, BOOL, unsigned short int * *, bool, bool, aaSettingsStruct *);
void pasteSpriteToBackDrop (int x1, int y1, sprite & single, const spritePalette & fontPal);
BOOL reserveSpritePal (spritePalette & sP, int n);
void fixScaleSprite (int x1, int y1, sprite & single, const spritePalette & fontPal, float scale, unsigned int drawMode, int, BOOL, unsigned short int * *, const int camX, const int camY, bool, aaSettingsStruct * aa);
void burnSpriteToBackDrop (int x1, int y1, sprite & single, const spritePalette & fontPal);
bool getScaleData (int * & scaleDataX, int * & scaleDataY, int diffX, int diffY, float scale, bool mirror);
