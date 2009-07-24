//
//  ProjectDocument.h
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2009-07-18.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface ProjectDocument : NSDocument {
	IBOutlet NSTableView *projectFiles;

	IBOutlet NSWindow *projectPrefs;
	IBOutlet NSTextField *prefQuit;
	IBOutlet NSTextField *prefIcon;
	IBOutlet NSTextField *prefData;
	IBOutlet NSTextField *prefFile;
	IBOutlet NSTextField *prefName;
	IBOutlet NSTextField *prefHeight;
	IBOutlet NSTextField *prefWidth;
	IBOutlet NSTextField *prefSpeed;
	IBOutlet NSButton *prefSilent;
	
	IBOutlet NSPanel *compilerWindow;
	IBOutlet NSProgressIndicator *progress1;
	IBOutlet NSProgressIndicator *progress2;
	IBOutlet NSTextField *compTask;
	IBOutlet NSTextField *compFile;
	IBOutlet NSTextField *compItem;
	IBOutlet NSButton *closeCompilerButton;
	
	//int progress1max, progress1val, progress2max, progress2val;
}

- (bool) compile;
- (bool)showProjectPrefs;
- (IBAction)endProjectPrefs:(id)sender;
- (IBAction)closeCompilerBox:(id)sender;

@end
