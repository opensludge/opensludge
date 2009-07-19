//
//  SpriteBank.h
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2009-07-18.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#include <OpenGL/gl.h>


@interface SpriteBank : NSDocument {
	
	IBOutlet NSOpenGLView *spriteView;

}

@end

@interface SpriteOpenGLView : NSOpenGLView
{
}
- (void) drawRect: (NSRect) bounds ;
@end
