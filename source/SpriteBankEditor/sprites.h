#ifdef __cplusplus
extern "C" {
#endif	

#ifndef HWND
#define HWND int
#endif

#define SPRITE_AREA_X	193
#define SPRITE_AREA_Y	100
#define SPRITE_AREA_W	264
#define SPRITE_AREA_H	260

struct sprite {
	int width, height, xhot, yhot;
	int tex_x;
	int texNum;
	unsigned char * data;
};

struct spritePalette {
	unsigned short int * pal;
	unsigned char * r;
	unsigned char * g;
	unsigned char * b;
	GLuint tex_names[256];
	GLuint burnTex_names[256];
	int tex_w[256], tex_h[256];
	int numTextures;
	unsigned char originalRed, originalGreen, originalBlue, total;
};

struct spriteBank {
	int total;
	int type;
	struct sprite * sprites;
	struct spritePalette myPalette;
	bool isFont;
};

bool reserveSpritePal (struct spritePalette * sP, int n);
void forgetSpriteBank (struct spriteBank * forgetme);
bool loadSpriteBank (const char * filename, struct spriteBank * loadhere);
bool loadSpriteTextures (struct spriteBank *loadhere);
bool saveSpriteBank (const char * filename, struct spriteBank *sprites);

void addSprite (int i, struct spriteBank *sprites);
void deleteSprite (int i, struct spriteBank *sprites);

bool loadSpriteFromPNG (const char * file, struct spriteBank *sprites, int index);
bool loadSpriteFromTGA (const char * file, struct spriteBank *sprites, int index);

bool convertSpriteBank8to32 (struct spriteBank *sprites);
bool exportToPNG (const char * file, struct spriteBank *sprites, int index);

void paintPal (HWND h);
void sortColours (HWND h);
void optimizePalette ();
void insertSprite (int);
bool saveSpriteToTGA (char * file, int);
bool trimSprite (int);

void pasteSprite (struct sprite * single, const struct spritePalette * fontPal, bool showBox);

#ifdef __cplusplus
}
#endif
