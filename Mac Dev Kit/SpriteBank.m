//
//  SpriteBank.m
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2009-07-18.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "SpriteBank.h"


@implementation SpriteBank

- (id)init
{
    self = [super init];
    if (self) {
		
        // Add your subclass-specific initialization here.
        // If an error occurs here, send a [self release] message and return nil.
		
    }
    return self;
}

- (NSString *)windowNibName
{
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers,
	// you should remove this method and override -makeWindowControllers instead.
    return @"SpriteBank";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
    // Add any code here that needs to be executed once the windowController has loaded the document's window.
	
}

- (BOOL)readFromData:(NSData *) data
			  ofType:(NSString *)typeName
			   error:(NSError **)outError
{
	fprintf (stderr, "Loading: %s\n", [typeName UTF8String]);
	if ([typeName isEqualToString:@"SLUDGE Sprite Bank"]) {
		return YES;
	} else {
		*outError = [NSError errorWithDomain:@"Error" code:1 userInfo:nil];
		return NO;
	}
}

- (NSData *)dataOfType:(NSString *)aType
				 error:(NSError **)e
{
	*e = [NSError errorWithDomain:@"Error" code:1 userInfo:nil];
	return nil;
}

@end

@implementation SpriteOpenGLView

static void drawAnObject ()
{
    glColor3f(1.0f, 0.85f, 0.35f);
    glBegin(GL_TRIANGLES);
    {
        glVertex3f(  20,  26, 0.0);
        glVertex3f(  18,  17, 0.0);
        glVertex3f(  22,  17 ,0.0);
    }
    glEnd();
}

/*
- (void)prepareOpenGL 
{
}
*/

- (void)reshape {
	NSRect bounds = [self bounds];
	glViewport (bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);	

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, bounds.size.width, 0, bounds.size.height, 1.0, -1.0);	
	glMatrixMode(GL_MODELVIEW);

}

-(void) drawRect: (NSRect) bounds
{
	
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    drawAnObject();
    glFlush();
}


@end
