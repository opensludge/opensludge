//
//  Floor.h
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2009-07-27.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#ifdef GNUSTEP
extern "C"
{
#import <Cocoa/Cocoa.h>
}
#else
#import <Cocoa/Cocoa.h>
#endif

#ifdef __linux__
#include <GL/gl.h>
#else
#include <OpenGL/gl.h>
#endif

#include "floormaker.h"
#include "sprites.h"

@interface FloorOpenGLView : NSOpenGLView
{
	id doc;

	struct spriteBank *backdrop;
	int x, y, w, h;
	float r,g,b;
	float z, zmul;
	
	int lit, litX, litY;
	
	int selection, selx1, sely1, selx2, sely2;
	
}
- (void) connectToDoc: (id) myDoc;
- (void) drawRect: (NSRect) bounds;
- (void) setFloorColour: (NSColor *) colour;
@end


@interface FloorDocument : NSDocument {
	
	IBOutlet FloorOpenGLView *floorView;	
	IBOutlet NSColorWell *floorColourWell;
	
	IBOutlet NSButton *modeButton1;
	IBOutlet NSButton *modeButton2;
	IBOutlet NSButton *modeButton3;
	IBOutlet NSButton *modeButton4;
	IBOutlet NSButton *modeButton5;

	struct spriteBank backdrop;
	struct polyList * firstPoly;
	
	int mode;
}
- (struct polyList *) getFloor ;

- (IBAction)loadBackdrop:(id)sender;
- (IBAction)setFloorColour:(id)sender;
- (IBAction)changeMode:(id)sender;

@end
