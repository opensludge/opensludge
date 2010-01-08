#ifndef SPRITE_H
#define SPRITE_H

//#include <SDL_opengl.h>
#include "GLee.h"

struct sprite {
	int width, height, xhot, yhot;
	int tex_x;
	int texNum;
	//unsigned char * data;
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
	sprite * sprites;
	spritePalette myPalette;
	bool isFont;
};

struct aaSettingsStruct;

void forgetSpriteBank (spriteBank & forgetme);
bool loadSpriteBank (char * filename, spriteBank & loadhere);
bool loadSpriteBank (int fileNum, spriteBank & loadhere, bool isFont);

void fontSprite		(int x1, int y1, sprite & single, const spritePalette & fontPal);
void flipFontSprite	(int x1, int y1, sprite & single, const spritePalette & fontPal);

bool scaleSprite (int x1, int y1, sprite & single, const spritePalette & fontPal, float, unsigned int, int, bool, bool, bool, bool, aaSettingsStruct *);
void pasteSpriteToBackDrop (int x1, int y1, sprite & single, const spritePalette & fontPal);
bool reserveSpritePal (spritePalette & sP, int n);
void fixScaleSprite (int x1, int y1, sprite & single, const spritePalette & fontPal, float scale, unsigned int drawMode, int, bool, bool light, const int camX, const int camY, bool, aaSettingsStruct * aa);
void burnSpriteToBackDrop (int x1, int y1, sprite & single, const spritePalette & fontPal);

#endif
