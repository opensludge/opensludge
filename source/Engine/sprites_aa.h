#ifndef __SPRITES_AA__
#define __SPRITES_AA__

#include <stdio.h>

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

void aaSave (aaSettingsStruct & aa, FILE * fp);
void aaLoad (aaSettingsStruct & aa, FILE * fp);
void aaCopy (aaSettingsStruct * toHere, const aaSettingsStruct * fromHere);
bool aaGetFromStack (aaSettingsStruct * toHere, loadedFunction * fun);

#endif
