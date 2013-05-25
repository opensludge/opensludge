#ifndef _SPRBANKS_H_
#define _SPRBANKS_H_

#include "sprites.h"

struct loadedSpriteBank {
	int ID, timesUsed;
	spriteBank bank;
	loadedSpriteBank * next;
};

extern loadedSpriteBank * allLoadedBanks;

void forgetSpriteBank (spriteBank & forgetme);
int freeSpriteBank (int ID);

loadedSpriteBank * loadBankForAnim (int ID);
void reloadSpriteTextures ();

#endif