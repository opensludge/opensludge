//
//  TranslationDocument.m
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2009-08-05.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "TranslationDocument.h"
#include "translator.h"

#include "moreio.h"
#include "settings.h"


@implementation stringTable

// We override this function to make the Return key work properly even though
// we remove rows from the list when they're edited. (NSTableView doesn't expect that.)
- (void)textDidEndEditing:(NSNotification *)aNotification
{	
	int move = [[[aNotification userInfo] valueForKey: @"NSTextMovement"] intValue];
		
	if ((type == 0 || type == 3) && move == NSReturnTextMovement) {
		int rowIndex = [self selectedRow];
		
		[super textDidEndEditing: [NSNotification notificationWithName:[aNotification name] object:[aNotification object]]];
		int numRows = [[self dataSource] numberOfRowsInTableView:self];
		if (numRows > rowIndex) {
			[[NSNotificationCenter defaultCenter] postNotificationName:@"NSTableViewSelectionDidChangeNotification" object:self];
			[self editColumn:2 row:rowIndex withEvent:nil select:YES];
		} else if (numRows) {
			[self selectRowIndexes: [NSIndexSet indexSetWithIndex:0] byExtendingSelection:NO];
			[self editColumn:2 row:0 withEvent:nil select:YES];
		}
	} else 
		[super textDidEndEditing: aNotification];
}

- (void) setType:(int)i
{
	type = i;
}
- (int) type
{
	return type;
}
@end

@implementation TranslationDocument

-(void) countRows
{
}

- (NSString *)windowNibName {
    return @"TranslationEditor";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
    // Add any code here that needs to be executed once the windowController has loaded the document's window.
	
	if (langName) [languageName setStringValue: [NSString stringWithCString: langName encoding:NSISOLatin1StringEncoding]];
	if (langID) [languageID setIntValue:langID];
}


- (BOOL)readFromURL:(NSURL *)absoluteURL 
			 ofType:(NSString *)typeName 
			  error:(NSError **)outError
{
	if ([typeName isEqualToString:@"SLUDGE Translation file"]) {	
		UInt8 buffer[1024];
		if (CFURLGetFileSystemRepresentation((CFURLRef) absoluteURL, true, buffer, 1023)) {
			if (loadTranslationFile ((char *) buffer, &firstTransLine, &langName, &langID)) {
				[listOfStrings noteNumberOfRowsChanged];
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
	if ([typeName isEqualToString:@"SLUDGE Translation file"]) {		
		UInt8 buffer[1024];
		if (CFURLGetFileSystemRepresentation((CFURLRef) absoluteURL, true, buffer, 1023)) {
			if (saveTranslationFile ((char *) buffer, firstTransLine, langName, langID)) {
				return YES;
			}
		}
	} 
	*outError = [NSError errorWithDomain:@"Error" code:1 userInfo:nil];
	return NO;
}

-(IBAction) showTheseChanged:(id)sender {
	[listOfStrings reloadData];	// This is done first, in case a row is being edited.
	[listOfStrings setType: [showThese indexOfSelectedItem]];
	[listOfStrings noteNumberOfRowsChanged];
}

// This is the project file list!
- (int)numberOfRowsInTableView:(stringTable *)tv
{
	int c = 0;
	int type = 	[tv type];
	struct transLine * line = firstTransLine;
	while (line) {
		switch (type) {
			case 0:
				if (line->type == TYPE_NEW) c++; break;
			case 2:
				if (line->type == TYPE_TRANS) c++; break;
			case 3:
				if (line->type == TYPE_NONE) c++; break;
			default:
			c++;
		}
		line = line->next;
	}
	return c;
}
- (id)tableView:(stringTable *)tv
	objectValueForTableColumn:(NSTableColumn *)tableColumn
			row:(int)row
{
	int c = -1;
	int col = [[tableColumn identifier] intValue];
	int type = 	[tv type];
	struct transLine * line = firstTransLine;
	while (line && c<row) {
		switch (type) {
			case 0:
				if (line->type == TYPE_NEW) c++; break;
			case 2:
				if (line->type == TYPE_TRANS) c++; break;
			case 3:
				if (line->type == TYPE_NONE) c++; break;
			default:
				c++;
		}
		if (c<row)line = line->next;
	}
	if (! line) return nil;
	
	switch (col) {
	case 0:
		if (line->transFrom)
			return [NSString stringWithCString: line->transFrom encoding:NSISOLatin1StringEncoding];
		else 
			return @"";
	case 1:
		if (line){
			return [NSNumber numberWithInt:line->type != TYPE_NONE];
		} else {
			return [NSNumber numberWithInt:0];
		}
	case 2:
		if (line->transTo)
			return [NSString stringWithCString: line->transTo encoding:NSISOLatin1StringEncoding];
		else 
			return @"";
	}
	return @"";
}

- (void)tableView:(stringTable *)aTableView 
   setObjectValue:(id)anObject 
   forTableColumn:(NSTableColumn *)tableColumn 
			  row:(int)row
{
	int c = -1;
	int type = 	[aTableView type];
	struct transLine * line = firstTransLine;
	while (line && c<row) {
		switch (type) {
			case 0:
				if (line->type == TYPE_NEW) c++; break;
			case 2:
				if (line->type == TYPE_TRANS) c++; break;
			case 3:
				if (line->type == TYPE_NONE) c++; break;
			default:
				c++;
		}
		if (c<row)line = line->next;
	}
	if (! line) return;
	
	int col = [[tableColumn identifier] intValue];
	switch (col) {
		case 1:
			if (! [anObject intValue]) {
				if (line->transTo) {
					deleteString (line->transTo);
					line->transTo = NULL;
				}
				line->type = TYPE_NONE;
			} else {
				if (line->transTo)
					line->type = TYPE_TRANS;
				else
					line->type = TYPE_NEW;
			}
			break;
		case 2:
			if (line->transTo) {
				if (! strcmp(line->transTo, [anObject cStringUsingEncoding: NSISOLatin1StringEncoding]))
					return;
				deleteString (line->transTo);
			} else if (! strlen([anObject cStringUsingEncoding: NSISOLatin1StringEncoding]))
				return;
			line->transTo = copyString([anObject cStringUsingEncoding: NSISOLatin1StringEncoding]);
			if (!strlen(line->transTo)) {
				if (line->type != TYPE_NONE)
					line->type = TYPE_NEW;
				deleteString (line->transTo);
				line->transTo = NULL;
			} else {
				line->type = TYPE_TRANS;
			}
			break;
		default:
			return;
	}	
	[listOfStrings noteNumberOfRowsChanged];
	[self updateChangeCount: NSChangeDone];	
}

- (IBAction)loadStrings:(id)sender {
	NSString *path = nil;
	NSOpenPanel *openPanel = [ NSOpenPanel openPanel ];
	[openPanel setTitle:@"Select a SLUDGE Project"];
	NSArray *files = [NSArray arrayWithObjects:@"slp", nil];
	
	if ( [ openPanel runModalForDirectory:nil file:nil types:files] ) {
		path = [ openPanel filename ];
		if (updateFromProject ([path UTF8String], &firstTransLine)) {
		
			[listOfStrings noteNumberOfRowsChanged];
			[self updateChangeCount: NSChangeDone];
		}
	}
}



- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
	int c = -1;
	int row = [listOfStrings selectedRow];
	if (row >=0 ) {
		int type = (int) [[aNotification object] type];
		struct transLine * line = firstTransLine;
		while (line && c<row) {
			switch (type) {
				case 0:
					if (line->type == TYPE_NEW) c++; break;
				case 2:
					if (line->type == TYPE_TRANS) c++; break;
				case 3:
					if (line->type == TYPE_NONE) c++; break;
				default:
					c++;
			}
			if (c<row)line = line->next;
		}
		if (! line) {
			[originalString setStringValue:@""];
			return;
		} 
		[originalString setStringValue: [NSString stringWithCString: line->transFrom encoding:NSISOLatin1StringEncoding]];
	} else {
		[originalString setStringValue:@""];
	}
}


- (IBAction)changeDone:(id)sender
{
	if (! langName) {
		if (strlen([[languageName stringValue] cStringUsingEncoding: NSISOLatin1StringEncoding])) {
			langName = newString ([[languageName stringValue] cStringUsingEncoding: NSISOLatin1StringEncoding]);
			[self updateChangeCount: NSChangeDone];
		}
	} else if (strcmp(langName, [[languageName stringValue] cStringUsingEncoding: NSISOLatin1StringEncoding])) {
		deleteString (langName);
		langName = newString ([[languageName stringValue] cStringUsingEncoding: NSISOLatin1StringEncoding]);
		[self updateChangeCount: NSChangeDone];
	}
	
	if (langID != [languageID intValue]) {
		langID = [languageID intValue];
		[self updateChangeCount: NSChangeDone];
	}
}

@end
