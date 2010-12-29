//
//  SLUDGE Document.m
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2010-12-28.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "SLUDGE Document.h"
#import "ProjectDocument.h"

@implementation SLUDGE_Document

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];

	// Are we opening a project file?
	if (! project) {
		// No, we're not! Check if we belong in an open project...
		
		SLUDGE_Document *doc = [[NSDocumentController sharedDocumentController] currentDocument];
		ProjectDocument *p = (ProjectDocument *)[doc project];
		if (! p) return;
		
		// Are we creating a new file?
		if (! [self fileURL]) {
			if (NSRunAlertPanel ([p getTitle], @"Do you want to add the new file to the project?", @"Yes", @"No", NULL) == NSAlertDefaultReturn) {
				
				NSString *path = nil;
				NSSavePanel *savePanel = [ NSSavePanel savePanel ];
				NSArray *files;

				if ([[self fileType] compare: @"SLUDGE Script"] == NSOrderedSame) {
					[savePanel setTitle:@"Save script file"];
					files = [NSArray arrayWithObjects:@"slu", nil];
				} else if ([[self fileType]compare: @"SLUDGE Sprite Bank"] == NSOrderedSame) {
					[savePanel setTitle:@"Save sprite bank"];
					files = [NSArray arrayWithObjects:@"duc", nil];
				} else if ([[self fileType]compare: @"SLUDGE Floor"] == NSOrderedSame) {
					[savePanel setTitle:@"Save floor"];
					files = [NSArray arrayWithObjects:@"flo", nil];
				} else if ([[self fileType]compare: @"SLUDGE Translation file"] == NSOrderedSame) {
					[savePanel setTitle:@"Save translation"];
					files = [NSArray arrayWithObjects:@"tra", nil];
				} else /*if ([[self fileType]compare: @"SLUDGE zBuffer"] == NSOrderedSame)*/ {
					[savePanel setTitle:@"Save zBuffer"];
					files = [NSArray arrayWithObjects:@"zbu", nil];
				}
				[savePanel setAllowedFileTypes:files];
				
				if ( [savePanel runModal] ) {
					path = [ savePanel filename ];
				}				
				if (path) {	
					NSURL *file = [NSURL fileURLWithPath: path];
					NSError **err;
					
					if ([self saveToURL: [file absoluteURL] 
							 ofType: [self fileType]
				   forSaveOperation: NSSaveOperation 
								  error: err]) {

						[p addNamedFileToProject: file];
					}
				}
			}
		} else {
			// File exists ... look for it in project!
			UInt8 buffer[1024];
			if (CFURLGetFileSystemRepresentation((CFURLRef) [self fileURL], true, buffer, 1023)) {
				if ([p isFileInProject: buffer]) {
					project = p;
				}
			}
		}
	}
}

- (SLUDGE_Document *) project
{
	return project;
}
- (void) setProject:(SLUDGE_Document *) p
{
	project = p;
}

@end
