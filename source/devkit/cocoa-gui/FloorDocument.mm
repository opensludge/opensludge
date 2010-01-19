//
//  Floor.m
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2009-07-27.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <stdint.h>

#import "FloorDocument.h"

#import "floormaker.h"
#include "project.h"
#import "macstuff.h"

@implementation FloorDocument

- (id)init
{
    self = [super init];
    if (self) {		
		firstPoly = 0;
		noFloor (&firstPoly);
		
		backdrop.total=0;
		backdrop.type=2;
		if (!reserveSpritePal (&backdrop.myPalette, 0)) {
			[self release];
			return nil;
		}
    }
    return self;
}


- (NSString *)windowNibName
{
    return @"Floor";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
	[floorView connectToDoc: self];
	[floorColourWell setToolTip: @"Choose colour for floor outlines"];
	[modeButton1 setToolTip: @"Define floor borders"];
	[modeButton2 setToolTip: @"Move vertices"];
	[modeButton3 setToolTip: @"Delete vertices"];
	[modeButton4 setToolTip: @"Split line"];
	[modeButton5 setToolTip: @"Split floor"];

	if ([self fileURL]) 
		[self changeMode: modeButton2];
}
- (void)mouseMoved:(NSEvent *)theEvent
{
	fprintf (stderr, ".");
}

- (BOOL)readFromURL:(NSURL *)absoluteURL 
			 ofType:(NSString *)typeName 
			  error:(NSError **)outError
{
	if ([typeName isEqualToString:@"SLUDGE Floor"]) {	
#ifdef GNUSTEP
		GSNativeChar buffer[1024];
		if ([absoluteURL getFileSystemRepresentation:buffer maxLength:1023]) {
#else
		uint8_t buffer[1024];
		if (CFURLGetFileSystemRepresentation((CFURLRef) absoluteURL, true, buffer, 1023)) {
#endif
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
#ifdef GNUSTEP
		GSNativeChar buffer[1024];
		if ([absoluteURL getFileSystemRepresentation:buffer maxLength:1023]) {
#else		
		uint8_t buffer[1024];
		if (CFURLGetFileSystemRepresentation((CFURLRef) absoluteURL, true, buffer, 1023)) {
#endif
			if (saveFloorToFile ((char *) buffer, &firstPoly)) {
				[floorView setNeedsDisplay:YES];
				return YES;
			}
		}
	} 
	*outError = [NSError errorWithDomain:@"Error" code:1 userInfo:nil];
	return NO;
}

- (struct spriteBank *) getBackdrop {
	return &backdrop;
}
- (struct polyList *) getFloor {
	return firstPoly;
}
- (void) setFloor:(struct polyList *) floor
{
	firstPoly = floor;
}

- (IBAction)loadBackdrop:(id)sender
{
	NSString *path = nil;
	NSOpenPanel *openPanel = [ NSOpenPanel openPanel ];
	[openPanel setTitle:@"Load file as sprite"];
	NSArray *files = [NSArray arrayWithObjects:@"tga", @"png", nil];
	
	if ( [ openPanel runModalForDirectory:nil file:nil types:files] ) {
		path = [ openPanel filename ];
		bool success = 0;
		if ([[path pathExtension] isEqualToString: @"png"]) {
			success = loadSpriteFromPNG ((char *) [path UTF8String], &backdrop, 0);
		} else if ([[path pathExtension] isEqualToString: @"tga"]) {
			success = loadSpriteFromTGA ((char *) [path UTF8String], &backdrop, 0);
		} else {
			errorBox ("Can't load image", "I don't recognise the file type. TGA and PNG are the supported file types.");
		}
		if (success) {
			backdrop.sprites[0].height = -backdrop.sprites[0].height;
			[floorView setNeedsDisplay:YES];
			//[self updateChangeCount: NSChangeDone];
		}
	}	
}

- (IBAction)setFloorColour:(id)sender 
{
	[floorView setFloorColour: [[floorColourWell color] colorUsingColorSpaceName:@"NSDeviceRGBColorSpace"]];
}


- (void)close 
{
	if (backdrop.total) {
		forgetSpriteBank (&backdrop);
		backdrop.total = 0;
		noFloor (&firstPoly);
		deleteString (firstPoly);
	}
	[super close];
}

- (IBAction)changeMode:(id)sender
{
	switch (mode) {
		case 0:
			[modeButton1 setState:NO];
			break;
		case 1:
			[modeButton2 setState:NO];
			break;
		case 2:
			[modeButton3 setState:NO];
			break;
		case 4:
			[modeButton4 setState:NO];
			break;
		case 5:
			[modeButton5 setState:NO];
			break;
	}
	mode = [sender tag];
	switch (mode) {
		case 0:
			[modeButton1 setState:YES];
			break;
		case 1:
			[modeButton2 setState:YES];
			break;
		case 2:
			[modeButton3 setState:YES];
			break;
		case 4:
			[modeButton4 setState:YES];
			break;
		case 5:
			[modeButton5 setState:YES];
			break;
	}
	/*
	mode = [modeSelector indexOfSelectedItem];
	 */
}
- (int) mode
{
	return mode;
}

@end

@implementation FloorOpenGLView


- (void) connectToDoc: (id) myDoc
{
	doc = myDoc;
	backdrop = [doc getBackdrop];
}

- (void) setFloorColour: (NSColor *) colour
{
	r = [colour redComponent];
	g = [colour greenComponent];
	b = [colour blueComponent];
	[self setNeedsDisplay:YES];
}

- (void) setCoords {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(x*zmul, (x+w)*zmul, (-y+h)*zmul, -y*zmul, 1.0, -1.0);	
	glMatrixMode(GL_MODELVIEW);	
}

- (BOOL)acceptsFirstResponder { return YES; }
- (BOOL)resignFirstResponder{ return NO; }

- (void)mouseMoved:(NSEvent *)theEvent
{
	NSPoint local_point = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	litX = (local_point.x+x)*zmul;
	litY = -(local_point.y+y-h)*zmul;
	if (lit != snapToClosest(&litX, &litY, [doc getFloor])){
		lit = !lit;
		[self setNeedsDisplay:YES];
	}
}

- (void)mouseDown:(NSEvent *)theEvent {
    BOOL keepOn = YES;
	int i, xx, yy;
    NSPoint mouseLoc;
	NSPoint mouseLoc1;
	NSPoint local_point;
	
	mouseLoc1 = [theEvent locationInWindow];

	unsigned int flags = [theEvent modifierFlags];
	if (flags & NSCommandKeyMask || flags & NSControlKeyMask) {
		local_point = [self convertPoint:mouseLoc1 fromView:nil];
		xx = (local_point.x+x)*zmul;
		yy = -(local_point.y+y-h)*zmul;
		
		switch ([doc mode]) {
		case 0: // Define floor border
			snapToClosest(&xx, &yy, [doc getFloor]);
			while (keepOn) {
				keepOn = false;
				i = addVertex (xx, yy, [doc getFloor]);
				switch (i) {
					case 0:
						errorBox ("Can't add vertex", "Out of memory.");
						return;
						
					case 3:
						errorBox ("Can't add vertex", "That vertex is already used in this polygon, but isn't the start point.");
						return;
						
					case 2:
						if (NSRunAlertPanel (@"Can't add vertex", @"Can't add another vertex as the floor is already complete... do you want to start a NEW polygon at this point?", @"Yes", @"No", NULL) == NSAlertDefaultReturn) { 
							[doc setFloor:addPoly ([doc getFloor])];
							keepOn = true;
						} else {
							return;
						}
						break;
				}
			}
			break;
		case 1: // Move vertices
			if (! snapToClosest(&xx, &yy, [doc getFloor]))
				return;
			
			selx1 = xx;
			sely1 = yy;
			selection = 1;
			while (keepOn) {
				theEvent = [[self window] nextEventMatchingMask: NSLeftMouseUpMask | NSLeftMouseDraggedMask];
				mouseLoc = [theEvent locationInWindow];
				local_point = [self convertPoint:mouseLoc fromView:nil];
				
				switch ([theEvent type]) {
					case NSLeftMouseUp:
						keepOn = NO;
						// continue
					case NSLeftMouseDragged:
						selx2 = xx = (local_point.x+x)*zmul;
						sely2 = yy = -(local_point.y+y-h)*zmul;
						
						lit = snapToClosest(&xx, &yy, [doc getFloor]);
						litX = xx; 
						litY = yy;
						
						if (lit && (xx != selx1 || yy != sely1)) {
							selx2 = xx;
							sely2 = yy;
						}
						
						[self setNeedsDisplay:YES];
						
						break;
					default:
						/* Ignore any other kind of event. */
						break;
				}
			}
			selection = 0;
			if (! moveVertices (selx1, sely1, selx2, sely2, [doc getFloor])) {
				errorBox ("Can't move vertex", "Sorry - that vertex is already contained in one or more of the polygons you're changing.");
				return;
			}
				
			break;
		case 2: // Remove vertices
			{
			if (! snapToClosest(&xx, &yy, [doc getFloor]))
				return;
			struct polyList * firstPoly = [doc getFloor];
			
			killVertex (xx, yy, &firstPoly);
			[doc setFloor: firstPoly];
			break;
			}
		case 4: // Split lines
		case 5: // Split segments
			if (! snapToClosest(&xx, &yy, [doc getFloor]))
				return;
			
			selx1 = xx;
			sely1 = yy;
			selection = 1;
			while (keepOn) {
				theEvent = [[self window] nextEventMatchingMask: NSLeftMouseUpMask | NSLeftMouseDraggedMask];
				mouseLoc = [theEvent locationInWindow];
				local_point = [self convertPoint:mouseLoc fromView:nil];
				
				switch ([theEvent type]) {
					case NSLeftMouseUp:
						keepOn = NO;
						// continue
					case NSLeftMouseDragged:
						xx = (local_point.x+x)*zmul;
						yy = -(local_point.y+y-h)*zmul;
						
						lit = snapToClosest(&xx, &yy, [doc getFloor]);
						selx2 = litX = xx; 
						sely2 = litY = yy;
						
						[self setNeedsDisplay:YES];
						
						break;
					default:
						/* Ignore any other kind of event. */
						break;
				}
			}
			selection = 0;
			
			if ([doc mode] == 4)
				splitLine (selx1, sely1, selx2, sely2, [doc getFloor]);
			else {
				struct polyList * firstPoly = [doc getFloor];
				splitPoly (selx1, sely1, selx2, sely2, &firstPoly);
				[doc setFloor: firstPoly];
			}
			break;
		}
		lit = snapToClosest(&xx, &yy, [doc getFloor]);
		litX = xx; litY = yy;
		[self setNeedsDisplay:YES];
		[doc updateChangeCount: NSChangeDone];	
	} else {
	
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
					x = x1 + (mouseLoc1.x - mouseLoc.x);
					y = y1 - (mouseLoc.y - mouseLoc1.y);
					[self setCoords];
					[self setNeedsDisplay:YES];

					break;
				default:
					/* Ignore any other kind of event. */
					break;
			}
		}
	}
	
    return;
}

- (void)scrollWheel:(NSEvent *)theEvent
{
	if ([theEvent deltaY]) {
		double x1, y1;
		NSPoint mouseLoc1 = [theEvent locationInWindow];
		NSPoint local_point = [self convertPoint:mouseLoc1 fromView:nil];
		x1 = zmul*(local_point.x+x);
		y1 = -zmul*(local_point.y+y-h);
	
		z += [theEvent deltaY];
		if (z > 200.0) z = 200.0;
		if (z < -16.0) z = -16.0;
	
		zmul = (1.0+z/20);

		x = -(local_point.x)+x1/zmul;
		y = -(local_point.y-h)-y1/zmul;
	}
		
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
	if (! backdrop->total)
		addSprite(0, backdrop);
	z = 1.0;
	//[doc setFloorColour:nil];
	r = g = b = 1.0;
	zmul = (1.0+z/20);
	[self setCoords];
	[[self window] setAcceptsMouseMovedEvents:YES];
	[[self window] makeFirstResponder: self];
		
}

- (void)reshape {
	NSRect bounds = [self bounds];
	if (! w) x = 0;
	else x -= (bounds.size.width-w)/2;
	if (! h) y = 0;
	else y += (bounds.size.height-h)/2;
	h = bounds.size.height;
	w = bounds.size.width;
	glViewport (bounds.origin.x, bounds.origin.y, w, h);	
	
	[self setCoords];
}

-(void) drawRect: (NSRect) bounds
{	
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
	
	glColor3f(1.0f, 1.00f, 1.00f);
	pasteSprite (&backdrop->sprites[0], &backdrop->myPalette, false);

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

	if (lit) {
		glColor3f(1.0f, 0.00f, 1.00f);
		glBegin(GL_QUADS);
		{
			glVertex3i(  litX-8,  litY-8, 0);
			glVertex3i(  litX+8,  litY-8, 0);
			glVertex3i(  litX+8,  litY+8, 0);
			glVertex3i(  litX-8,  litY+8, 0);
		}
		glEnd();
	}
	
	drawFloor ([doc getFloor], r, g, b);
	
	if (selection == 1) {
		glColor3f(1.0f, 0.00f, 1.00f);
		glBegin(GL_LINES);
		{
			glVertex3i(selx1, sely1, 0);
			glVertex3i(selx2, sely2, 0);
		}
		glEnd();
	}
	
	glFlush();
}


@end
