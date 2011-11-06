#include "allfiles.h"
#include "sprites.h"
#include "sprbanks.h"
#include "newfatal.h"
#include "debug.h"

loadedSpriteBank * allLoadedBanks = NULL;
extern spriteBank theFont;
extern int loadedFontNum, fontTableSize;


loadedSpriteBank * loadBankForAnim (int ID) {
	//debugOut ("loadBankForAnim: Looking for sprite bank with ID %d\n", ID);
	loadedSpriteBank * returnMe = allLoadedBanks;
	while (returnMe) {
		if (returnMe -> ID == ID) {
			//debugOut ("loadBankForAnim: Found existing sprite bank with ID %d\n", returnMe -> ID);
			return returnMe;
		}
		returnMe = returnMe -> next;
	}
	returnMe = new loadedSpriteBank;
	//debugOut ("loadBankForAnim: No existing sprite bank with ID %d\n", ID);
	if (checkNew (returnMe)) {
		returnMe -> ID = ID;
		if (loadSpriteBank (ID, returnMe -> bank, false)) {
			returnMe -> timesUsed = 0;
			returnMe -> next = allLoadedBanks;
			allLoadedBanks = returnMe;
			debugOut ("loadBankForAnim: New sprite bank created OK\n");
			return returnMe;
		} else {
			debugOut ("loadBankForAnim: I guess I couldn't load the sprites...\n");
			return NULL;
		}
	} else return NULL;
}

void reloadSpriteTextures () {
	loadedSpriteBank * spriteBank = allLoadedBanks;
	while (spriteBank) {
		//fprintf (stderr, "Reloading bank %d: %s.\n", spriteBank->ID, resourceNameFromNum (spriteBank->ID));
		delete spriteBank-> bank.sprites;
		spriteBank-> bank.sprites = NULL;
		loadSpriteBank (spriteBank->ID, spriteBank->bank, false);
		spriteBank = spriteBank -> next;
	}
	if (fontTableSize) {
		delete theFont.sprites;
		theFont.sprites = NULL;
		loadSpriteBank (loadedFontNum, theFont, true);
	}
}
