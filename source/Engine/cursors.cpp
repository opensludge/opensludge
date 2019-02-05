#if !defined(HAVE_GLES2)
#include <GL/glew.h>
#else
#include <GLES2/gl2.h>
#endif

#include "allfiles.h"
#include "cursors.h"
#include "colours.h"
#include "sprites.h"
#include "sprbanks.h"
#include "people.h"
#include "sludger.h"

personaAnimation * mouseCursorAnim;
int mouseCursorFrameNum = 0;
int mouseCursorCountUp = 0;

extern inputType input;

void pickAnimCursor (personaAnimation * pp) {
	deleteAnim (mouseCursorAnim);
	mouseCursorAnim = pp;
	mouseCursorFrameNum = 0;
	mouseCursorCountUp = 0;
}

void displayCursor () {
	if (mouseCursorAnim && mouseCursorAnim -> numFrames) {
		
		int spriteNum = mouseCursorAnim -> frames[mouseCursorFrameNum].frameNum;
		int flipMe = 0;

		if (spriteNum < 0) {
			spriteNum = -spriteNum; 
			flipMe = 1;
			if (spriteNum >= mouseCursorAnim -> theSprites -> bank.total) spriteNum = 0;
		} else {
			if (spriteNum >= mouseCursorAnim -> theSprites -> bank.total) flipMe = 2;
		}
		
		if (flipMe != 2) {
			(flipMe ? flipFontSprite : fontSprite) (input.mouseX, input.mouseY,
				mouseCursorAnim -> theSprites -> bank.sprites[spriteNum],
				mouseCursorAnim -> theSprites -> bank.myPalette /* ( spritePalette&) NULL*/);
		}

		if (++ mouseCursorCountUp >= mouseCursorAnim -> frames[mouseCursorFrameNum].howMany) {
			mouseCursorCountUp = 0;
			mouseCursorFrameNum ++;
			mouseCursorFrameNum %= mouseCursorAnim -> numFrames;
		}
	}
}

void pasteCursor (int x, int y, personaAnimation * c) {
	if (c -> numFrames) pasteSpriteToBackDrop (x, y,
						  c -> theSprites -> bank.sprites[c -> frames[0].frameNum],
						  c -> theSprites -> bank.myPalette);
}
