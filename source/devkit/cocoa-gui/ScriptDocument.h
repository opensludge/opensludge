//
//  ScriptDocument.h
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2009-08-23.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#ifdef GNUSTEP
extern "C"
{
#import <Cocoa/Cocoa.h>
}
#else
#import <Cocoa/Cocoa.h>
#endif


@interface ScriptDocument : NSDocument {
	IBOutlet NSTextView *text;

	NSString*						sourceCode;				// Temp. storage for data from file until NIB has been read.
}

@end
