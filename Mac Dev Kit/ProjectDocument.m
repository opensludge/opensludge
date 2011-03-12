//
//  ProjectDocument.m
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2009-07-18.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "ProjectDocument.h"
#import "ScriptDocument.h"
#include "Project.hpp"

#include "moreio.h"
//#include "helpers.h"
#include "settings.h"
#include "compilerinfo.h"

extern char * sourceDirectory;

// -- These are from "MessBox.h"
struct errorLinkToFile
{
	int errorType;
	char * overview;
	char * filename;
	char * fullText;
	int lineNumber;
	struct errorLinkToFile * next;
};

extern char * errorTypeStrings[];


extern struct errorLinkToFile * errorList;
extern int numErrors;

// --

ProjectDocument * me;
NSModalSession session = nil;

@implementation ProjectDocument

- (id)init
{
    self = [super init];
    if (self) {	
		numResources = 0;		
		project = self;
    }
    return self;
}


- (NSString *)windowNibName {
    // Implement this to return a nib to load OR implement -makeWindowControllers to manually create your controllers.
    return @"ProjectDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
	
	[resourceFiles setDoubleAction:@selector(openItem:)];
	[resourceFiles setTarget:self];
	[projectFiles setDoubleAction:@selector(openProjectFile:)];
	[projectFiles setTarget:self];
	[compilerErrors setDoubleAction:@selector(openError:)];
	[compilerErrors setTarget:self];
	
	UInt8 filename[1024];
	if (! CFURLGetFileSystemRepresentation((CFURLRef) [self fileURL], true, filename, 1023))
		return;
	getSourceDirFromName ((char *) filename);
	[self getSettings];
}	

- (BOOL)readFromURL:(NSURL *)absoluteURL 
			 ofType:(NSString *)typeName 
			  error:(NSError **)outError
{
	if ([typeName isEqualToString:@"SLUDGE Project file"]) {	
		UInt8 buffer[1024];
		if (CFURLGetFileSystemRepresentation((CFURLRef) absoluteURL, true, buffer, 1023)) {
			if (loadProject ((char *) buffer, fileList, &fileListNum)) {
				[projectFiles noteNumberOfRowsChanged];
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
	[self setSettings];
	if ([typeName isEqualToString:@"SLUDGE Project file"]) {	
		UInt8 buffer[1024];
		if (CFURLGetFileSystemRepresentation((CFURLRef) absoluteURL, true, buffer, 1023)) {
			if (saveProject ((char *) buffer, fileList, &fileListNum)) {
				return YES;
			}
		}
	} 
	*outError = [NSError errorWithDomain:@"Error" code:1 userInfo:nil];
	return NO;
}


- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
	if ([aNotification object] == projectFiles) {
		if ([projectFiles numberOfSelectedRows]) {
			[removeFileButton setEnabled:YES];
			clearFileList (resourceList, &numResources);
			UInt8 filename[1024];
			if (! CFURLGetFileSystemRepresentation((CFURLRef) [self fileURL], true, filename, 1023))
				return;
			getSourceDirFromName ((char *) filename);
			
			int i;
			for (i = 0; i < fileListNum; i ++) {
				if (! [projectFiles isRowSelected:i]) continue;
				
				populateResourceList (fileList[i], resourceList, &numResources);
			}
			[resourceFiles noteNumberOfRowsChanged];
		} else {
			clearFileList (resourceList, &numResources);
			[resourceFiles noteNumberOfRowsChanged];
			[removeFileButton setEnabled:NO];
		}
	}
}

- (void)openProjectFile:(id)sender
{
	int row = [projectFiles clickedRow];
	if (row == -1) 
		return;
	
	char *file = getFullPath (fileList[row]);
	[[NSWorkspace sharedWorkspace] openFile: [NSString stringWithUTF8String:file]];
	deleteString (file);
}

- (void)openError:(id)sender
{
	int row = [compilerErrors clickedRow];
	if (row == -1) 
		return;
	
	struct errorLinkToFile * index = errorList;
	if (! index) return;
	int i = numErrors-1;
	while (i>row) {
		if (! (index = index->next)) return;
		i--;
	}
	
	if (! index-> filename) {
		errorBox (errorTypeStrings[index->errorType], index->overview);
		return;
	}
	char *file = getFullPath (index->filename);

	NSError **err;

	NSDocumentController *docControl = [NSDocumentController sharedDocumentController];
	NSURL *url = [NSURL fileURLWithPath: [NSString stringWithUTF8String:file]];
	ScriptDocument * doc = [docControl openDocumentWithContentsOfURL: url
															 display: YES
															   error: err];
	
	if (doc) {
		[doc selectLine:index->lineNumber];
	}
	 
	//[[NSWorkspace sharedWorkspace] openFile: [NSString stringWithUTF8String:file]];
	deleteString (file);
}

- (void)openItem:(id)sender
{
	int row = [resourceFiles clickedRow];
	if (row == -1) 
		return;
	
	char *file = getFullPath (resourceList[row]);
	[[NSWorkspace sharedWorkspace] openFile: [NSString stringWithUTF8String:file]];
	
	deleteString (file);
}

// This is the project file list!
- (int)numberOfRowsInTableView:(NSTableView *)tv
{
	if (tv == compilerErrors)
		return numErrors;
	else if (tv == projectFiles)
		return fileListNum; 
	else 
		return numResources;
}
- (id)tableView:(NSTableView *)tv
objectValueForTableColumn:(NSTableColumn *)tableColumn
			row:(int)row
{
	NSString *v;
	if (tv == compilerErrors) {
		struct errorLinkToFile * index = errorList;
		if (! index) return nil;
		int i = numErrors-1;
		while (i>row) {
			if (! (index = index->next)) return nil;
			i--;
		}
		v = [NSString stringWithUTF8String:index->fullText];
	} else if (tv == projectFiles) {
		if (row >= fileListNum) return nil;
		v = [NSString stringWithUTF8String:fileList[row]];
	} else {
		v = [NSString stringWithUTF8String:resourceList[row]];
	}
	return v;
}


- (IBAction)addNamedFileToProject:(NSURL *)fileURL
{
	UInt8 path[1024];
	if (! CFURLGetFileSystemRepresentation((CFURLRef) fileURL, true, path, 1023))
		return;
	
	UInt8 filename[1024];
	if (! CFURLGetFileSystemRepresentation((CFURLRef) [self fileURL], true, filename, 1023))
		return;
	getSourceDirFromName ((char *) filename);
	addFileToProject ((char *) path, sourceDirectory, fileList, &fileListNum);
		
	[projectFiles noteNumberOfRowsChanged];
	[self updateChangeCount: NSChangeDone];
}

- (IBAction)addFileToProject:(id)sender
{
	NSString *path = nil;
	NSOpenPanel *openPanel = [ NSOpenPanel openPanel ];
	[openPanel setTitle:@"Add file to SLUDGE Project"];
	NSArray *files = [NSArray arrayWithObjects:@"slu", @"sld", @"tra", nil];
	
	if ( [ openPanel runModalForDirectory:nil file:nil types:files] ) {
		path = [ openPanel filename ];
		UInt8 filename[1024];
		if (! CFURLGetFileSystemRepresentation((CFURLRef) [self fileURL], true, filename, 1023))
			return;
		getSourceDirFromName ((char *) filename);
		addFileToProject ([path UTF8String], sourceDirectory, fileList, &fileListNum);
		
		[projectFiles noteNumberOfRowsChanged];
		[self updateChangeCount: NSChangeDone];
	}
}
- (IBAction)removeFileFromProject:(id)sender{
	if ([projectFiles numberOfSelectedRows]) {
		if (! NSRunAlertPanel (@"Remove files?", @"Do you want to remove the selected files from the project? (They will not be deleted from the disk.)", @"Yes", @"No", NULL) == NSAlertDefaultReturn) {
			return;
		}
		int i, o, num = fileListNum;
		for (i = 0, o= 0; i < num; i ++, o++) {
			if (! [projectFiles isRowSelected:i]) continue;
			[projectFiles deselectRow:i];
			removeFileFromList (o, fileList, &fileListNum);
			o--;
		}
		
		[projectFiles noteNumberOfRowsChanged];
		[removeFileButton setEnabled:NO];
		[self updateChangeCount: NSChangeDone];
	}
}

- (void)close 
{
	closeProject (fileList, &fileListNum);
	
	// Let's also close all related documents!
	NSEnumerator *enumerator = [[[NSDocumentController sharedDocumentController] documents] objectEnumerator];
	SLUDGE_Document * anObject;
	while (anObject = [enumerator nextObject]) {
		if (anObject == self) continue;
		if ([anObject project] == self) {
			[anObject close];
		}
	}
	
	[super close];
}

-(bool) compile
{
	bool val = false;
	int success = false;
	
	[self setSettings];
	
	me = self;
	session = [NSApp beginModalSessionForWindow:compilerWindow];
	[closeCompilerButton setEnabled:NO];
	[runGameButton setEnabled:NO];
	[NSApp runModalSession:session];
	
	UInt8 buffer[1024];
	if (CFURLGetFileSystemRepresentation((CFURLRef) [self fileURL], true, buffer, 1023)) {
		success = compileEverything(buffer, fileList, &fileListNum);
		val = true;
	}
	[closeCompilerButton setEnabled:YES];
	[NSApp endModalSession:session];
	if (numErrors) {
		[compilerErrors noteNumberOfRowsChanged];
		[tabView selectTabViewItemAtIndex:1];
		[projectWindow orderFront: self];
	} 
	if (success) {
		[runGameButton setEnabled:YES];
	}
	return val;
}

- (IBAction)closeCompilerBox:(id)sender
{
	[compilerWindow close];
}

extern char * gameFile;

- (IBAction)runGame:(id)sender
{
	// Run the game
	char *file = getFullPath (gameFile);
	[[NSWorkspace sharedWorkspace] openFile: [NSString stringWithUTF8String:file]];
	fprintf(stderr, "*%s* *%s*\n", settings.finalFile, file);
	deleteString (file);
}


- (bool)showProjectPrefs
{
	
	[NSApp beginSheet:projectPrefs
	   modalForWindow:[self windowForSheet]
		modalDelegate:nil
	   didEndSelector:NULL
		  contextInfo:NULL];
	return true;
}

- (IBAction)endProjectPrefs:(id)sender{
	
	[NSApp endSheet:projectPrefs];
	[projectPrefs orderOut:sender];
	[self updateChangeCount: NSChangeDone];
}

- (NSString *) getTitle
{
	return [prefName stringValue];
}

- (void) getSettings
{
	[prefQuit setStringValue:[NSString stringWithUTF8String: settings.quitMessage]];
	[prefIcon setStringValue:[NSString stringWithUTF8String: settings.customIcon]];
	[prefLogo setStringValue:[NSString stringWithUTF8String: settings.customLogo]];
	[prefData setStringValue:[NSString stringWithUTF8String: settings.runtimeDataFolder]];
	[prefFile setStringValue:[NSString stringWithUTF8String: settings.finalFile]];
	[prefName setStringValue:[NSString stringWithUTF8String: settings.windowName]];
	[prefLanguage setStringValue:[NSString stringWithUTF8String: settings.originalLanguage]];
	[prefHeight setIntValue:settings.screenHeight];
	[prefWidth setIntValue:settings.screenWidth];
	[prefSpeed setIntValue:settings.frameSpeed];
	[prefSilent setIntValue: settings.forceSilent];
}

- (void) setSettings
{
	killSettingsStrings();
	settings.quitMessage = copyString ([[prefQuit stringValue] UTF8String]);
	settings.customIcon = copyString ([[prefIcon stringValue] UTF8String]);
	settings.customLogo = copyString ([[prefLogo stringValue] UTF8String]);
	settings.runtimeDataFolder = copyString ([[prefData stringValue]UTF8String]);
	settings.finalFile = copyString ([[prefFile stringValue]UTF8String]);
	settings.windowName = copyString ([[prefName stringValue]UTF8String]);
	settings.originalLanguage = copyString ([[prefLanguage stringValue]UTF8String]);
	settings.screenHeight = [prefHeight intValue];
	settings.screenWidth = [prefWidth intValue];
	settings.frameSpeed = [prefSpeed intValue];
	settings.forceSilent = [prefSilent intValue];	
}

- (void) setProgress1max:(int)i
{
	[progress1 setMaxValue: i];
}
- (void) setProgress2max:(int)i
{
	[progress2 setMaxValue: i];
}
- (void) setProgress1val:(int)i
{
	[progress1 setDoubleValue: i];
}
- (void) setProgress2val:(int)i
{
	[progress2 setDoubleValue: i];
}

- (void) setText:(const char*) tx
			here:(int) where {
	
	NSString *s = [NSString stringWithUTF8String: tx];
	if (!s) return;
	
	switch (where) {
		case COMPILER_TXT_ACTION:
			[compTask setStringValue:s];
			break;
		case COMPILER_TXT_FILENAME:
			[compFile setStringValue:s];
			break;
		case COMPILER_TXT_ITEM:
			[compItem setStringValue:s];
			break;
	}
}

- (void) setStats: (int) funcs 
			  obj:(int)objTypes 
			  res:(int)resources 
			 glob:(int)globals 
		  strings:(int)strings
{
	[compFuncs setIntValue: funcs];
	[compObjs setIntValue: objTypes];
	[compGlobs setIntValue: resources];
	[compStrings setIntValue: globals];
	[compResources setIntValue: strings];
}

- (void) newComments
{
	[compilerErrors noteNumberOfRowsChanged];
	[tabView selectTabViewItemAtIndex:1];
}

- (bool) isFileInProject: (UInt8 *) f {
	if (! fileListNum) return false;
	
	UInt8 filename[1024];
	if (! CFURLGetFileSystemRepresentation((CFURLRef) [self fileURL], true, filename, 1023))
		return false;
	getSourceDirFromName ((char *) filename);

	char * x = strstr((char *) f, (char *)sourceDirectory);
	
	if (! x) return false;
	x += strlen(sourceDirectory) + 1;
	
	int row;
	
	for (row = 0; row < fileListNum; row++) {
		if (strcmp(fileList[row], x)) return true;
	}
	for (row = 0; row < fileListNum; row++) {
		if (isResource (fileList[row], x)) return true;
	}

	return false;
}


@end

void percRect (unsigned int i, int whichBox) {
	if (! whichBox) {
		[me setProgress1val:i];
	} else
		[me setProgress2val:i];
	[NSApp runModalSession:session];
}

void clearRect (int i, int whichBox) {
	if (! whichBox) {
		[me setProgress1max:i?i:1];
	} else
		[me setProgress2max:i?i:1];
}

void setCompilerText (const where, const char * tx) {
	[me setText:tx here:where];
}

void setCompilerStats (int funcs, int objTypes, int resources, int globals, int strings) {
	[me setStats: funcs obj:objTypes res:resources glob:globals strings:strings];
}

void compilerCommentsUpdated() {
	[me newComments];
}

// For Mac OS X, we don't use these functions:

void setFinished(bool success)
{}

void setInfoReceiver(void (*infoReceiver)(compilerInfo *))
{}
