#ifndef __SPRITES_AA__
#define __SPRITES_AA__

struct sprite;
struct spritePalette;
struct loadedFunction;

struct aaSettingsStruct
{
	bool 	useMe;
	float 	blurX;
	float 	blurY;
};

extern aaSettingsStruct maxAntiAliasSettings;

BOOL scaleSprite_AA (int x, int y, sprite & single, const spritePalette & fontPal, float scale, unsigned int drawMode, int floaty, BOOL useZB, unsigned short int * * lightMapImage, bool mirror, bool boundingBoxCollision, aaSettingsStruct * aa);
void fixScaleSprite_AA (int x1, int y1, sprite & single, const spritePalette & fontPal, float scale, unsigned int drawMode, int floaty, BOOL useZB, unsigned short int * * lightMapImage, const int camX, const int camY, bool mirror, aaSettingsStruct * aa);

void aaSave (aaSettingsStruct & aa, FILE * fp);
void aaLoad (aaSettingsStruct & aa, FILE * fp);
void aaCopy (aaSettingsStruct * toHere, const aaSettingsStruct * fromHere);
bool aaGetFromStack (aaSettingsStruct * toHere, loadedFunction * fun);

#endif