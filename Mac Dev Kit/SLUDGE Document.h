//
//  SLUDGE Document.h
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2010-12-28.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface SLUDGE_Document : NSDocument {
	SLUDGE_Document *project;
}

- (SLUDGE_Document *) project;
- (void) setProject:(SLUDGE_Document *) p;

@end
