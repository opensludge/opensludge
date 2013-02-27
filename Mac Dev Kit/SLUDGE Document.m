//
//  SLUDGE Document.m
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2010-12-28.
//  Copyright 2010. All rights reserved.
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
			NSString *path = nil;
			NSSavePanel *savePanel = [ NSSavePanel savePanel ];
			NSArray *files;

			if ([[self fileType] compare: @"SLUDGE Script"] == NSOrderedSame) {

				if (NSRunAlertPanel ([p getTitle], @"Do you want to add the new file to the project?", @"Yes", @"No", NULL) == NSAlertDefaultReturn) {				
					
					[savePanel setTitle:@"Save script file"];
					files = [NSArray arrayWithObjects:@"slu", @"sld", nil];
					[savePanel setAllowedFileTypes:files];
				
					if ( [savePanel runModal] ) {
						path = [ savePanel filename ];
					}				
				}
			} else if ([[self fileType] compare: @"SLUDGE Translation file"] == NSOrderedSame) {
				
				if (NSRunAlertPanel ([p getTitle], @"Do you want to add the new translation to the project?", @"Yes", @"No", NULL) == NSAlertDefaultReturn) {
										
					[savePanel setTitle:@"Save translation"];
					files = [NSArray arrayWithObjects:@"tra", nil];
					[savePanel setAllowedFileTypes:files];
					
					if ( [savePanel runModal] ) {
						path = [ savePanel filename ];
					}				
				}
			}
			if (path) {	
				NSURL *file = [NSURL fileURLWithPath: path];
				NSError *err;
				
				if ([self saveToURL: [file absoluteURL] 
							 ofType: [self fileType]
				   forSaveOperation: NSSaveOperation 
							  error: &err]) {
					
					[p addNamedFileToProject: file];
					project = p;
				}
                if (err) {
                    [NSApp presentError:err];
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
