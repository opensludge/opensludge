#include "allfiles.h"
#include "graphics.h"
#include "people.h"
#include "sprites.h"
#include "sprbanks.h"
#include "newfatal.h"
#include "debug.h"

loadedSpriteBank * allLoadedBanks = NULL;
extern spriteBank theFont;
extern int loadedFontNum, fontTableSize;
extern onScreenPerson * allPeople;


void forgetSpriteBank (spriteBank & forgetme) {
    
	deleteTextures (forgetme.myPalette.numTextures, forgetme.myPalette.tex_names);
	if (forgetme.isFont) {
		deleteTextures (forgetme.myPalette.numTextures, forgetme.myPalette.burnTex_names);
		delete [] forgetme.myPalette.burnTex_names;
		forgetme.myPalette.burnTex_names = NULL;
	}
	
	delete [] forgetme.myPalette.tex_names;
	forgetme.myPalette.tex_names = NULL;
	delete [] forgetme.myPalette.tex_w;
	forgetme.myPalette.tex_w = NULL;
	delete [] forgetme.myPalette.tex_h;
	forgetme.myPalette.tex_h = NULL;
	
	if (forgetme.myPalette.pal) {
		delete  [] forgetme.myPalette.pal;
		forgetme.myPalette.pal = NULL;
		delete  [] forgetme.myPalette.r;
		forgetme.myPalette.r = NULL;
		delete  [] forgetme.myPalette.g;
		forgetme.myPalette.g = NULL;
		delete  [] forgetme.myPalette.b;
		forgetme.myPalette.b = NULL;
	}
    
	delete forgetme.sprites;
	forgetme.sprites = NULL;
        
}

int freeSpriteBank (int ID) {
    
    loadedSpriteBank * thisBank = allLoadedBanks;
    loadedSpriteBank * bankToKill = NULL;
    
    while (thisBank) {
        if (thisBank -> next) {
            if (thisBank -> next -> ID == ID) {
                // fprintf (stderr, "freeSpriteBank: Found existing sprite bank with ID %d: %s\n",thisBank ->next -> ID, resourceNameFromNum(thisBank ->next-> ID));
                bankToKill = thisBank -> next;
                
                // First check that the bank isn't in use
                onScreenPerson * thisPerson = allPeople;
                while (thisPerson) {
                    if (thisBank == thisPerson->myAnim->theSprites) return 0;
                    thisPerson = thisPerson -> next;
                }

                
                forgetSpriteBank(bankToKill->bank);
                
                thisBank->next = bankToKill->next;
                delete bankToKill;
                return 1;
            }
        }
        thisBank = thisBank -> next;
    }
    
    return 0;
}


loadedSpriteBank * loadBankForAnim (int ID) {
	//fprintf (stderr, "loadBankForAnim: Looking for sprite bank with ID %d: %s\n", ID, resourceNameFromNum(ID));
	loadedSpriteBank * returnMe = allLoadedBanks;
	while (returnMe) {
		if (returnMe -> ID == ID) {
			debugOut ("loadBankForAnim: Found existing sprite bank with ID %d\n", returnMe -> ID);
			return returnMe;
		}
		returnMe = returnMe -> next;
	}
	returnMe = new loadedSpriteBank;
	//fprintf (stderr, "loadBankForAnim: No existing sprite bank with ID %d\n", ID);
	if (checkNew (returnMe)) {
		returnMe -> ID = ID;
		if (loadSpriteBank (ID, returnMe -> bank, false)) {
			returnMe -> timesUsed = 0;
			returnMe -> next = allLoadedBanks;
			allLoadedBanks = returnMe;
	//		fprintf (stderr, "loadBankForAnim: New sprite bank created OK\n");
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
