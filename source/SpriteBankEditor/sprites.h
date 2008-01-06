#define SPRITE_AREA_X	193
#define SPRITE_AREA_Y	100
#define SPRITE_AREA_W	264
#define SPRITE_AREA_H	260

struct sprite {
	int width, height, xhot, yhot;
	byte * data;
};

struct spritePalette {
	unsigned short int * pal;
	byte total;
};

struct spriteBank {
	int total;
	sprite * sprites;
	spritePalette myPalette;
	int myPaletteNum, myPaletteStart;
};

void forgetSpriteBank ();
BOOL loadSpriteBank (char * filename);
BOOL saveSpriteBank (char * filename);
void paintPal (HWND h);
BOOL initSpriteArea ();
void paintSpriteArea ();
void sortColours (HWND h);
void optimizePalette ();
void insertSprite (int);
void deleteSprite (int);
BOOL loadSpriteFromTGA (char * file, int);
BOOL saveSpriteToTGA (char * file, int);
BOOL trimSprite (int);
void spriteFit (int);
void initSpriteBank ();

void pasteSprite (int num);
