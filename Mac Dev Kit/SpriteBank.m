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
		sprites.total=0;
		if (!reserveSpritePal (&sprites.myPalette, 0)) {
			[self release];
			return nil;
		}
    }
    return self;
}

- (NSString *)windowNibName
{
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers,
	// you should remove this method and override -makeWindowControllers instead.
    return @"SpriteBank";
}

- (void)setupButtons
{
	if (sprites.total < 256) {
		[spriteIndexSlider setMaxValue:(double) sprites.total];
		[spriteIndexSlider setNumberOfTickMarks:sprites.total+1];
		[insertButton setEnabled:YES];
	} else {
		[spriteIndexSlider setMaxValue:255];
		[spriteIndexSlider setNumberOfTickMarks:256];
		[insertButton setEnabled:NO];
	}
	if ([spriteView spriteIndex] < sprites.total) {
		[deleteButton setEnabled:YES];
		[exportButton setEnabled:YES];
		[replaceButton setEnabled:YES];
	} else {
		[deleteButton setEnabled:NO];
		[exportButton setEnabled:NO];
		[replaceButton setEnabled:NO];
	}
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
    // Add any code here that needs to be executed once the windowController has loaded the document's window.
	[spriteView connectToDoc: self];
	[self setupButtons];
	fprintf (stderr, "Sprite bank type: %d", sprites.type);
	if (sprites.total) {
		[spriteIndexSlider setEnabled:YES];
		if (sprites.type < 2) {
			sprites.type = 1;
			[palMode selectCellWithTag:1];
		} else {
			[palMode selectCellWithTag:2];
			[[palMode cellWithTag: 0] setEnabled:NO];
			[[palMode cellWithTag: 1] setEnabled:NO];
		}
	} else {
		[palMode selectCellWithTag:0];
		[[palMode cellWithTag: 1] setEnabled:NO];
	}
}

- (BOOL)readFromData:(NSData *) data
			  ofType:(NSString *)typeName
			   error:(NSError **)outError
{
	if ([typeName isEqualToString:@"SLUDGE Sprite Bank"]) {		
		if (loadSpriteBank ((unsigned char *) [data bytes], &sprites))
		{
			return YES;
		}
	} 
	*outError = [NSError errorWithDomain:@"Error" code:1 userInfo:nil];
	return NO;
}


- (BOOL)writeToURL:(NSURL *)absoluteURL 
			ofType:(NSString *)typeName 
			 error:(NSError **)outError
{
	if ([typeName isEqualToString:@"SLUDGE Sprite Bank"]) {		
		UInt8 buffer[1024];
		if (CFURLGetFileSystemRepresentation((CFURLRef) absoluteURL, true, buffer, 1023)) {
			if (saveSpriteBank ((char *) buffer, &sprites)) {
				return YES;
			}
		}
	} 
	
	*outError = [NSError errorWithDomain:@"Error" code:1 userInfo:nil];
	return NO;
}

- (struct spriteBank *) getSprites {
	return &sprites;
}

- (int) hotSpotX
{
	return hotSpotX;
}
- (void) setHotSpotX:(int)i
{
	hotSpotX = i;
	if (([spriteView spriteIndex] < sprites.total) && (i != sprites.sprites[[spriteView spriteIndex]].xhot)) {
		sprites.sprites[[spriteView spriteIndex]].xhot = i;
		[spriteView setNeedsDisplay:YES];
		[self updateChangeCount: NSChangeDone];
	}
}
- (int) hotSpotY
{
	return hotSpotY;
}
- (void) setHotSpotY:(int)i
{
	hotSpotY = i;
	if (([spriteView spriteIndex] < sprites.total) && (i != sprites.sprites[[spriteView spriteIndex]].yhot)) {
		sprites.sprites[[spriteView spriteIndex]].yhot = i;
		[spriteView setNeedsDisplay:YES];
		[self updateChangeCount: NSChangeDone];
	}
}

- (IBAction)hotSpotCentre:(id)sender
{
	if (sprites.total) {
		[self setHotSpotX: sprites.sprites[[spriteView spriteIndex]].width / 2];
		[self setHotSpotY: sprites.sprites[[spriteView spriteIndex]].height / 2];
	}
}
- (IBAction)hotSpotBase:(id)sender{
	if (sprites.total) {
		[self setHotSpotX: sprites.sprites[[spriteView spriteIndex]].width / 2];
		[self setHotSpotY: sprites.sprites[[spriteView spriteIndex]].height-1];
	}
}

- (IBAction)setModePalOpen:(id)sender
{
	sprites.type = 0;
}
- (IBAction)setModePalClosed:(id)sender
{
	sprites.type = 1;
}
- (IBAction)setModePalNone:(id)sender
{
	if (sprites.total && sprites.type < 2) {
		if (! NSRunAlertPanel (@"Convert sprite bank?", @"This will convert your entire sprite bank to 32-bit colour mode. This action can not be reversed. Do you want to proceed?", @"Yes", @"No", NULL) == NSAlertDefaultReturn) {
			[palMode selectCellWithTag:sprites.type];
			return;
		}
		// Convert sprite bank to 32-bit

		[[palMode cellWithTag: 0] setEnabled:NO];
		[[palMode cellWithTag: 1] setEnabled:NO];
	}
	sprites.type = 2;	
}
- (IBAction)insertSprite:(id)sender
{
	NSString *path = nil;
	NSOpenPanel *openPanel = [ NSOpenPanel openPanel ];
	[openPanel setTitle:@"Load TGA file as sprite"];
	[openPanel setRequiredFileType:@"tga"];
	
	if ( [ openPanel runModalForDirectory:nil file:nil ] ) {
		path = [ openPanel filename ];
		addSprite ([spriteView spriteIndex], &sprites);
		if (! loadSpriteFromTGA ((char *) [path UTF8String], &sprites, [spriteView spriteIndex])) {
			deleteSprite ([spriteView spriteIndex], &sprites);
		} else {
			[self setHotSpotX: sprites.sprites[[spriteView spriteIndex]].width / 2];
			[self setHotSpotY: sprites.sprites[[spriteView spriteIndex]].height / 2];
			[self updateChangeCount: NSChangeDone];
			[spriteIndexSlider setEnabled:YES];
			if (sprites.type < 2) {
				[[palMode cellWithTag: 1] setEnabled:YES];
			} else {
				[[palMode cellWithTag: 0] setEnabled:NO];
				[[palMode cellWithTag: 1] setEnabled:NO];
			}
		}
		[spriteView setSpriteIndex:[spriteView spriteIndex]];
		[spriteView setNeedsDisplay:YES];
	}	

}
- (IBAction)replaceSprite:(id)sender
{
	NSString *path = nil;
	NSOpenPanel *openPanel = [ NSOpenPanel openPanel ];
	[openPanel setTitle:@"Load TGA file as sprite"];
	[openPanel setRequiredFileType:@"tga"];
	
	if ( [ openPanel runModalForDirectory:nil file:nil ] ) {
		path = [ openPanel filename ];
		loadSpriteFromTGA ((char *) [path UTF8String], &sprites, [spriteView spriteIndex]);
		[spriteView setNeedsDisplay:YES];
		[self updateChangeCount: NSChangeDone];
	}	
}

- (IBAction)deleteSprite:(id)sender
{
	if (! NSRunAlertPanel (@"Delete this sprite?", @"Are you sure?", @"Yes", @"No", NULL) == NSAlertDefaultReturn) 
		return;
	deleteSprite ([spriteView spriteIndex], &sprites);
	[spriteView setSpriteIndex:[spriteView spriteIndex]];
	if (! sprites.total)
		[spriteIndexSlider setEnabled:NO];
	[spriteView setNeedsDisplay:YES];
	[self updateChangeCount: NSChangeDone];
}
- (IBAction)exportSprite:(id)sender
{
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
	// Validation shouldn't be done here, but I'm cheating.
	if (i > sprites->total) i = sprites->total;
	if (i > 255) i = 255;
	else if (i<0) i = 0;
	spriteIndex = i;
	
	if (i < sprites->total) {
		[doc setHotSpotX:sprites->sprites[i].xhot];
		[doc setHotSpotY:sprites->sprites[i].yhot];
	} else {
		[doc setHotSpotX:0];
		[doc setHotSpotY:0];
	}
	[doc setupButtons];
}

- (void) connectToDoc: (id) myDoc
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
	[self setSpriteIndex:0];
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
	
	if (spriteIndex < sprites->total)
		pasteSprite (&sprites->sprites[spriteIndex], &sprites->myPalette, showBox);

    glFlush();
}


@end
