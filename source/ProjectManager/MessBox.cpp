/*
 *  MessBox.cpp
 *  Sludge Dev Kit
 *
 *  Created by Rikard Peterson on 2009-07-15.
 *  Copyright 2009 Hungry Software and contributors. All rights reserved.
 *
 */

#include <stdio.h>

#include "MessBox.h"

bool errorBox (const char * tx1, const char * tx2){
	fprintf (stderr, "%s %s\n", tx1, tx2);
	return false;
}
bool errorBox (const char * tx1, unsigned int number){
	fprintf (stderr, "%s %d\n", tx1, number);
	return false;
}
bool errorBox (int errorType, const char *tx1, const char *tx2, const char * filename){
	fprintf (stderr, "%d %s %s %s\n", errorType, tx1, tx2, filename);
	return false;
}
bool errorBox (const char * tx1, unsigned int number, unsigned int n2){
	fprintf (stderr, "%s %d %d\n", tx1, number, n2);
	return false;
}