//
//  ProjectDocument.h
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2009-07-18.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "SLUDGE Document.h"


@interface ProjectDocument : SLUDGE_Document {
	// Main window
	IBOutlet NSWindow *projectWindow;
	IBOutlet NSTabView *tabView;
	IBOutlet NSTableView *projectFiles;
	IBOutlet NSTableView *resourceFiles;
	IBOutlet NSTableView *compilerErrors;
	IBOutlet NSButton *removeFileButton;
	
	// Project Preferences
	IBOutlet NSWindow *projectPrefs;
	IBOutlet NSTextField *prefQuit;
	IBOutlet NSTextField *prefIcon;
	IBOutlet NSTextField *prefLogo;
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
	IBOutlet NSButton *runGameButton;
	IBOutlet NSButton *closeCompilerButton;
	
	int numResources;
	char *resourceList[1000];
	
	char *fileList[1000];
	int fileListNum;	
}
- (IBAction)addNamedFileToProject:(NSURL *)fileURL;
- (IBAction)addFileToProject:(id)sender;
- (IBAction)removeFileFromProject:(id)sender;

- (bool) isFileInProject: (UInt8 *) f;

- (bool) compile;
- (bool)showProjectPrefs;
- (IBAction)endProjectPrefs:(id)sender;
- (IBAction)runGame:(id)sender;
- (IBAction)closeCompilerBox:(id)sender;

- (NSString *) getTitle; 

- (void) getSettings;
- (void) setSettings;

@end
