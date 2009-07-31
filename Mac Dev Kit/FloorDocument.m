//
//  Floor.m
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2009-07-27.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "FloorDocument.h"

#import "FloorMaker.h"

@implementation FloorDocument

- (id)init
{
    self = [super init];
    if (self) {		
        // Add your subclass-specific initialization here.
        // If an error occurs here, send a [self release] message and return nil.
		firstPoly = 0;
		noFloor (&firstPoly);
    }
    return self;
}

- (NSString *)windowNibName
{
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers,
	// you should remove this method and override -makeWindowControllers instead.
    return @"Floor";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
	[floorView connectToDoc: self];
}

- (BOOL)readFromURL:(NSURL *)absoluteURL 
			 ofType:(NSString *)typeName 
			  error:(NSError **)outError
{
	if ([typeName isEqualToString:@"SLUDGE Floor"]) {	
		UInt8 buffer[1024];
		if (CFURLGetFileSystemRepresentation((CFURLRef) absoluteURL, true, buffer, 1023)) {
			if (loadFloorFromFile ((char *) buffer, &firstPoly)) {
				return YES;
			}
		}
	} 
	*outError = [NSError errorWithDomain:@"Error" code:1 userInfo:nil];
	return NO;
}

- (BOOL)writeToURL:(NSURL *)absoluteURL 
			ofType:(NSString *)typeName 
			 error:(NSError **)outError
{
	if ([typeName isEqualToString:@"SLUDGE Floor"]) {		
		UInt8 buffer[1024];
		if (CFURLGetFileSystemRepresentation((CFURLRef) absoluteURL, true, buffer, 1023)) {
			if (saveFloorToFile ((char *) buffer, firstPoly)) {
				return YES;
			}
		}
	} 
	*outError = [NSError errorWithDomain:@"Error" code:1 userInfo:nil];
	return NO;
}

- (struct polyList *) getFloor {
	return firstPoly;
}


@end

@implementation FloorOpenGLView


- (void) connectToDoc: (id) myDoc
{
	doc = myDoc;
}

- (void) setCoords {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(x+(-w/2)*zmul, x+(w/2)*zmul, y+(h/2)*zmul, y+(-h/2)*zmul, 1.0, -1.0);	
	glMatrixMode(GL_MODELVIEW);	
}

- (void)mouseDown:(NSEvent *)theEvent {
    BOOL keepOn = YES;
    NSPoint mouseLoc;
	NSPoint mouseLoc1;
	
	mouseLoc1 = [theEvent locationInWindow];

	int x1 = x;
	int y1 = y;
	
    while (keepOn) {
        theEvent = [[self window] nextEventMatchingMask: NSLeftMouseUpMask |
			NSLeftMouseDraggedMask];
        mouseLoc = [theEvent locationInWindow];
		
        switch ([theEvent type]) {
            case NSLeftMouseUp:
				keepOn = NO;
				// continue
            case NSLeftMouseDragged:
				x = x1 + (mouseLoc1.x - mouseLoc.x)*zmul;
				y = y1 + (mouseLoc.y - mouseLoc1.y)*zmul;
				[self setCoords];
				[self setNeedsDisplay:YES];

				break;
            default:
				/* Ignore any other kind of event. */
				break;
        }
    };
	
    return;
}

- (void)scrollWheel:(NSEvent *)theEvent
{

	z += [theEvent deltaY];
	if (z > 200.0) z = 200.0;
	if (z < -16.0) z = -16.0;
	
	zmul = (1.0+z/20);
	
	/*	
	if ([theEvent deltaX]<0.0)
		[self setValue:[NSNumber numberWithInt:spriteIndex+1] forKey:@"spriteIndex"];
	if ([theEvent deltaX]>0.0)
		[self setValue:[NSNumber numberWithInt:spriteIndex-1] forKey:@"spriteIndex"];
*/	
	[self setCoords];
	[self setNeedsDisplay:YES];
}

				
- (void)prepareOpenGL 
{
	z = 1.0;
	zmul = (1.0+z/20);
	[self setCoords];
}

- (void)reshape {
	NSRect bounds = [self bounds];
	if (! w) x = bounds.size.width / 2;
	if (! h) y = bounds.size.height / 2;
	h = bounds.size.height;
	w = bounds.size.width;
	glViewport (bounds.origin.x, bounds.origin.y, w, h);	
	
	[self setCoords];
}

-(void) drawRect: (NSRect) bounds
{	
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
	
	glDisable (GL_TEXTURE_2D);
    glColor3f(1.0f, 0.35f, 0.35f);
    glBegin(GL_LINES);
    {
        glVertex3f(  0.0,  0.0, 0.0);
        glVertex3f(  1000.0,  0.0, 0.0);
        glVertex3f(  0.0, 0.0, 0.0);
        glVertex3f(  0.0, 1000.0, 0.0);
    }
    glEnd();
	
	glColor3f(1.0f, 1.0f, 1.0f);
	
	drawFloor ([doc getFloor]);
	
	glFlush();
}


@end
