//
//  ProjectDocument.m
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2009-07-18.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "ProjectDocument.h"
#include "Project.hpp"

#include "settings.h"
#include "WInterfa.h"

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

extern struct errorLinkToFile * errorList;
extern int numErrors;
// --

ProjectDocument * me;
NSModalSession session = nil;

@implementation ProjectDocument

- (NSString *)windowNibName {
    // Implement this to return a nib to load OR implement -makeWindowControllers to manually create your controllers.
    return @"ProjectDocument";
}

- (BOOL)readFromURL:(NSURL *)absoluteURL 
			 ofType:(NSString *)typeName 
			  error:(NSError **)outError
{
	if ([typeName isEqualToString:@"SLUDGE Project file"]) {	
		UInt8 buffer[1024];
		if (CFURLGetFileSystemRepresentation((CFURLRef) absoluteURL, true, buffer, 1023)) {
			if (loadProject ((char *) buffer)) {
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
	if ([typeName isEqualToString:@"SLUDGE Project file"]) {	
		UInt8 buffer[1024];
		if (CFURLGetFileSystemRepresentation((CFURLRef) absoluteURL, true, buffer, 1023)) {
			if (saveProject ((char *) buffer)) {
				return YES;
			}
		}
	} 
	*outError = [NSError errorWithDomain:@"Error" code:1 userInfo:nil];
	return NO;
}

// This is the project file list!
- (int)numberOfRowsInTableView:(NSTableView *)tv
{
	if (tv == compilerErrors)
		return numErrors;
	else
		return fileListNum; 
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
	} else {
		if (row >= fileListNum) return nil;
		v = [NSString stringWithUTF8String:getFileFromList(row)];
	}
	return v;
}

- (void)close 
{
	closeProject ();
	[super close];
}

-(bool) compile
{
	bool val = false;
	me = self;
	session = [NSApp beginModalSessionForWindow:compilerWindow];
	[closeCompilerButton setEnabled:NO];
	[NSApp runModalSession:session];
/*	for (;;) {
		if ([NSApp runModalSession:session] != NSRunContinuesResponse)
			break;
		[self doSomeWork];
	}*/
	UInt8 buffer[1024];
	if (CFURLGetFileSystemRepresentation((CFURLRef) [self fileURL], true, buffer, 1023)) {
		compileEverything(buffer);
		val = true;
	}
	[closeCompilerButton setEnabled:YES];
	[NSApp endModalSession:session];
	if (numErrors) {
		[compilerErrors noteNumberOfRowsChanged];
		[tabView selectTabViewItemAtIndex:1];
	}
	return val;
}

- (IBAction)closeCompilerBox:(id)sender
{
	[compilerWindow close];
}

- (bool)showProjectPrefs
{
	[prefQuit setStringValue:[NSString stringWithUTF8String: settings.quitMessage]];
	[prefIcon setStringValue:[NSString stringWithUTF8String: settings.customIcon]];
	[prefData setStringValue:[NSString stringWithUTF8String: settings.runtimeDataFolder]];
	[prefFile setStringValue:[NSString stringWithUTF8String: settings.finalFile]];
	[prefName setStringValue:[NSString stringWithUTF8String: settings.windowName]];
	[prefHeight setIntValue:settings.screenHeight];
	[prefWidth setIntValue:settings.screenWidth];
	[prefSpeed setIntValue:settings.frameSpeed];
	[prefSilent setIntValue: settings.forceSilent];
	
	[NSApp beginSheet:projectPrefs
	   modalForWindow:[self windowForSheet]
		modalDelegate:nil
	   didEndSelector:NULL
		  contextInfo:NULL];
	return true;
}

- (IBAction)endProjectPrefs:(id)sender{
	killSettingsStrings();
	settings.quitMessage = newString ([[prefQuit stringValue] UTF8String]);
	settings.customIcon = newString ([[prefIcon stringValue] UTF8String]);
	settings.runtimeDataFolder = newString ([[prefData stringValue]UTF8String]);
	settings.finalFile = newString ([[prefFile stringValue]UTF8String]);
	settings.windowName = newString ([[prefName stringValue]UTF8String]);
	settings.screenHeight = [prefHeight intValue];
	settings.screenWidth = [prefWidth intValue];
	settings.frameSpeed = [prefSpeed intValue];
	settings.forceSilent = [prefSilent intValue];	
	
	[NSApp endSheet:projectPrefs];
	[projectPrefs orderOut:sender];
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
		case COM_PROGTEXT:
			[compTask setStringValue:s];
			break;
		case COM_FILENAME:
			[compFile setStringValue:s];
			break;
		case COM_ITEMTEXT:
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

void setCompilerText (int where, const char * tx) {
	[me setText:tx here:where];
}

void setCompilerStats (int funcs, int objTypes, int resources, int globals, int strings) {
	[me setStats: funcs obj:objTypes res:resources glob:globals strings:strings];
}

void compilerCommentsUpdated() {
	[me newComments];
}
