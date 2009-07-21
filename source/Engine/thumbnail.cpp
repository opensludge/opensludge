#include "allfiles.h"
#include "errors.h"
#include "moreio.h"
#include "version.h"
#include "sludger.h"
#include "colours.h"
#include "backdrop.h"
#include "Graphics.h"
#include "sprites_AA.h"
#include "newfatal.h"

bool freeze ();
void unfreeze (bool);	// Because freeze.h needs a load of other includes

int thumbWidth = 0, thumbHeight = 0;

extern GLuint backdropTextureName;


bool saveThumbnail (FILE * fp) {
	GLuint thumbnailTextureName = NULL;
	
	put4bytes (thumbWidth, fp);
	put4bytes (thumbHeight, fp);
		
	if (thumbWidth && thumbHeight) {
		if (! freeze ()) return false;
		
	
		glEnable (GL_TEXTURE_2D);
		setPixelCoords (true);
		
		glGenTextures (1, &thumbnailTextureName);
		glBindTexture(GL_TEXTURE_2D, thumbnailTextureName);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, thumbWidth, thumbHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		
		// Render the backdrop (scaled)
		glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glBindTexture (GL_TEXTURE_2D, backdropTextureName);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3f(0.325, 0.325, 0.0);
		glTexCoord2f(backdropTexW, 0.0); glVertex3f(thumbWidth-0.325, 0.325, 0.0);
		glTexCoord2f(backdropTexW, backdropTexH); glVertex3f(thumbWidth-0.325, thumbHeight-0.325, 0.0);
		glTexCoord2f(0.0, backdropTexH); glVertex3f(0.325, thumbHeight-0.325, 0.0);
		glEnd();	
		
		if (! maxAntiAliasSettings.useMe) {
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}	
		
		// Copy Our ViewPort To The Texture
		glBindTexture(GL_TEXTURE_2D, thumbnailTextureName);
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, viewportOffsetX, viewportOffsetY, thumbWidth, thumbHeight);
		
		GLushort* image = new GLushort [thumbWidth*thumbHeight];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, image);
		glDeleteTextures (1, &thumbnailTextureName);		
		thumbnailTextureName = 0;
		
		setPixelCoords (false);
	
		for (int y = 0; y < thumbHeight; y ++) {
			for (int x = 0; x < thumbWidth; x ++) {
				put2bytes ((*(image +y*thumbWidth+x)), fp);
			}
		}
		delete image;
		unfreeze (true);
	}	
	fputc ('!', fp);
	return true;
}

void showThumbnail (char * filename, int atX, int atY) {
	GLubyte * thumbnailTexture = NULL;
	GLuint thumbnailTextureName = NULL;

	int ssgVersion;
	FILE * fp = openAndVerify (filename, 'S', 'A', ERROR_GAME_LOAD_NO, ssgVersion);
	if (ssgVersion >= VERSION(1,4)) {
		if (fp == NULL) return;
		int fileWidth = get4bytes (fp);
		int fileHeight = get4bytes (fp);
		
		int picWidth = fileWidth;
		int picHeight = fileHeight;
		if (! NPOT_textures) {
			picWidth = getNextPOT(picWidth);
			picHeight = getNextPOT(picHeight);
		}
		
		thumbnailTexture = new GLubyte [picHeight*picWidth*4];
		if (thumbnailTexture == NULL) return;
		
		int t1, t2;
		unsigned short c;
		GLubyte * target;
		for (t2 = 0; t2 < fileHeight; t2 ++) {
			t1 = 0;
			while (t1 < fileWidth) {
				c = (unsigned short) get2bytes (fp);
				target = thumbnailTexture + 4*picWidth*t2 + t1*4;
				target[0] = (GLubyte) redValue(c);
				target[1] = (GLubyte) greenValue(c);
				target[2] = (GLubyte) blueValue(c);
				target[3] = (GLubyte) 255;
				t1++;
			}
		}

		fclose (fp);

		glGenTextures (1, &thumbnailTextureName);
		glBindTexture(GL_TEXTURE_2D, thumbnailTextureName);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, picWidth, picHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, thumbnailTexture);
		
		delete thumbnailTexture;

		if (atX<0){
			fileWidth+= atX;
			atX=0;
		}
		if (atY<0){
			fileHeight+= atY;
			atY=0;
		}
		if (fileWidth + atX > sceneWidth) fileWidth = sceneWidth-atX;
		if (fileHeight + atY > sceneHeight) fileHeight = sceneHeight-atY;
		
		glEnable (GL_TEXTURE_2D);
		setPixelCoords (true);
		int xoffset = 0;
		while (xoffset < fileWidth) {
			int w = (fileWidth-xoffset < viewportWidth) ? fileWidth-xoffset : viewportWidth;
			
			int yoffset = 0;
			while (yoffset < fileHeight) {
				int h = (fileHeight-yoffset < viewportHeight) ? fileHeight-yoffset : viewportHeight;
				
				glBindTexture (GL_TEXTURE_2D, thumbnailTextureName);
								
				glBegin(GL_QUADS);			
					glTexCoord2f(backdropTexW, 0.0); glVertex3f(fileWidth-0.325-xoffset, 0.325-yoffset, 0.0);
					glTexCoord2f(0.0, 0.0); glVertex3f(0.325-xoffset, 0.325-yoffset, 0.0);
					glTexCoord2f(0.0, backdropTexH); glVertex3f(0.325-xoffset, fileHeight-0.325-yoffset, 0.0);
					glTexCoord2f(backdropTexW, backdropTexH); glVertex3f(fileWidth-0.325-xoffset, fileHeight-0.325-yoffset, 0.0);
				glEnd();
				glDisable(GL_BLEND);
				
				// Copy Our ViewPort To The Texture
				glBindTexture(GL_TEXTURE_2D, backdropTextureName);
				glCopyTexSubImage2D(GL_TEXTURE_2D, 0, atX+xoffset, atY+yoffset, viewportOffsetX, viewportOffsetY, w, h);
				
				yoffset += viewportHeight;
			}		
			xoffset += viewportWidth;
		}
		
		setPixelCoords (false);
		glDeleteTextures (1, &thumbnailTextureName);
		thumbnailTextureName = 0;
	}
}

bool skipThumbnail (FILE * fp) {
	thumbWidth = get4bytes (fp);
	thumbHeight = get4bytes (fp);
	unsigned long skippy = thumbWidth;
	skippy *= thumbHeight << 1;
	fseek (fp, skippy, 1);
	return (fgetc (fp) == '!');
}
