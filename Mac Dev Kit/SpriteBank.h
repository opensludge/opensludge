//
//  SpriteBank.h
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2009-07-18.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#include <OpenGL/gl.h>
#include "Sprites.h"

@interface SpriteOpenGLView : NSOpenGLView
{
	id doc;
	struct spriteBank *sprites;
	int spriteIndex;
	bool showBox;
	int x, y, w, h;
	float z;
}
- (void) connectToDoc: (id) myDoc;
- (void) drawRect: (NSRect) bounds ;
- (int) spriteIndex;
- (void) setSpriteIndex: (int)i;
@end


@interface SpriteBank : NSDocument {
	
	IBOutlet SpriteOpenGLView *spriteView;
	IBOutlet NSSlider *spriteIndexSlider;
				
	struct spriteBank sprites;
	
	int hotSpotX, hotSpotY;
}
- (struct spriteBank *) getSprites;

- (IBAction)hotSpotCentre:(id)sender;
- (IBAction)hotSpotBase:(id)sender;

@end

