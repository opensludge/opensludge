//
//  zDocument.m
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2009-08-07.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <stdint.h>

#import "zDocument.h"
#include "zbuffer.h"

@implementation zDocument

- (id)init
{
    self = [super init];
    if (self) {		
        // Add your subclass-specific initialization here.
        // If an error occurs here, send a [self release] message and return nil.
		buffer = 1;

		backdrop.total=0;
		backdrop.type=2;
		if (!reserveSpritePal (&backdrop.myPalette, 0)) {
			[self release];
			return nil;
		}
		
    }
    return self;
}

- (NSString *)windowNibName {
	if (! [self fileURL]) {
		NSString *path = nil;
		NSOpenPanel *openPanel = [ NSOpenPanel openPanel ];
		[openPanel setTitle:@"Load file to zBuffer"];
		NSArray *files = [NSArray arrayWithObjects:@"tga", nil];
		
		if ( [ openPanel runModalForDirectory:nil file:nil types:files] ) {
			path = [ openPanel filename ];
			bool success = loadZBufferFromTGA ((char *) [path UTF8String], &backdrop);
			if (! success) {
				[self close];
				return nil;
			}
			[self updateChangeCount: NSChangeDone];
		} else {
			[self close];
			return nil;
		}
	}
	
    return @"zDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{	
    [super windowControllerDidLoadNib:aController];
	[zView connectToDoc: self];
	[zBufSlider setMaxValue:backdrop.total-1];
	[zBufSlider setNumberOfTickMarks:backdrop.total-1];
	[numBuffers setIntValue:backdrop.total-1];
	[self setBuffer:1];
}

- (BOOL)readFromURL:(NSURL *)absoluteURL 
			 ofType:(NSString *)typeName 
			  error:(NSError **)outError
{
	if ([typeName isEqualToString:@"SLUDGE zBuffer"]) {
#ifdef GNUSTEP
		GSNativeChar path[1024];
		if ([[absoluteURL absoluteString] getFileSystemRepresentation:path maxLength:1023]) {
#else		
		uint8_t path[1024];
		if (CFURLGetFileSystemRepresentation((CFURLRef) absoluteURL, true, path, 1023)) {
#endif
			if (loadZBufferFile ((char *) path, &backdrop)) {
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
	if ([typeName isEqualToString:@"SLUDGE zBuffer"]) {
#ifdef GNUSTEP
		GSNativeChar path[1024];
		if ([[absoluteURL absoluteString] getFileSystemRepresentation:path maxLength:1023]) {
#else		
		uint8_t path[1024];
		if (CFURLGetFileSystemRepresentation((CFURLRef) absoluteURL, true, path, 1023)) {
#endif
			if (saveZBufferFile ((char *) path, &backdrop)) {
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

- (int) buffer
{
	return buffer;
}

- (void) setBuffer:(int)i
{
	// Validation shouldn't be done here, but I'm cheating.
	if (i < 1) i = 1;
	if (i > backdrop.total-1) i = backdrop.total-1;
	if (i > 0)
		[bufferYTextField setEnabled:YES];
	else
		[bufferYTextField setEnabled:NO];
	buffer = i;
	[self setBufferY: backdrop.sprites[i].tex_x];
}

- (void)setBufferY:(int)i
{
	if (buffer && i != backdrop.sprites[buffer].tex_x) {
		backdrop.sprites[buffer].tex_x = i;
		[self updateChangeCount: NSChangeDone];
	}
	[zView setNeedsDisplay:YES];
}
- (int)bufferY
{
	if (! buffer) return 0;
	return backdrop.sprites[buffer].tex_x;
}


@end

@implementation zOpenGLView

- (void) connectToDoc: (id) myDoc
{
	doc = myDoc;
	backdrop = [doc getBackdrop];
}

- (void) setCoords {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrtho(x+(-w/2)*zmul, x+(w/2)*zmul, y+(h/2)*zmul, y+(-h/2)*zmul, 1.0, -1.0);	
	glOrtho((x-w/2)*zmul, (x+w/2)*zmul, (-y+h/2)*zmul, (-y-h/2)*zmul, 1.0, -1.0);	
	glMatrixMode(GL_MODELVIEW);	
}

- (void)mouseDown:(NSEvent *)theEvent {
    BOOL keepOn = YES;
    NSPoint mouseLoc;
	NSPoint mouseLoc1;
	
	mouseLoc1 = [theEvent locationInWindow];
	unsigned int flags = [theEvent modifierFlags];
	if (flags & NSCommandKeyMask || flags & NSControlKeyMask) {
		int b = [doc buffer];
		if (b<1) return;
		
		int y1 =  backdrop->sprites[b].tex_x;
		
		while (keepOn) {
			theEvent = [[self window] nextEventMatchingMask: NSLeftMouseUpMask | NSLeftMouseDraggedMask];
			mouseLoc = [theEvent locationInWindow];
			
			switch ([theEvent type]) {
				case NSLeftMouseUp:
					keepOn = NO;
					// continue
				case NSLeftMouseDragged:
					[doc setBufferY: y1 - (mouseLoc.y - mouseLoc1.y)*zmul];
					
					break;
				default:
					/* Ignore any other kind of event. */
					break;
			}
		};
	} else {
		int x1 = x;
		int y1 = y;
		
		while (keepOn) {
			theEvent = [[self window] nextEventMatchingMask: NSLeftMouseUpMask | NSLeftMouseDraggedMask];
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
		};
	}
    return;
}

- (void)scrollWheel:(NSEvent *)theEvent
{
	
	if ([theEvent deltaY]) {
		double x1, y1;
		NSPoint mouseLoc1 = [theEvent locationInWindow];
		NSPoint local_point = [self convertPoint:mouseLoc1 fromView:nil];
		x1 = zmul*(local_point.x-w/2+x);
		y1 = -zmul*(local_point.y-h/2+y);
		
		z += [theEvent deltaY];
		if (z > 200.0) z = 200.0;
		if (z < -16.0) z = -16.0;
		
		zmul = (1.0+z/20);
		
		x = -(local_point.x-w/2)+x1/zmul;
		y = -(local_point.y-h/2)-y1/zmul;
	}
	
	if ([theEvent deltaX]<0.0)
		[doc setBuffer: [doc buffer]+1];
	if ([theEvent deltaX]>0.0)
		[doc setBuffer: [doc buffer]-1];

	[self setCoords];
	[self setNeedsDisplay:YES];
}


- (void)prepareOpenGL 
{
	if (! backdrop->total) {
		addSprite(0, backdrop);
		backdrop->sprites[0].width = 640;
		backdrop->sprites[0].height = 480;
	} else
		loadZTextures (backdrop);

	z = 1.0;
	zmul = (1.0+z/20);
	[self setCoords];
}

- (void)reshape {
	NSRect bounds = [self bounds];
	if (! w) x = bounds.size.width / 2;
	if (! h) y = -bounds.size.height / 2;
	h = bounds.size.height;
	w = bounds.size.width;
	glViewport (bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);	
	
	[self setCoords];
}

-(void) drawRect: (NSRect) bounds
{		
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
	
	int i;
	int b = [doc buffer];
	
	if (backdrop->total>1)
		for (i=1; i< backdrop->total; i++) {
			if (i == b) {
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			} else 
				glColor4f(0.2f, 0.0f, 0.5f, 1.0f);
			
			pasteSprite (&backdrop->sprites[i], &backdrop->myPalette, false);
		}
	
	glDisable (GL_TEXTURE_2D);
	
		
    glColor3f(1.0f, 0.35f, 0.35f);
    glBegin(GL_LINE_LOOP);
    {
        glVertex3f(  0.0,  0.0, 0.0);
        glVertex3f(  backdrop->sprites[0].width,  0.0, 0.0);
        glVertex3f(  backdrop->sprites[0].width, -backdrop->sprites[0].height, 0.0);
        glVertex3f(  0.0, -backdrop->sprites[0].height, 0.0);
    }
    glEnd();

	if (b>0) {
		glColor4f(0.0f, 1.0f, 0.0f, 0.7f);
		glBegin(GL_LINES);
		{
			glVertex3f(  0.0,  backdrop->sprites[b].tex_x, 0.0);
			glVertex3f(  backdrop->sprites[0].width,  backdrop->sprites[b].tex_x, 0.0);
		}
		glEnd();
	}
	
	glFlush();
}


@end
