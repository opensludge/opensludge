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
		sprites.total=0;
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
	[spriteView connectToDoc: self];
	if (sprites.total) {
		[spriteIndexSlider setMaxValue:(double) sprites.total-1];
		[spriteIndexSlider setNumberOfTickMarks:sprites.total];
		[spriteIndexSlider setEnabled:YES];
	}
}

- (BOOL)readFromData:(NSData *) data
			  ofType:(NSString *)typeName
			   error:(NSError **)outError
{
	fprintf (stderr, "Loading: %s\n", [typeName UTF8String]);
	if ([typeName isEqualToString:@"SLUDGE Sprite Bank"]) {		
		if (loadSpriteBank ((unsigned char *) [data bytes], &sprites))
		{
			return YES;
		}
	} 
	*outError = [NSError errorWithDomain:@"Error" code:1 userInfo:nil];
	return NO;
}

- (NSData *)dataOfType:(NSString *)aType
				 error:(NSError **)e
{
	*e = [NSError errorWithDomain:@"Error" code:1 userInfo:nil];
	return nil;
}

- (struct spriteBank *) getSprites {
	return &sprites;
}


@end

@implementation SpriteOpenGLView

- (bool) showBox
{
	return spriteIndex;
}
- (void) setShowBox:(bool)i
{
	showBox = i;
	[self setNeedsDisplay:YES];
}


- (int) spriteIndex
{
	return spriteIndex;
}
- (void) setSpriteIndex:(int)i
{
	if (i >= sprites->total) i = sprites->total-1;
	if (i<0) i = 0;
	spriteIndex = i;
	[self setNeedsDisplay:YES];
}

- (void) connectToDoc: (SpriteBank *) myDoc
{
	doc = myDoc;
	sprites = [doc getSprites];
	spriteIndex = 0;
}

- (void) setCoords {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho((-w/2+x)*(1.0+z/20), (w/2+x)*(1.0+z/20), (-h/2+y)*(1.0+z/20), (h/2+y)*(1.0+z/20), 1.0, -1.0);	
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
				x = x1 + mouseLoc1.x - mouseLoc.x;
				y = y1 + mouseLoc1.y - mouseLoc.y;
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
	
	if ([theEvent deltaX]<0.0)
		[self setValue:[NSNumber numberWithInt:spriteIndex+1] forKey:@"spriteIndex"];
	if ([theEvent deltaX]>0.0)
		[self setValue:[NSNumber numberWithInt:spriteIndex-1] forKey:@"spriteIndex"];
	
	[self setCoords];
	[self setNeedsDisplay:YES];
}

				
- (void)prepareOpenGL 
{
	showBox = false;
	if (sprites->total)
		loadSpriteTextures (sprites);
}

- (void)reshape {
	NSRect bounds = [self bounds];
	if (w) 
		x = bounds.size.width * x / w;
	else 
		x = 0;
	if (h) 
		y = bounds.size.height * y / h;
	else
		y =  bounds.size.height / 4;
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
        glVertex3f(  -1000.0,  0.0, 0.0);
        glVertex3f(  1000.0,  0.0, 0.0);
        glVertex3f(  0.0, -1000.0, 0.0);
        glVertex3f(  0.0, 1000.0, 0.0);
    }
    glEnd();
	
    glColor3f(1.0f, 1.0f, 1.0f);
	
	if (sprites->total)
		pasteSprite (&sprites->sprites[spriteIndex], &sprites->myPalette, showBox);

    glFlush();
}


@end
