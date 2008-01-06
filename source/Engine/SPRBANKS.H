struct loadedSpriteBank {
	int ID, timesUsed;
	spriteBank bank;
	loadedSpriteBank * next;
};

loadedSpriteBank * loadBankForAnim (int ID);
