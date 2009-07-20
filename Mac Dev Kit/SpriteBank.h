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



@interface SpriteBank : NSDocument {
	
	IBOutlet NSOpenGLView *spriteView;
	IBOutlet NSSlider *spriteIndexSlider;
		
	struct spriteBank sprites;
}
- (struct spriteBank *) getSprites;


@end

@interface SpriteOpenGLView : NSOpenGLView
{
	SpriteBank *doc;
	struct spriteBank *sprites;
	int spriteIndex;
	int x, y, w, h;
	float z;
	//NSPoint lastMouseLoc;
	//bool draggingView;
}
- (void) connectToDoc: (SpriteBank *) myDoc;
- (void) drawRect: (NSRect) bounds ;
@end
