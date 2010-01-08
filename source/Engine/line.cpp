
//#include <SDL_opengl.h>
#include "GLee.h"
#include <stdlib.h>
#include "Graphics.h"
#include "Sprites_AA.h"

#include "ALLFILES.H"

extern int sceneWidth, sceneHeight;
extern GLuint backdropTextureName;


void drawLine(int x1, int y1, int x2, int y2) {
	int x, y;
	bool backwards = false;
	
	if (x1 < 0)  x1 = 0;
	if (y1 < 0)  y1 = 0;
	if (x2 < 0)  x2 = 0;
	if (y2 < 0)  y2 = 0;
	if (x1 > sceneWidth) x1 = sceneWidth - 1;
	if (x2 > sceneWidth) x2 = sceneWidth - 1;
	if (y1 > sceneHeight) y1 = sceneHeight - 1;
	if (y2 > sceneHeight) y2 = sceneHeight - 1;

	if (x1 > x2) {
		x = x2; 
		backwards = !backwards;
	} else x = x1;
		
	if (y1 > y2) {
		y = y2; 
		backwards = !backwards;
	} else y = y1;	
	
	int diffX = abs(x2-x1);
	int diffY = abs(y2-y1);	
	
	if (! diffX) {
		diffX = 1;
		if (x == sceneWidth - 1) x = sceneWidth -2;
	}
	if (! diffY) {
		diffY = 1;
		if (y == sceneHeight - 1) y = sceneHeight -2;
	}
	
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	setPixelCoords (true);
	
	glLineWidth (2.0);
	glColor3ub (255, 255, 255);
	
	int xoffset = 0;
	while (xoffset < diffX) {
		int w = (diffX-xoffset < viewportWidth) ? diffX-xoffset : viewportWidth;
		
		int yoffset = 0;
		while (yoffset < diffY) {
			int h = (diffY-yoffset < viewportHeight) ? diffY-yoffset : viewportHeight;
			
			// Render the scene - first the old backdrop
			glEnable (GL_TEXTURE_2D);
			glBindTexture (GL_TEXTURE_2D, backdropTextureName);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex3f(-x-xoffset, 1-y-yoffset, 0.0);
			glTexCoord2f(backdropTexW, 0.0); glVertex3f(sceneWidth-x-xoffset, 1-y-yoffset, 0.0);
			glTexCoord2f(backdropTexW, backdropTexH); glVertex3f(sceneWidth-x-xoffset, sceneHeight-y-yoffset, 0.0);
			glTexCoord2f(0.0, backdropTexH); glVertex3f(-x-xoffset, sceneHeight-y-yoffset, 0.0);
			glEnd();
			
			glDisable (GL_TEXTURE_2D);
			
			// Then the line
			glEnable (GL_COLOR_LOGIC_OP);
			glLogicOp (GL_XOR);
			
			glBegin(GL_LINES);			
			if (! backwards) {
				glVertex3f(-xoffset, -yoffset, 0.0);
				glVertex3f(diffX-xoffset, -yoffset+diffY, 0.0);
			} else {
				glVertex3f(diffX-xoffset, -yoffset, 0.0);
				glVertex3f(-xoffset, -yoffset+diffY, 0.0);
			}
			glEnd();
			
			glDisable (GL_COLOR_LOGIC_OP);				
				
			// Copy Our ViewPort To The Texture
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, x+xoffset, y+yoffset, viewportOffsetX, viewportOffsetY, w, h);
			
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
