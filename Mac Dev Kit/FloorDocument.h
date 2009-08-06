//
//  Floor.h
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2009-07-27.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include <OpenGL/gl.h>

#include "FloorMaker.h"
#include "Sprites.h"

@interface FloorOpenGLView : NSOpenGLView
{
	id doc;

	struct spriteBank *backdrop;
	int x, y, w, h;
	float r,g,b;
	float z, zmul;
	
}
- (void) connectToDoc: (id) myDoc;
- (void) drawRect: (NSRect) bounds;
- (void) setFloorColour: (NSColor *) colour;
@end


@interface FloorDocument : NSDocument {
	
	IBOutlet FloorOpenGLView *floorView;
	
	IBOutlet NSColorWell *floorColourWell;

	struct spriteBank backdrop;
	struct polyList * firstPoly;
	
}
- (struct polyList *) getFloor ;

- (IBAction)loadBackdrop:(id)sender;
- (IBAction)setFloorColour:(id)sender;

@end
