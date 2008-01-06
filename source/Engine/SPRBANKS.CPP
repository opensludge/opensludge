#include "allfiles.h"
#include "sprites.h"
#include "sprbanks.h"
#include "newfatal.h"
//#include "debug.h"

loadedSpriteBank * allLoadedBanks = NULL;

loadedSpriteBank * loadBankForAnim (int ID) {
//	debug2 ("Looking for sprite bank with ID", ID);
	loadedSpriteBank * returnMe = allLoadedBanks;
	while (returnMe) {
//		debug2 ("Found existing sprite bank with ID", returnMe -> ID);
		if (returnMe -> ID == ID) return returnMe;
		returnMe = returnMe -> next;
	}
	returnMe = new loadedSpriteBank;
//	debug2 ("No existing sprite bank with ID", ID);
	if (checkNew (returnMe)) {
		returnMe -> ID = ID;
		if (loadSpriteBank (ID, returnMe -> bank)) {
			returnMe -> timesUsed = 0;
			returnMe -> next = allLoadedBanks;
			allLoadedBanks = returnMe;
//			debug ("New sprite bank created OK");
			return returnMe;
		} else {
//			debug ("I guess I couldn't load the sprites...");
			return NULL;
		}
	} else return NULL;
}