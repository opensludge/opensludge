#include <stdlib.h>
#include "ALLFILES.H"

void * allKnownMem[3000];
int allKnownNum = 0;

//void db (char *);

void outputKnownMem () {
	FILE * debu = fopen ("debuTURN.txt", "at");

	fprintf (debu, "%i lumps:", allKnownMem);
	for (int i = 0; i < allKnownNum; i ++) {
		fprintf (debu, " %p", allKnownMem[i]);
	}
	fprintf (debu, "\n");
	fclose (debu);
}

void adding (void * mem) {
	allKnownMem[allKnownNum] = mem;
	allKnownNum ++;

	outputKnownMem ();
	if (allKnownNum == 3000) {
		//db ("Error! Array too full!");
		exit (1);
	}
}

void deleting (void * mem) {
	allKnownNum --;
	for (int i = 0; i <= allKnownNum; i ++) {
		if (allKnownMem[i] == mem) {
			allKnownMem[i] = allKnownMem[allKnownNum];
			outputKnownMem ();
			return;
		}
	}
	//db ("Error! Deleted a block which hasn't been allocated!");
	exit (1);
}