//
//  Floor.h
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2009-07-27.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

//#include <OpenGL/gl.h>
#include "glee.h"
#import <Cocoa/Cocoa.h>
#import "SLUDGE Document.h"

#include "FloorMaker.h"
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


@interface FloorDocument : SLUDGE_Document {
	
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
