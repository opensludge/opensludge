#ifdef WIN32

#include "percbar.h"
#include "wintext.h"
#include "winterfa.h"

#define BOXX		13
#define BOXWIDTH	280
#define BOX1		86
#define BOX2		132
#define BOXHEIGHT	20

unsigned int percTotal[2] = {100, 100}, lastVal[2] = {0, 0};

void clearRect (int i, whichPerc whichBox) {
	percTotal[whichBox] = i ? i : 1;
	percRect (i ? 0 : 1, whichBox);
}

void percRect (unsigned int i, whichPerc whichBox) {
	HDC hdc = GetDC (compWin);
	char buffff[10];
	
	HPEN myPen = CreatePen (PS_SOLID, 0, RGB (0, 0, 0));
	HBRUSH myBrush = CreateSolidBrush (RGB (255, 255, 0));
	HPEN lastPen = (HPEN) SelectObject (hdc, myPen);
	HBRUSH lastBrush = (HBRUSH) SelectObject (hdc, myBrush);

	lastVal[whichBox] = i;

	if (percTotal[whichBox]) {
		sprintf (buffff, "%i%", (i * 100) / percTotal[whichBox]);
		i = (i * BOXWIDTH) / percTotal[whichBox];
	} else {
		sprintf (buffff, "");
		i = 0;
	}

	if (whichBox) {
		if (i) {
			Rectangle (hdc, BOXX - 1, BOX2, BOXX + i, BOX2 + BOXHEIGHT);
		}
		SelectObject (hdc, lastBrush);
		DeleteObject (myBrush);

		myBrush = CreateSolidBrush (RGB (128, 0, 0));
		lastBrush = (HBRUSH) SelectObject (hdc, myBrush);
		Rectangle (hdc, BOXX - 1 + i, BOX2, BOXX + BOXWIDTH, BOX2 + BOXHEIGHT);
		SelectObject (hdc, lastBrush);
		DeleteObject (myBrush);
	} else {
		if (i) {
			Rectangle (hdc, BOXX - 1, BOX1, BOXX + i, BOX1 + BOXHEIGHT);
		}
		SelectObject (hdc, lastBrush);
		DeleteObject (myBrush);

		myBrush = CreateSolidBrush (RGB (0, 128, 0));
		lastBrush = (HBRUSH) SelectObject (hdc, myBrush);
		Rectangle (hdc, BOXX - 1 + i, BOX1, BOXX + BOXWIDTH, BOX1 + BOXHEIGHT);
		SelectObject (hdc, lastBrush);
		DeleteObject (myBrush);
	}

	SelectObject (hdc, lastPen);
	DeleteObject (myPen);
	ReleaseDC (compWin, hdc);
}

#endif