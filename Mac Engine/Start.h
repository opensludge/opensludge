//
//  Start.h
//  OpenSludge
//
//  Created by Rikard Peterson on 2009-06-29.
//

#import <Cocoa/Cocoa.h>


@interface StartController : NSWindowController {
	IBOutlet NSButton *fullScreenCheck;
	IBOutlet NSPopUpButton *languageList;
	IBOutlet NSButton *aaCheck;
}
- (IBAction)okButton: (id)sender;
- (IBAction)cancelButton: (id)sender;

@end

