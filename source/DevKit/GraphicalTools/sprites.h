#ifndef __SPRITES_H__
#define __SPRITES_H__

//#include <OpenGL/gl.h>
#include "GLee.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WIN32
#define HWND int
#endif

#define SPRITE_AREA_X	193
#define SPRITE_AREA_Y	100
#define SPRITE_AREA_W	264
#define SPRITE_AREA_H	260

struct sprite {
	int width, height, xhot, yhot;
	int tex_x;
	int special;
	int texNum;
	unsigned char * data;
};

struct spritePalette {
	unsigned short int * pal;
	unsigned char * r;
	unsigned char * g;
	unsigned char * b;
	GLuint tex_names[65535];
	GLuint burnTex_names[65535];
	int tex_w[65535], tex_h[65535];
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

void doFontification (struct spriteBank *sprites, unsigned int fontifySpaceWidth);

bool convertSpriteBank8to32 (struct spriteBank *sprites);
bool exportToPNG (const char * file, struct spriteBank *sprites, int index);

void paintPal (HWND h);
void sortColours (HWND h);
void optimizePalette ();
bool saveSpriteToTGA (char * file, int);
bool trimSprite (int);

void pasteSprite (struct sprite * single, const struct spritePalette * fontPal, bool showBox);

#ifdef __cplusplus
}
#endif

#endif
