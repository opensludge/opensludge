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
		UInt8 buffer[1024];
		if (CFURLGetFileSystemRepresentation((CFURLRef) [self fileURL], true, buffer, 1023)) {
			if ([p isFileInProject: buffer]) {
				project = p;
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
