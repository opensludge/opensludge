
#if !defined(HAVE_GLES2)
#include "GLee.h"
#else
#include <GLES2/gl2.h>
#endif

#include <stdlib.h>
#include "graphics.h"

#include "allfiles.h"

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
	
	setPixelCoords (true);
	
	glLineWidth (2.0);
	
	int xoffset = 0;
	while (xoffset < diffX) {
		int w = (diffX-xoffset < viewportWidth) ? diffX-xoffset : viewportWidth;
		
		int yoffset = 0;
		while (yoffset < diffY) {
			int h = (diffY-yoffset < viewportHeight) ? diffY-yoffset : viewportHeight;
			
			// Render the scene - first the old backdrop
			glBindTexture (GL_TEXTURE_2D, backdropTextureName);
			//glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

			const GLint vertices[] = { 
				-x-xoffset, 1-y-yoffset, 0, 
				sceneWidth-x-xoffset, 1-y-yoffset, 0, 
				-x-xoffset, sceneHeight-y-yoffset, 0,
				sceneWidth-x-xoffset, sceneHeight-y-yoffset, 0
			};

			const GLfloat texCoords[] = { 
				0.0f, 0.0f,
				backdropTexW, 0.0f,
				0.0f, backdropTexH,
				backdropTexW, backdropTexH
			}; 

			glUseProgram(shader.texture);
			setPMVMatrix(shader.texture);
			drawTexturedQuadNew(shader.texture, vertices, 1, texCoords);
			
			// Then the line
			//FIXME:Removing the lines doesn't work, but also didn't work properly before.

			GLint xo1=-xoffset, xo2=-xoffset;
			if (! backwards) {
				xo2 += diffX;
			} else {
				xo1 += diffX;
			}
			const GLint lineVertices[] = { 
				xo1, -yoffset, 0, 
				xo2, -yoffset+diffY, 0, 
			};

			glUseProgram(shader.color);
			setPMVMatrix(shader.color);
			glUniform4f(glGetUniformLocation(shader.color, "myColor"), 0.0f, 0.0f, 0.0f, 1.0f);

			int vertexLoc = glGetAttribLocation(shader.color, "myVertex");
			glEnableVertexAttribArray(vertexLoc);
			glVertexAttribPointer(vertexLoc, 3, GL_INT, GL_FALSE, 0, lineVertices);
			glDrawArrays(GL_LINES, 0, 2);
			glDisableVertexAttribArray(vertexLoc);

			glUseProgram(0);
	
			// Copy Our ViewPort To The Texture
			glBindTexture(GL_TEXTURE_2D, backdropTextureName);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, x+xoffset, y+yoffset, viewportOffsetX, viewportOffsetY, w, h);
			
			yoffset += viewportHeight;
		}		
		xoffset += viewportWidth;
	}
	setPixelCoords (false);	
}
