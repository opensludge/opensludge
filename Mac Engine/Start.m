//
//  Start.m
//  OpenSludge
//
//	The startup-option window. (OS X version)
//
//  Created by Rikard Peterson on 2009-06-29.
//

#import "Start.h"
#include "Language.h"

@interface SDLApplication : NSApplication
@end

extern char **languageName;


@implementation StartController

- (id)init
{
	if (![super initWithWindowNibName:@"Startup"])
		return nil;
	
	return self;
}

extern unsigned char * gameLogo;

// Load all the options, and check the boxes that reflect the current settings
- (void)windowDidLoad
{
	if (gameLogo) {
		NSImage * l = [[NSImage alloc] initWithSize: NSMakeSize(310, 88)];
		NSBitmapImageRep *imrep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes: &gameLogo 
																		  pixelsWide:310 
																		  pixelsHigh:88 
																	   bitsPerSample:8 
																	 samplesPerPixel:4 
																			hasAlpha:YES 
																			isPlanar:NO 
																	  colorSpaceName:NSDeviceRGBColorSpace 
																		 bytesPerRow:1240 
																		bitsPerPixel:32];
		[l addRepresentation: imrep];
		[logo setImage: l];
		[l release];
		[imrep release];
	}
	[fullScreenCheck setState:gameSettings.userFullScreen != 0];
	
	[aaCheck selectItemWithTag: gameSettings.antiAlias];
	
	[languageList removeAllItems];
	if (gameSettings.numLanguages) {
		for (unsigned int i=0; i<= gameSettings.numLanguages; i++) {
			if (languageName[i]) { 
				[languageList addItemWithTitle:[NSString stringWithUTF8String:languageName[i]]];	
			} else {
				[languageList addItemWithTitle:[NSString stringWithFormat:@"Language %d", i]];	
			}
		}
		[languageList selectItemAtIndex: getLanguageForFileB()];
	} else {
		[languageList addItemWithTitle:@"No translations available"];			
		[languageList setEnabled:NO];
	}
}

extern int *languageTable;

-(IBAction)okButton:(id)sender
{
	gameSettings.userFullScreen = [fullScreenCheck state];
	gameSettings.antiAlias = [[aaCheck selectedItem] tag];
	if (gameSettings.numLanguages) {
		gameSettings.languageID = [languageList indexOfSelectedItem];
		if (gameSettings.languageID > gameSettings.numLanguages) gameSettings.languageID = 0;
		gameSettings.languageID = languageTable[gameSettings.languageID];
	}
	[[SDLApplication sharedApplication] stopModalWithCode:1];
	[self close];
}
-(IBAction)cancelButton:(id)sender
{
	[[SDLApplication sharedApplication] stopModalWithCode:0];
	[self close];
}

@end
