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

@interface FloorOpenGLView : NSOpenGLView
{
	id doc;

	int x, y, w, h;
	float z, zmul;
	
}
- (void) connectToDoc: (id) myDoc;
- (void) drawRect: (NSRect) bounds ;
@end


@interface FloorDocument : NSDocument {
	
	IBOutlet FloorOpenGLView *floorView;

	struct polyList * firstPoly;
	
}
- (struct polyList *) getFloor ;

@end
