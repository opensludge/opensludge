#ifndef HWND
#define HWND int
#endif

#define SPRITE_AREA_X	193
#define SPRITE_AREA_Y	100
#define SPRITE_AREA_W	264
#define SPRITE_AREA_H	260

struct sprite {
	int width, height, xhot, yhot;
	unsigned char * data;
};

struct spritePalette {
	unsigned short int * pal;
	unsigned char total;
};

struct spriteBank {
	int total;
	sprite * sprites;
	spritePalette myPalette;
	int myPaletteNum, myPaletteStart;
};

void forgetSpriteBank ();
bool loadSpriteBank (char * filename);
bool saveSpriteBank (char * filename);
void paintPal (HWND h);
bool initSpriteArea ();
void paintSpriteArea ();
void sortColours (HWND h);
void optimizePalette ();
void insertSprite (int);
void deleteSprite (int);
bool loadSpriteFromTGA (char * file, int);
bool saveSpriteToTGA (char * file, int);
bool trimSprite (int);
void spriteFit (int);
void initSpriteBank ();

void pasteSprite (int num);
