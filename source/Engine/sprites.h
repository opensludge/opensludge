#ifndef SPRITE_H
#define SPRITE_H

#if !defined(HAVE_GLES2)
#include "GLee.h"
#else
#include <GLES2/gl2.h>
#endif

struct onScreenPerson;

struct sprite {
	int width, height, xhot, yhot;
	int tex_x;
	int texNum;
	//unsigned char * data;
};

class spritePalette {
public:
	unsigned short int * pal;
	unsigned char * r;
	unsigned char * g;
	unsigned char * b;
	GLuint * tex_names;
	GLuint * burnTex_names;
	int * tex_w, * tex_h;
	int numTextures;
	unsigned char originalRed, originalGreen, originalBlue, total;
	
	spritePalette(): pal(0), r(0), g(0), b(0), tex_names(0), burnTex_names(0)
	, tex_w(0), tex_h(0), numTextures(0)
	, total(0) {}
	
	~spritePalette() {
		delete [] pal;
		delete [] r;
		delete [] g;
		delete [] b;
		delete [] tex_names;
		delete [] burnTex_names;
		delete [] tex_w;
		delete [] tex_h;
	}
};

struct spriteBank {
	int total;
	int type;
	sprite * sprites;
	spritePalette myPalette;
	bool isFont;
};

bool loadSpriteBank (char * filename, spriteBank & loadhere);
bool loadSpriteBank (int fileNum, spriteBank & loadhere, bool isFont);

void fontSprite		(double x1, double y1, sprite & single, const spritePalette & fontPal);
void flipFontSprite	(double x1, double y1, sprite & single, const spritePalette & fontPal);

void pasteSpriteToBackDrop (int x1, int y1, sprite & single, const spritePalette & fontPal);
void burnSpriteToBackDrop (int x1, int y1, sprite & single, const spritePalette & fontPal);

bool reserveSpritePal (spritePalette & sP, int n);

bool scaleSprite (sprite & single, const spritePalette & fontPal, onScreenPerson * thisPerson, bool mirror, bool paste);

#endif
