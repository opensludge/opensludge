#ifdef _MSC_VER
#include <complex>
#endif
#include <stdlib.h>

#include "glee.h"
//#include <SDL_opengl.h>


#include "allfiles.h"

#include "winstuff.h"
#include "newfatal.h"
#include "colours.h"
#include "fileset.h"
#include "cursors.h"
#include "backdrop.h"
#include "moreio.h"
#include "variable.h"
#include "zbuffer.h"
#include "Graphics.h"
#include "line.h"
#include "sprites_AA.h"

bool freeze ();
void unfreeze (bool);	// Because freeze.h needs a load of other includes

GLubyte * backdropTexture = NULL;
GLuint backdropTextureName = NULL;
bool backdropExists = false;
double backdropTexW = 1.0;
double backdropTexH = 1.0;

GLubyte * lightMapTexture = NULL;
GLuint lightMapTextureName = NULL;

GLuint snapshotTextureName = 0;

int lightMapMode = LIGHTMAPMODE_PIXEL;

parallaxLayer * parallaxStuff = NULL;

int cameraPX = 0, cameraPY = 0;

int sceneWidth, sceneHeight;
int lightMapNumber;
unsigned int currentBlankColour = makeColour (0, 0, 0);

extern int cameraX, cameraY;

void nosnapshot () {
	glDeleteTextures (1, &snapshotTextureName);
	snapshotTextureName = 0;
}


bool snapshot () {
	nosnapshot ();
	if (! freeze ()) return false;
	snapshotTextureName = backdropTextureName;
	unfreeze (false);
	return true;
}

bool restoreSnapshot (FILE * fp) {
	int realPicWidth, realPicHeight;
	int picWidth = realPicWidth = get2bytes (fp);
	int picHeight = realPicHeight = get2bytes (fp);
	

	int t1, t2, n;
	unsigned short c;	
	
	GLubyte * target;
	if (! NPOT_textures) {
		picWidth = getNextPOT(picWidth);
		picHeight = getNextPOT(picHeight);
	}
	GLubyte * snapshotTexture = new GLubyte [picHeight*picWidth*4];

	for (t2 = 0; t2 < realPicHeight; t2 ++) {
		t1 = 0;
		while (t1 < realPicWidth) {
			c = (unsigned short) get2bytes (fp);
			if (c & 32) {
				n = fgetc (fp) + 1;
				c -= 32;
			} else {
				n = 1;
			}
			while (n --) {
				target = snapshotTexture + 4*picWidth*t2 + t1*4;
				target[0] = (GLubyte) redValue(c);
				target[1] = (GLubyte) greenValue(c);
				target[2] = (GLubyte) blueValue(c);
				target[3] = (GLubyte) 255;
				t1++;
			}
		}
	}
	
	if (! snapshotTextureName) glGenTextures (1, &snapshotTextureName);
	glBindTexture(GL_TEXTURE_2D, snapshotTextureName);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, picWidth, picHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, snapshotTexture);
	
	delete snapshotTexture;
	
	return true;
}

bool reserveNewLightMap () {
	int picWidth, picHeight;
	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
	
	if (lightMapTexture) delete lightMapTexture;
	if (! NPOT_textures) {
		picWidth = getNextPOT(sceneWidth);
		picHeight = getNextPOT(sceneHeight);
	}	
	lightMapTexture = new GLubyte [picWidth*picHeight*4];
	
	if (! lightMapTextureName) glGenTextures (1, &lightMapTextureName);
	glBindTexture (GL_TEXTURE_2D, lightMapTextureName);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, picWidth, picHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, lightMapTexture);

	return true;
}

void killBackDrop () {
	glDeleteTextures (1, &backdropTextureName);
	backdropTextureName = 0;
	backdropExists = false;
}

void killLightMap () {
	glDeleteTextures (1, &lightMapTextureName);
	lightMapTextureName = 0;
	if (lightMapTexture) {
		delete lightMapTexture;
		lightMapTexture = NULL;
	}
	lightMapNumber = 0;
}

void killParallax () {
	while (parallaxStuff) {
	
		parallaxLayer * k = parallaxStuff;
		parallaxStuff = k -> next;
		
		// Now kill the image
		glDeleteTextures (1, &k -> textureName);
		delete k -> texture;
		delete k;
	}
}

bool reserveBackdrop () {
	cameraX = 0;
	cameraY = 0;
	int picWidth = sceneWidth;
	int picHeight = sceneHeight;

	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
	
	if (backdropTexture) delete backdropTexture;
	if (! NPOT_textures) {
		picWidth = getNextPOT(sceneWidth);
		picHeight = getNextPOT(sceneHeight);
		backdropTexW = (double) sceneWidth / picWidth;
		backdropTexH = (double) sceneHeight / picHeight;
	}	
	backdropTexture = new GLubyte [picWidth*picHeight*4];
	
	if (! backdropTextureName) glGenTextures (1, &backdropTextureName);
	glBindTexture (GL_TEXTURE_2D, backdropTextureName);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if (maxAntiAliasSettings.useMe) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //GL_NEAREST
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, picWidth, picHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, backdropTexture);
		
	return true;
}

bool resizeBackdrop (int x, int y) {
	killLightMap ();
	killBackDrop ();
	killParallax ();
	killZBuffer ();
	sceneWidth = x;
	sceneHeight = y;
	return reserveBackdrop ();
}

void loadBackDrop (int fileNum, int x, int y) {
	setResourceForFatal (fileNum);

	if (! openFileFromNum (fileNum)) {
		fatal ("Can't load overlay image");
		return;
	}

	if (! loadHSI (bigDataFile, x, y, false)) {
		char mess[200];
		sprintf (mess, "Can't paste overlay image outside scene dimensions\n\nX = %i\nY = %i\nWidth = %i\nHeight = %i", x, y, sceneWidth, sceneHeight);
		fatal (mess);
	}

	finishAccess ();
	setResourceForFatal (-1);
}

void mixBackDrop (int fileNum, int x, int y) {
	setResourceForFatal (fileNum);
	if (! openFileFromNum (fileNum)) {
		fatal ("Can't load overlay image");
		return;
	}

	if (! mixHSI (bigDataFile, x, y)) {
		fatal ("Can't paste overlay image outside screen dimensions");
	}

	finishAccess ();
	setResourceForFatal (-1);
}

void blankScreen (int x1, int y1, int x2, int y2) {

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (x2 >= sceneWidth) x2 = sceneWidth - 1;
	if (y2 >= sceneHeight) y2 = sceneHeight - 1;

	int picWidth = x2-x1;
	int picHeight = y2-y1;
	
	setPixelCoords (true);
	
	int xoffset = 0;
	while (xoffset < picWidth) {
		int w = (picWidth-xoffset < viewportWidth) ? picWidth-xoffset : viewportWidth;
		
		int yoffset = 0;
		while (yoffset < picHeight) {
			int h = (picHeight-yoffset < viewportHeight) ? picHeight-yoffset : viewportHeight;
			
			// Render the scene 
			glColor3ub(redValue(currentBlankColour), greenValue(currentBlankColour), blueValue(currentBlankColour));
			
			glDisable (GL_TEXTURE_2D);
			glBegin(GL_QUADS);
			glVertex3f(-10.325, -1.325, 0.0);
			glVertex3f(w+1.325, -1.325, 0.0);
			glVertex3f(w+1.325, h+1.325, 0.0);
			glVertex3f(-10.325, h+1.325, 0.0);
			glEnd();							
			
			// Copy Our ViewPort To The Texture
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, x1+xoffset, y1+yoffset, viewportOffsetX, viewportOffsetY, w, h);
			
			yoffset += viewportHeight;
		}		
		xoffset += viewportWidth;
	}
	setPixelCoords (false);
}

void hardScroll (int distance) {
	if (abs (distance) >= sceneHeight) {
		blankScreen (0, 0, sceneWidth, sceneHeight);
		return;
	}
	
	if (! distance) return;

	setPixelCoords (true);
	
	int xoffset = 0;
	while (xoffset < sceneWidth) {
		int w = (sceneWidth-xoffset < viewportWidth) ? sceneWidth-xoffset : viewportWidth;
		
		int yoffset = 0;
		while (yoffset < sceneHeight) {
			int h = (sceneHeight-yoffset < viewportHeight) ? sceneHeight-yoffset : viewportHeight;

			glClear(GL_COLOR_BUFFER_BIT);	// Clear The Screen
	
			// Render the backdrop
			glBindTexture (GL_TEXTURE_2D, backdropTextureName);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			
			glEnable (GL_TEXTURE_2D);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex3f(0.325-xoffset, 0.325-distance-yoffset, 0.0);
			glTexCoord2f(1.0, 0.0); glVertex3f(sceneWidth-0.325-xoffset, 0.325-distance-yoffset, 0.0);
			glTexCoord2f(1.0, 1.0); glVertex3f(sceneWidth-0.325-xoffset, sceneHeight-0.325-distance-yoffset, 0.0);
			glTexCoord2f(0.0, 1.0); glVertex3f(0.325-xoffset, sceneHeight-0.325-distance-yoffset, 0.0);
			glEnd();	

		
			// Copy Our ViewPort To The Texture
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, viewportOffsetX, viewportOffsetY, w, h);
			
			yoffset += viewportHeight;
		}		
		xoffset += viewportWidth;
	}
	glDisable(GL_TEXTURE_2D);
	setPixelCoords (false);
	if (maxAntiAliasSettings.useMe) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}			
}

void drawVerticalLine (unsigned int x, unsigned int y1, unsigned int y2) {
	drawLine (x, y1, x, y2);
}

void drawHorizontalLine (unsigned int x1, unsigned int y, unsigned int x2) {
	drawLine (x1, y, x2, y);
}

void darkScreen () {
	setPixelCoords (true);
	
	int xoffset = 0;
	while (xoffset < sceneWidth) {
		int w = (sceneWidth-xoffset < viewportWidth) ? sceneWidth-xoffset : viewportWidth;
		
		int yoffset = 0;
		while (yoffset < sceneHeight) {
			int h = (sceneHeight-yoffset < viewportHeight) ? sceneHeight-yoffset : viewportHeight;
			
			// Render the scene - first the old backdrop
			glEnable (GL_TEXTURE_2D);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glBindTexture (GL_TEXTURE_2D, backdropTextureName);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex3f(0.325-xoffset, 0.325-yoffset, 0.0);
			glTexCoord2f(1.0, 0.0); glVertex3f(sceneWidth-0.325-xoffset, 0.325-yoffset, 0.0);
			glTexCoord2f(1.0, 1.0); glVertex3f(sceneWidth-0.325-xoffset, sceneHeight-0.325-yoffset, 0.0);
			glTexCoord2f(0.0, 1.0); glVertex3f(0.325-xoffset, sceneHeight-0.325-yoffset, 0.0);
			glEnd();	
			

			// Then the darkness
			glDisable (GL_TEXTURE_2D);
			glColor4ub (0, 0, 0, 127);
			glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
			
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			
			glBegin(GL_QUADS);			
			glVertex3f(-xoffset, -yoffset, 0.0);
			glVertex3f(sceneWidth-xoffset, -yoffset, 0.0);
			glVertex3f(sceneWidth-xoffset, sceneHeight-yoffset, 0.0);
			glVertex3f(-xoffset, sceneHeight-yoffset, 0.0);
			glEnd();
			glDisable(GL_BLEND);
			glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			
			// Copy Our ViewPort To The Texture
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, viewportOffsetX, viewportOffsetY, w, h);
			
			yoffset += viewportHeight;
		}		
		xoffset += viewportWidth;
	}
	setPixelCoords (false);
	if (maxAntiAliasSettings.useMe) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}			
}

inline int sortOutPCamera (int cX, int fX, int sceneMax, int boxMax) {
	return (fX == 65535) ?
		(sceneMax ? ((cX * boxMax) / sceneMax) : 0)
	:
		((cX * fX) / 100);
}


/* 
/ copyToBackdrop
/ This function is used to copy the old backdrop when doing a freeze
*/
void copyToBackDrop (GLuint fromHere, int orW, int orH, int orX, int orY, parallaxLayer * parallaxS) {
				
	setPixelCoords (true);
	
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glEnable (GL_TEXTURE_2D);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen
	
	int xoffset = 0;
	
	while (xoffset < winWidth) {
		int w = (winWidth-xoffset < viewportWidth) ? winWidth-xoffset : viewportWidth;
		
		int yoffset = 0;
		while (yoffset < orH) {
			int h = (winHeight-yoffset < viewportHeight) ? winHeight-yoffset : viewportHeight;

			if (parallaxS) {
				parallaxLayer * ps = parallaxS;
				
				while (ps->next) ps = ps->next;
				
				while (ps) {
					ps -> cameraX = sortOutPCamera (orX, ps -> fractionX, orW - w, ps -> width - winWidth);
					ps -> cameraY = sortOutPCamera (orY, ps -> fractionY, orH - h, ps -> height - winHeight);
					
					glBindTexture (GL_TEXTURE_2D, ps->textureName);
					glBegin(GL_QUADS);
					
					float texw = (ps->wrapS) ? (float) orW / ps->width: 1.0;
					float wt = (ps->wrapS) ? orW : ps->width;
					float texh = (ps->wrapT) ? (float) orH / ps->height: 1.0;
					float ht = (ps->wrapT) ? orH : ps->height;
					
					glTexCoord2f(0.0, 0.0); glVertex3f(0.325-ps -> cameraX-xoffset, 0.325-ps -> cameraY-yoffset, 0.1);
					glTexCoord2f(texw, 0.0); glVertex3f(wt-0.325 -ps -> cameraX-xoffset, 0.325-ps -> cameraY-yoffset, 0.1);
					glTexCoord2f(texw, texh); glVertex3f(wt-0.325 -ps -> cameraX-xoffset, ht -0.325 -ps -> cameraY-yoffset, 0.1);
					glTexCoord2f(0.0, texh); glVertex3f(0.325-ps -> cameraX-xoffset, ht -0.325 -ps -> cameraY-yoffset, 0.1);
					
					glEnd();
					
					ps = ps -> prev;
				}
			}
						
			
			// Render the backdrop
			glBindTexture (GL_TEXTURE_2D, fromHere);
			glColor4f(1.0, 1.0, 1.0, 1.0);
					
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex3i(0.325-orX-xoffset, 0.325-orY-yoffset, 0);
			glTexCoord2f(1.0, 0.0); glVertex3i(orW-0.325-orX-xoffset, 0.325-orY-yoffset, 0);
			glTexCoord2f(1.0, 1.0); glVertex3i(orW-0.325-orX-xoffset, orH-0.325-orY-yoffset, 0);
			glTexCoord2f(0.0, 1.0); glVertex3i(0.325-orX-xoffset, orH-0.325-orY-yoffset, 0);
			glEnd();	
								
			// Copy Our ViewPort To The Texture
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0+xoffset, 0+yoffset, viewportOffsetX, viewportOffsetY, w, h);

			yoffset += viewportHeight;
		}
		xoffset += viewportWidth;
	}
	

	setPixelCoords(false);
	backdropExists = true;
}




void drawBackDrop () {

	glColor4f(1.0, 1.0, 1.0, 1.0);
	glEnable (GL_TEXTURE_2D);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	if (parallaxStuff) {
		parallaxLayer * ps = parallaxStuff;
		
		while (ps->next) ps = ps->next;
		
		while (ps) {
			ps -> cameraX = sortOutPCamera (cameraX, ps -> fractionX, sceneWidth - winWidth, ps -> width - winWidth);
			ps -> cameraY = sortOutPCamera (cameraY, ps -> fractionY, sceneHeight - winHeight, ps -> height - winHeight);
			
			glBindTexture (GL_TEXTURE_2D, ps->textureName);
			glBegin(GL_QUADS);
			
			float texw = (ps->wrapS) ? (float) sceneWidth / ps->width: 1.0;
			float w = (ps->wrapS) ? sceneWidth : ps->width;
			float texh = (ps->wrapT) ? (float) sceneHeight / ps->height: 1.0;
			float h = (ps->wrapT) ? sceneHeight : ps->height;
			
			glTexCoord2f(0.0, 0.0); glVertex3f(0.325-ps -> cameraX, 0.325-ps -> cameraY, 0.1);
			glTexCoord2f(texw, 0.0); glVertex3f(w-0.325 -ps -> cameraX, 0.325-ps -> cameraY, 0.1);
			glTexCoord2f(texw, texh); glVertex3f(w-0.325 -ps -> cameraX, h -0.325 -ps -> cameraY, 0.1);
			glTexCoord2f(0.0, texh); glVertex3f(0.325-ps -> cameraX, h -0.325 -ps -> cameraY, 0.1);
			
			glEnd();
						
			ps = ps -> prev;
		}
	}
			
	glBindTexture (GL_TEXTURE_2D, backdropTextureName);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(0.325-cameraX, 0.325-cameraY, 0.0);
	glTexCoord2f(backdropTexW, 0.0); glVertex3f(sceneWidth-0.325-cameraX, 0.325-cameraY, 0.0);
	glTexCoord2f(backdropTexW, backdropTexH); glVertex3f(sceneWidth-0.325-cameraX, sceneHeight-0.325-cameraY, 0.0);
	glTexCoord2f(0.0, backdropTexH); glVertex3f(0.325-cameraX, sceneHeight-0.325-cameraY, 0.0);
	glEnd();
	glDisable(GL_BLEND);
	
}

bool loadLightMap (int v) {
	setResourceForFatal (v);
	if (! openFileFromNum (v)) return fatal ("Can't open light map.");

	int picWidth = get2bytes (bigDataFile);
	int picHeight = get2bytes (bigDataFile);
	
	if (picWidth != sceneWidth || picHeight != sceneHeight) {
		return fatal ("Light map width and height don't match scene width and height.");
	}
	int realPicWidth = picWidth;
	int realPicHeight = picHeight;
	if (! NPOT_textures) {
		picWidth = getNextPOT(realPicWidth);
		picHeight = getNextPOT(realPicHeight);
	}	
	
	killLightMap ();
	lightMapNumber = v;
	if (! reserveNewLightMap ()) {
		return fatal ("Out of memory loading light map.");
	}

	int t1, t2, n;
	unsigned short c;
	GLubyte * target;
	for (t2 = 0; t2 < realPicHeight; t2 ++) {
		t1 = 0;
		while (t1 < realPicWidth) {
			c = (unsigned short) get2bytes (bigDataFile);
			if (c & 32) {
				n = fgetc (bigDataFile) + 1;
				c -= 32;
			} else {
				n = 1;
			}
			while (n --) {
				target = lightMapTexture + 4*picWidth*t2 + t1*4;
				target[0] = (GLubyte) redValue(c);
				target[1] = (GLubyte) greenValue(c);
				target[2] = (GLubyte) blueValue(c);
				target[3] = (GLubyte) 255;
				t1++;
			}
		}
	}
	
	if (! lightMapTextureName) glGenTextures (1, &lightMapTextureName);
	glBindTexture(GL_TEXTURE_2D, lightMapTextureName);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, picWidth, picHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, lightMapTexture);
	
	finishAccess ();
	setResourceForFatal (-1);

	return true;
}

void reloadParallaxTextures () {
	parallaxLayer * nP = parallaxStuff;
	if (! nP) return;

	while (nP) {
		//fprintf (stderr, "Reloading parallax. (%d, %d) ", nP->width, nP->height);
		nP->textureName = 0;
		glGenTextures (1, &nP->textureName);
		glBindTexture (GL_TEXTURE_2D, nP->textureName);
		if (nP -> wrapS)
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		else
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		if (nP -> wrapT)
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		else
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		if (maxAntiAliasSettings.useMe) {
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		} else {
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, nP->width, nP->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nP->texture);
		nP = nP->next;
	}
}

bool loadParallax (unsigned short v, unsigned short fracX, unsigned short fracY) {
	setResourceForFatal (v);
	if (! openFileFromNum (v)) return fatal ("Can't open parallax image");

	parallaxLayer * nP = new parallaxLayer;
	if (! checkNew (nP)) return false;
	
	nP -> next = parallaxStuff;
	parallaxStuff = nP;
	if (nP -> next) {
		nP -> next -> prev = nP;
	}
	nP -> prev = NULL;

	int picWidth = nP -> width = get2bytes (bigDataFile);
	int picHeight = nP -> height = get2bytes (bigDataFile);
	if (! NPOT_textures) {
		picWidth = getNextPOT(picWidth);
		picHeight = getNextPOT(picHeight);
	}	
	
	nP -> fileNum = v;
	nP -> fractionX = fracX;
	nP -> fractionY = fracY;

	if (fracX == 65535) {
		nP -> wrapS = false;
		if (nP -> width < winWidth) {
			fatal ("For AUTOFIT parallax backgrounds, the image must be at least as wide as the game window/screen.");
			return false;
		}		
	} else {
		nP -> wrapS = true;	
	}

	if (fracY == 65535) {
		nP -> wrapT = false;
		if (nP -> height < winHeight) {
			fatal ("For AUTOFIT parallax backgrounds, the image must be at least as tall as the game window/screen.");
			return false;
		}		
	} else {
		nP -> wrapT = true;	
	}
		
	nP -> texture = new GLubyte [nP -> height * nP -> width * 4];
	
	if (! checkNew (nP -> texture)) return false;
	
	int t1, t2, n;
	unsigned short c;
	GLubyte * target;

	for (t2 = 0; t2 < picHeight; t2 ++) {
		t1 = 0;
		while (t1 < picWidth) {
			c = (unsigned short) get2bytes (bigDataFile);
			if (c & 32) {
				n = fgetc (bigDataFile) + 1;
				c -= 32;
			} else {
				n = 1;
			}
			while (n--) {
				target = nP -> texture + 4*nP->width*t2 + t1*4;
				if (c == 63519 || c == 2015) {
					target[0] = (GLubyte) 0;
					target[1] = (GLubyte) 0;
					target[2] = (GLubyte) 0;
					target[3] = (GLubyte) 0;
				} else {
					target[0] = (GLubyte) redValue(c);
					target[1] = (GLubyte) greenValue(c);
					target[2] = (GLubyte) blueValue(c);
					target[3] = (GLubyte) 255;
				}
				t1 ++;
			}
		}
	}

	glGenTextures (1, &nP->textureName);
	glBindTexture (GL_TEXTURE_2D, nP->textureName);
	if (nP -> wrapS)
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	else
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	if (nP -> wrapT)
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	else
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if (maxAntiAliasSettings.useMe) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, nP->width, nP->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nP->texture);
	
	finishAccess ();
	setResourceForFatal (-1);
	return true;
}

extern int viewportOffsetX, viewportOffsetY;


bool loadHSI (FILE * fp, int x, int y, bool reserve) {
	
	int realPicWidth, realPicHeight;
	int picWidth = realPicWidth = get2bytes (fp);
	int picHeight = realPicHeight = get2bytes (fp);
	int t1, t2, n;
	unsigned short c;
	GLubyte * target;
	signed long transCol = reserve ? -1 : 63519;

	if (x == IN_THE_CENTRE) x = (sceneWidth - realPicWidth) >> 1;
	if (y == IN_THE_CENTRE) y = (sceneHeight - realPicHeight) >> 1;
	if (x < 0 || x + realPicWidth > sceneWidth || y < 0 || y + realPicHeight > sceneHeight) return false;	
	
	if (! NPOT_textures) {
		picWidth = getNextPOT(picWidth);
		picHeight = getNextPOT(picHeight);
	}
	
	if (reserve) {
		if (! resizeBackdrop (sceneWidth, sceneHeight)) return false;
	}
	
			
	for (t2 = 0; t2 < realPicHeight; t2 ++) {
		t1 = 0;
		while (t1 < realPicWidth) {
			c = (unsigned short) get2bytes (fp);
			if (c & 32) {
				n = fgetc (fp) + 1;
				c -= 32;
			} else {
				n = 1;
			}
			while (n --) {
				target = backdropTexture + 4*picWidth*t2 + t1*4;
				if (c == transCol || c == 2015) {
					target[0] = (GLubyte) 0;
					target[1] = (GLubyte) 0;
					target[2] = (GLubyte) 0;
					target[3] = (GLubyte) 0;
				} else {
					target[0] = (GLubyte) redValue(c);
					target[1] = (GLubyte) greenValue(c);
					target[2] = (GLubyte) blueValue(c);
					target[3] = (GLubyte) 255;
				}
				t1++;
			}
		}
	}
		
	GLuint tmpTex;		 
	
	glGenTextures (1, &tmpTex);
	glBindTexture(GL_TEXTURE_2D, tmpTex);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, picWidth, picHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, backdropTexture);

	glEnable (GL_TEXTURE_2D);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	setPixelCoords (true);

	int xoffset = 0;
	while (xoffset < picWidth) {
		int w = (picWidth-xoffset < viewportWidth) ? picWidth-xoffset : viewportWidth;
		
		int yoffset = 0;
		while (yoffset < picHeight) {
			int h = (picHeight-yoffset < viewportHeight) ? picHeight-yoffset : viewportHeight;
			
			glClear(GL_COLOR_BUFFER_BIT);	// Clear The Screen

			if (backdropExists) {
				// Render the scene - first the old backdrop
				glBindTexture (GL_TEXTURE_2D, backdropTextureName);
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glColor4f(1.0, 1.0, 1.0, 1.0);
			
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0); glVertex3f(0.325-x-xoffset, 0.325-y+yoffset, 0.0);
				glTexCoord2f(1.0, 0.0); glVertex3f(sceneWidth-0.325-x-xoffset, 0.325-y+yoffset, 0.0);
				glTexCoord2f(1.0, 1.0); glVertex3f(sceneWidth-0.325-x-xoffset, -0.325-y+yoffset+sceneHeight, 0.0);
				glTexCoord2f(0.0, 1.0); glVertex3f(0.325-x-xoffset, -0.325-y+yoffset+sceneHeight, 0.0);
				glEnd();
			}


			// Then the new!
			glBindTexture(GL_TEXTURE_2D, tmpTex);
			
			glColor4f(1.0, 0.0, 0.0, 0.0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex3f(0.325-xoffset, 0.325-yoffset, 0.0);
			glTexCoord2f(1.0, 0.0); glVertex3f(picWidth-0.325-xoffset, 0.325-yoffset, 0.0);
			glTexCoord2f(1.0, 1.0); glVertex3f(picWidth-0.325-xoffset, -yoffset+picHeight-0.325, 0.0);
			glTexCoord2f(0.0, 1.0); glVertex3f(0.325-xoffset, -yoffset+picHeight-0.325, 0.0);
			glEnd();
		
			glDisable (GL_BLEND);
		
			// Copy Our ViewPort To The Texture
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, x+xoffset, y+yoffset, viewportOffsetX, viewportOffsetY, w, h);
		
			yoffset += viewportHeight;
		}
			
		xoffset += viewportWidth;
	}
	glDeleteTextures(1, &tmpTex);	

	setPixelCoords (false);
	if (maxAntiAliasSettings.useMe) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}			
	
	backdropExists = true;
	return true;
}

bool mixHSI (FILE * fp, int x, int y) {
	int realPicWidth, realPicHeight;
	int picWidth = realPicWidth = get2bytes (fp);
	int picHeight = realPicHeight = get2bytes (fp);

	if (x == IN_THE_CENTRE) x = (sceneWidth - realPicWidth) >> 1;
	if (y == IN_THE_CENTRE) y = (sceneHeight - realPicHeight) >> 1;
	if (x < 0 || x + realPicWidth > sceneWidth || y < 0 || y + realPicHeight > sceneHeight) return false;	

	if (! NPOT_textures) {
		picWidth = getNextPOT(picWidth);
		picHeight = getNextPOT(picHeight);
	}	
	int t1, t2, n;
	unsigned short c;
	GLubyte * target;
	signed long transCol = 63519;
	
	
	for (t2 = 0; t2 < realPicHeight; t2 ++) {
		t1 = 0;
		while (t1 < realPicWidth) {
			c = (unsigned short) get2bytes (fp);
			if (c & 32) {
				n = fgetc (fp) + 1;
				c -= 32;
			} else {
				n = 1;
			}
			while (n --) {
				target = backdropTexture + 4*picWidth*t2 + t1*4;
				if (c == transCol) {
					target[0] = (GLubyte) 255;
					target[1] = (GLubyte) 255;
					target[2] = (GLubyte) 255;
					target[3] = (GLubyte) 0;
				} else {
					target[0] = (GLubyte) redValue(c);
					target[1] = (GLubyte) greenValue(c);
					target[2] = (GLubyte) blueValue(c);
					target[3] = (GLubyte) 127;
				}
				t1++;
			}
		}
	}
	
	GLuint tmpTex;		 
	
	glGenTextures (1, &tmpTex);
	glBindTexture(GL_TEXTURE_2D, tmpTex);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, picWidth, picHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, backdropTexture);
	
	glEnable (GL_TEXTURE_2D);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	
	setPixelCoords (true);
	
	int xoffset = 0;
	while (xoffset < picWidth) {
		int w = (picWidth-xoffset < viewportWidth) ? picWidth-xoffset : viewportWidth;
		
		int yoffset = 0;
		while (yoffset < picHeight) {
			int h = (picHeight-yoffset < viewportHeight) ? picHeight-yoffset : viewportHeight;
			
			// Render the scene - first the old backdrop
			glBindTexture (GL_TEXTURE_2D, backdropTextureName);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glColor3f(0.0, 1.0, 0.0);
			
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex3f(0.325-x-xoffset, 0.325-y-yoffset, 0.0);
			glTexCoord2f(1.0, 0.0); glVertex3f(sceneWidth-0.325-x-xoffset, 0.325-y-yoffset, 0.0);
			glTexCoord2f(1.0, 1.0); glVertex3f(sceneWidth-0.325-x-xoffset, sceneHeight-0.325-y-yoffset, 0.0);
			glTexCoord2f(0.0, 1.0); glVertex3f(0.325-x-xoffset, sceneHeight-0.325-y-yoffset, 0.0);
			glEnd();	
			
			// Then the new!
			glBindTexture(GL_TEXTURE_2D, tmpTex);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			
			glColor3f(1.0, 0.0, 0.0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex3f(0.325-xoffset, 0.325-yoffset, 0.0);
			glTexCoord2f(1.0, 0.0); glVertex3f(picWidth-0.325-xoffset, 0.325-yoffset, 0.0);
			glTexCoord2f(1.0, 1.0); glVertex3f(picWidth-0.325-xoffset, picHeight-0.325-yoffset, 0.0);
			glTexCoord2f(0.0, 1.0); glVertex3f(0.325-xoffset, picHeight-0.325-yoffset, 0.0);
			glEnd();
			
			glDisable (GL_BLEND);
			
			// Copy Our ViewPort To The Texture
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, x+xoffset, y+yoffset, viewportOffsetX, viewportOffsetY, w, h);
			
			yoffset += viewportHeight;
		}
		
		xoffset += viewportWidth;
	}
	glDeleteTextures(1, &tmpTex);	
	setPixelCoords (false);
	if (maxAntiAliasSettings.useMe) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}			
	
	return true;
}

void saveCoreHSI (FILE * writer, GLuint texture) {

	GLint w, h;
	
	glBindTexture (GL_TEXTURE_2D, texture);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
				
	GLushort* image = new GLushort [w*h];
	
	glPixelStorei (GL_PACK_ALIGNMENT, 1);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, image);
	
	int x, y, lookAhead;
	unsigned short int * fromHere, * lookPointer;

	put2bytes (w, writer);
	put2bytes (h, writer);
	
	for (y = 0; y < h; y ++) {
		fromHere = image +(y*w);
		x = 0;
		while (x < w) {
			lookPointer = fromHere + 1;
			for (lookAhead = x + 1; lookAhead < w; lookAhead ++) {
				if (lookAhead - x == 256) break;
				if (* fromHere != * lookPointer) break;
				lookPointer ++;
			}
			if (lookAhead == x + 1) {
				put2bytes ((* fromHere) & 65503, writer);
			} else {
				put2bytes (* fromHere | 32, writer);
				fputc (lookAhead - x - 1, writer);
			}
			fromHere = lookPointer;
			x = lookAhead;
		}
	}
}

void saveHSI (FILE * writer) {
	saveCoreHSI (writer, backdropTextureName);
}

bool getRGBIntoStack (unsigned int x, unsigned int y, stackHandler * sH) {
	if (x >= sceneWidth || y >= sceneHeight) {
		return fatal ("Co-ordinates are outside current scene!");
	}

	variable newValue;
	
	newValue.varType = SVT_NULL;
	
	glGetTexImage (GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, backdropTexture);
	
	GLubyte * target;
	if (! NPOT_textures) {
		target = backdropTexture + 4*getNextPOT(sceneWidth)*y + x*4;
	} else {	
		target = backdropTexture + 4*sceneWidth*y + x*4;
	}
	
	setVariable (newValue, SVT_INT, target[2]);
	if (! addVarToStackQuick (newValue, sH -> first)) return false;
	sH -> last = sH -> first;

	setVariable (newValue, SVT_INT, target[1]);
	if (! addVarToStackQuick (newValue, sH -> first)) return false;

	setVariable (newValue, SVT_INT, target[0]);
	if (! addVarToStackQuick (newValue, sH -> first)) return false;

	return true;
}

void saveParallaxRecursive (parallaxLayer * me, FILE * fp) {
	if (me) {
		saveParallaxRecursive (me -> next, fp);
		fputc (1, fp);
		put2bytes (me->fileNum, fp);
		put2bytes (me ->fractionX, fp);
		put2bytes (me->fractionY, fp);
	}
}

