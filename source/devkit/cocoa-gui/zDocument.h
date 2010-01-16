//
//  zDocument.h
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2009-08-07.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


//
//  SpriteBank.h
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2009-07-18.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#include <OpenGL/gl.h>
#include "sprites.h"

@interface zOpenGLView : NSOpenGLView
{
	struct spriteBank *backdrop;

	id doc;
	int x, y, w, h;
	float z, zmul;
}
- (void) connectToDoc: (id) myDoc;
- (void) drawRect: (NSRect) bounds ;
@end


@interface zDocument : NSDocument {
	struct spriteBank backdrop;
	
	IBOutlet zOpenGLView *zView;
	IBOutlet NSSlider *zBufSlider;
	
	IBOutlet NSTextField *numBuffers;
	IBOutlet NSTextField *bufferYTextField;

	int buffer;
	int bufferY;
}
- (int) buffer;
- (void) setBuffer: (int)i;

- (int) bufferY;
- (void) setBufferY: (int)i;

//- (IBAction)setBufferY:(id)sender;

@end
