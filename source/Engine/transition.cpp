#include <string.h>

#include "ALLFILES.H"
#include "COLOURS.H"
#include "BACKDROP.H"
#include "Graphics.h"

extern GLuint snapshotTextureName;
extern unsigned char brightnessLevel;

unsigned char fadeMode = 2;



//----------------------------------------------------
// PROPER BRIGHTNESS FADING
//----------------------------------------------------

unsigned lastFrom, lastTo;

void transitionFader () {
	glDisable (GL_TEXTURE_2D);
	glColor4ub (0, 0, 0, 255 - brightnessLevel);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glBegin(GL_QUADS);			
	glVertex3f(0, winHeight, 0.0);
	glVertex3f(winWidth, winHeight, 0.0);
	glVertex3f(winWidth, 0, 0.0);
	glVertex3f(0, 0, 0.0);
	glEnd();
	glDisable(GL_BLEND);
}

void transitionCrossFader () {

	if (! snapshotTextureName) return;
	
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable (GL_TEXTURE_2D);
	glColor4ub (255, 255, 255, 255 - brightnessLevel);
	glBindTexture (GL_TEXTURE_2D,snapshotTextureName);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glBegin(GL_QUADS);			
	glTexCoord2f(0.0, backdropTexH); glVertex3f(0, winHeight, 0.0);
	glTexCoord2f(backdropTexW, backdropTexH); glVertex3f(winWidth, winHeight, 0.0);
	glTexCoord2f(backdropTexW, 0.0); glVertex3f(winWidth, 0, 0.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(0, 0, 0.0);
	glEnd();
	glDisable(GL_BLEND);
}

void transitionSnapshotBox () {
	
	if (! snapshotTextureName) return; 

	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D,snapshotTextureName);
	
	float xScale = (float) brightnessLevel * winWidth / 510.f;	// 510 = 255*2
	float yScale = (float) brightnessLevel * winHeight / 510.f;
	
	glBegin(GL_QUADS);			
	glTexCoord2f(0.0, backdropTexH); glVertex3f(xScale, winHeight-yScale, 0.0);
	glTexCoord2f(backdropTexW, backdropTexH); glVertex3f(winWidth-xScale, winHeight-yScale, 0.0);
	glTexCoord2f(backdropTexW, 0.0); glVertex3f(winWidth-xScale, yScale, 0.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(xScale, yScale, 0.0);
	glEnd();

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

GLubyte * transitionTexture = NULL;
GLuint transitionTextureName = NULL;

bool reserveTransitionTexture () {
	
	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
	
	if (! transitionTexture) 
		transitionTexture = new GLubyte [256*256*4];
	
	if (! transitionTextureName) glGenTextures (1, &transitionTextureName);
	glBindTexture (GL_TEXTURE_2D, transitionTextureName);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, transitionTexture);
	
	return true;
}


void transitionDisolve () {
	
	if (! transitionTextureName) reserveTransitionTexture();

	if (! brightnessLevel) {
		transitionFader ();
		return;
	}
	
	unsigned long n;
	unsigned long y;

	GLubyte * toScreen = transitionTexture;
	GLubyte * end = transitionTexture + (256 * 256*4);
	
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
		
		if ((y & 255u) > brightnessLevel) {
			toScreen[0] =toScreen[1] =toScreen[2] = 0;
			toScreen[3] = 255;
		} else {
			toScreen[0] =toScreen[1] =toScreen[2] =toScreen[3] = 0;
		}
		toScreen += 4;
	} while (toScreen < end);

	glBindTexture (GL_TEXTURE_2D, transitionTextureName);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, transitionTexture);

	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable (GL_TEXTURE_2D);
	glColor4ub (255, 255, 255, 255);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glBegin(GL_QUADS);			
	glTexCoord2f(0.0, 1.0); glVertex3f(0, winHeight, 0.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(winWidth, winHeight, 0.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(winWidth, 0, 0.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(0, 0, 0.0);
	glEnd();
	glDisable(GL_BLEND);
}

void transitionTV () {

	if (! transitionTextureName) reserveTransitionTexture();

	unsigned long n;
	unsigned long y;

	GLubyte * toScreen = transitionTexture;
	GLubyte * end = transitionTexture + (256 * 256*4);
	
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
		
		if ((y & 255u) > brightnessLevel) {
			toScreen[0] =toScreen[1] =toScreen[2] = (n & 255);
			toScreen[3] = (n & 255);
		} else {
			toScreen[0] =toScreen[1] =toScreen[2] =toScreen[3] = 0;
		}
		toScreen += 4;
	} while (toScreen < end);
	
	glBindTexture (GL_TEXTURE_2D, transitionTextureName);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, transitionTexture);
	
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable (GL_TEXTURE_2D);
	glColor4ub (255, 255, 255, 255);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glBegin(GL_QUADS);			
	glTexCoord2f(0.0, 1.0); glVertex3f(0, winHeight, 0.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(winWidth, winHeight, 0.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(winWidth, 0, 0.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(0, 0, 0.0);
	glEnd();
	glDisable(GL_BLEND);	
}

void transitionBlinds () {
	GLubyte stippleMask[128];

	int level = brightnessLevel/8;
	
	if (level) memset (stippleMask, 0, 4*level);
	if (level < 32) memset (stippleMask+level*4, 255, 4*(32-level));

	glDisable (GL_TEXTURE_2D);
	glColor4ub (0, 0, 0, 255);
	
	glPolygonStipple(stippleMask);
	glEnable(GL_POLYGON_STIPPLE);
	
	glBegin(GL_QUADS);			
	glVertex3f(0, winHeight, 0.0);
	glVertex3f(winWidth, winHeight, 0.0);
	glVertex3f(winWidth, 0, 0.0);
	glVertex3f(0, 0, 0.0);
	glEnd();
	glDisable(GL_POLYGON_STIPPLE);
	
}

//----------------------------------------------------

void fixBrightness () {
	switch (fadeMode) {
		case 0:		transitionFader ();				break;		
		case 1:		resetRandW ();					// Fall through!
		case 2:		transitionDisolve ();			break;
		case 3:		transitionTV ();				break;
		case 4:		transitionBlinds ();			break;
		case 5:		transitionCrossFader ();		break;
		case 6:		transitionSnapshotBox ();		break;
 
	}
}
