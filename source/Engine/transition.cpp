#include "allfiles.h"
#include "colours.h"
#include "backdrop.h"

extern unsigned short int * screen;
extern unsigned short int * * snapshotImage;
extern unsigned char brightnessLevel;

unsigned char fadeMode = 2;

void blackBuffer () {
	unsigned short int a, b;
	unsigned short int * toScreen = screen;

	for (a = 0; a < winHeight; a ++) {
		for (b = 0; b < winWidth; b ++) {
			* (toScreen ++) = 0;
		}
	}
}

//----------------------------------------------------
// PROPER BRIGHTNESS FADING
//----------------------------------------------------

unsigned lastFrom, lastTo;

void transitionFader () {
	if (! brightnessLevel) {
		blackBuffer ();
		return;
	}
	
	unsigned short int * toScreen = screen;
	unsigned short int * end = screen + (winWidth * winHeight);
	
	lastTo = 0;
	lastFrom = 0;
	
	do {
		if (* toScreen == lastFrom) {
			* toScreen = lastTo;
		} else {
			lastFrom = * toScreen;
			lastTo = * toScreen = makeColour ((redValue   (* toScreen) * brightnessLevel) >> 8,
											  (greenValue (* toScreen) * brightnessLevel) >> 8,
											  (blueValue  (* toScreen) * brightnessLevel) >> 8);
		}
	} while (++ toScreen != end);
}

void transitionCrossFader () {
//	if (! brightnessLevel) {
//		blackBuffer ();
//		return;
//	}

	if (! snapshotImage) return; //transitionFader ();
	
	unsigned char brightnessLevel2 = 255 - brightnessLevel;
	unsigned short int * toScreen = screen;
	unsigned short int * end = screen + (winWidth * winHeight);

	for (int y = 0; y < winHeight; y ++) {
		unsigned short int * snapPointer = snapshotImage[y];
		for (int x = 0; x < winWidth; x ++) {
//			* toScreen = * snapPointer;
			unsigned int newRed   =	redValue   (* toScreen) * brightnessLevel + redValue   (* snapPointer) * brightnessLevel2;
			unsigned int newGreen = greenValue (* toScreen) * brightnessLevel + greenValue (* snapPointer) * brightnessLevel2;
			unsigned int newBlue  = blueValue  (* toScreen) * brightnessLevel + blueValue  (* snapPointer) * brightnessLevel2;
			* (toScreen ++) = makeColour (newRed >> 8, newGreen >> 8, newBlue >> 8);
			snapPointer ++;
		}
	}
}

void transitionSnapshotBox () {
	int boxBorderSize = 3;
	
	if (! snapshotImage) return; //transitionFader ();

	float howDone = (1.f - (brightnessLevel / 255.f)) / 2.f;

	if (howDone >= 0.5f)
	{
		howDone = 0.5f;
	}
	else if (howDone <= 0.f)
	{
		return;
	}

	int x1 = howDone * winWidth;
	int y1 = howDone * winHeight;

	if (y1 < x1)
		x1 = y1;
	else
		y1 = x1;
	
	int x2 = winWidth - x1;
	int y2 = winHeight - y1;
	int y = 0;
	
	for (; y < y1; y ++)
	{
		memcpy (screen + y * winWidth, snapshotImage[y], winWidth << 1);
	}
	for (; y < y1 + boxBorderSize; y ++)
	{
		memcpy (screen + y * winWidth, snapshotImage[y], x1 << 1);
		memset (screen + y * winWidth + x1, 0, (x2 - x1) << 1);
		memcpy (screen + y * winWidth + x2, snapshotImage[y] + x2, x1 << 1);
	}
	for (; y < y2 - boxBorderSize; y ++)
	{
		memcpy (screen + y * winWidth, snapshotImage[y], x1 << 1);
		memset (screen + y * winWidth + x1, 0, boxBorderSize << 1);
		memset (screen + y * winWidth + x2 - boxBorderSize, 0, boxBorderSize << 1);
		memcpy (screen + y * winWidth + x2, snapshotImage[y] + x2, x1 << 1);
	}
	for (; y < y2; y ++)
	{
		memcpy (screen + y * winWidth, snapshotImage[y], x1 << 1);
		memset (screen + y * winWidth + x1, 0, (x2 - x1) << 1);
		memcpy (screen + y * winWidth + x2, snapshotImage[y] + x2, x1 << 1);
	}
	for (; y < winHeight; y ++)
	{
		memcpy (screen + y * winWidth, snapshotImage[y], winWidth << 1);
	}
}

//----------------------------------------------------
// FAST PSEUDO-RANDOM NUMBER STUFF FOR DISOLVE EFFECT
//----------------------------------------------------

#define KK 17
unsigned long randbuffer[KK][2];  // history buffer
int p1, p2;

void resetRandW () {
	long int seed = 12345;
	
	for (int i=0; i<KK; i++) {
		for (int j=0; j<2; j++) {
			seed = seed * 2891336453u + 1;
			randbuffer[i][j] = seed;
		}
	}
	
	p1 = 0, p2 = 10;
}

void transitionDisolve () {
	if (! brightnessLevel) {
		blackBuffer ();
		return;
	}
	
	unsigned long n;
	unsigned long y;

	unsigned short int * toScreen = screen;
	unsigned short int * end = screen + (winWidth * winHeight);
	
	do {
		// generate next number
		n = randbuffer[p1][1];
		y = (n << 27) | (n >> (32 - 27)) + randbuffer[p2][1];

		n = randbuffer[p1][0];
		randbuffer[p1][1] = (n << 19) | (n >> (32 - 19)) + randbuffer[p2][0];
		randbuffer[p1][0] = y;

		// rotate list pointers
		if (! p1 --) p1 = KK - 1;
		if (! p2 --) p2 = KK - 1;
		
//		(* toScreen) = y;
		if ((y & 255u) > brightnessLevel) (* toScreen) = 0;
	} while (++ toScreen != end);
}

void transitionTV () {
	unsigned long n;
	unsigned long y;

	unsigned short int * toScreen = screen;
	unsigned short int * end = screen + (winWidth * winHeight);
	
	do {
		// generate next number
		n = randbuffer[p1][1];
		y = (n << 27) | (n >> (32 - 27)) + randbuffer[p2][1];

		n = randbuffer[p1][0];
		randbuffer[p1][1] = (n << 19) | (n >> (32 - 19)) + randbuffer[p2][0];
		randbuffer[p1][0] = y;

		// rotate list pointers
		if (! p1 --) p1 = KK - 1;
		if (! p2 --) p2 = KK - 1;
		
//		(* toScreen) = y;
		if ((y & 255u) > brightnessLevel) (* toScreen) = makeGrey (n & 255);
	} while (++ toScreen != end);
}

void transitionBlinds () {
	int i = 0, cutoff = (brightnessLevel - 16) >> 4, w = winWidth << 1;

	unsigned short int * toScreen = screen;
	unsigned short int * end = screen + (winWidth * winHeight);
	
	do {
		i ++;
		i &= 15;
		if (i >= cutoff) memset (toScreen, 0, w);
		toScreen += winWidth;
	} while (toScreen != end);
}

//----------------------------------------------------

void fixBrightness () {
	switch (fadeMode) {
		case 0:		transitionFader ();				break;
		case 1:		resetRandW ();
		case 2:		transitionDisolve ();			break;
		case 3:		transitionTV ();				break;
		case 4:		transitionBlinds ();			break;
		case 5:		transitionCrossFader ();		break;
		case 6:		transitionSnapshotBox ();		break;
	}
}
