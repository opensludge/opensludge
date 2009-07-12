#include <stdlib.h>

extern unsigned short int * * backDropImage;
extern int HORZ_RES, VERT_RES;

void drawLine(int x1, int y1, int x2, int y2, unsigned short adder) {
	int i, deltax, deltay, numpixels, d, dinc1, dinc2, x, xinc1, xinc2, y, yinc1, yinc2;

	// Calculate deltax and deltay for initialisation }
	deltax = abs(x2 - x1);
	deltay = abs(y2 - y1);

	// Initialize all vars based on which is the independent variable }
	if (deltax >= deltay) {
		// x is independent variable }
		numpixels = deltax + 1;
		d = (2 * deltay) - deltax;
		dinc1 = deltay << 1;
		dinc2 = (deltay - deltax) << 1;
		xinc1 = 1;
		xinc2 = 1;
		yinc1 = 0;
		yinc2 = 1;
	} else {
		// y is independent variable }
		numpixels = deltay + 1;
		d = (2 * deltax) - deltay;
		dinc1 = deltax << 1;
		dinc2 = (deltax - deltay) << 1;
		xinc1 = 0;
		xinc2 = 1;
		yinc1 = 1;
		yinc2 = 1;
	}
	
	// Make sure x and y move in the right directions }
	if (x1 > x2) {
		xinc1 = -xinc1;
		xinc2 = -xinc2;
	}
	if (y1 > y2) {
		yinc1 = -yinc1;
		yinc2 = -yinc2;
	}
	
	// Start drawing at <x1, y1> }
	x = x1;
	y = y1;

	// Draw the pixels }
	for (i = 0; i < numpixels; i ++) {
		if (x < HORZ_RES && y < VERT_RES && x >= 0 && y >= 0)
			backDropImage[y][x] += adder;
      	
		if (d < 0) {
			d += dinc1;
			x += xinc1;
			y += yinc1;
		} else {
			d += dinc2;
			x += xinc2;
			y += yinc2;
		}
	}
}
