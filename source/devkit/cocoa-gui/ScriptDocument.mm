//
//  ScriptDocument.m
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2009-08-23.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "ScriptDocument.h"


@implementation ScriptDocument

- (NSString *)windowNibName {
    return @"ScriptDocument";
}

-(void)	windowControllerDidLoadNib: (NSWindowController*)aController
{
    [super windowControllerDidLoadNib:aController];
		
	if( sourceCode != nil )
	{
		[text setString: sourceCode];
		[sourceCode release];
		sourceCode = nil;
	}
}	
	
-(NSData*)	dataRepresentationOfType: (NSString*)aType
{
    return [[text string] dataUsingEncoding: NSISOLatin1StringEncoding];
}

-(BOOL)	loadDataRepresentation: (NSData*)data ofType: (NSString*)aType
{
	if(sourceCode) {
		[sourceCode release];
		sourceCode = nil;
	}
	sourceCode = [[NSString alloc] initWithData:data encoding: NSISOLatin1StringEncoding];
		
	return YES;
}

@end
