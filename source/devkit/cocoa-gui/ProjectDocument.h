//
//  ProjectDocument.h
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2009-07-18.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface ProjectDocument : NSDocument {
	// Main window
	IBOutlet NSTabView *tabView;
	IBOutlet NSTableView *projectFiles;
	IBOutlet NSTableView *resourceFiles;
	IBOutlet NSTableView *compilerErrors;
	IBOutlet NSButton *removeFileButton;
	
	// Project Preferences
	IBOutlet NSWindow *projectPrefs;
	IBOutlet NSTextField *prefQuit;
	IBOutlet NSTextField *prefIcon;
	IBOutlet NSTextField *prefData;
	IBOutlet NSTextField *prefFile;
	IBOutlet NSTextField *prefName;
	IBOutlet NSTextField *prefLanguage;
	IBOutlet NSTextField *prefHeight;
	IBOutlet NSTextField *prefWidth;
	IBOutlet NSTextField *prefSpeed;
	IBOutlet NSButton *prefSilent;
	
	// Compiler window
	IBOutlet NSPanel *compilerWindow;
	IBOutlet NSProgressIndicator *progress1;
	IBOutlet NSProgressIndicator *progress2;
	IBOutlet NSTextField *compTask;
	IBOutlet NSTextField *compFile;
	IBOutlet NSTextField *compItem;
	IBOutlet NSTextField *compFuncs;
	IBOutlet NSTextField *compObjs;
	IBOutlet NSTextField *compGlobs;
	IBOutlet NSTextField *compStrings;
	IBOutlet NSTextField *compResources;
	IBOutlet NSButton *closeCompilerButton;
	
	int numResources;
	char *resourceList[1000];
	
	char *fileList[1000];
	int fileListNum;	
}
- (IBAction)addFileToProject:(id)sender;
- (IBAction)removeFileFromProject:(id)sender;

- (bool) compile;
- (bool)showProjectPrefs;
- (IBAction)endProjectPrefs:(id)sender;
- (IBAction)closeCompilerBox:(id)sender;

@end
