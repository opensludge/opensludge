//
//  ScriptDocument.m
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2009-08-23.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "ScriptDocument.h"


NSMutableDictionary * keyWords;
NSColor * blue;
NSColor * green;


@implementation ScriptDocument

- (NSString *)windowNibName {
    return @"ScriptDocument";
}

NSCharacterSet *whiteSpaceSet;

void addFunction (NSMutableDictionary *words, char *name) {
	[words setObject:[NSString stringWithUTF8String: name] forKey:[NSString stringWithUTF8String: name]];
}

- (void)awakeFromNib
{

	[[text textStorage] setDelegate:self];
	
	// load our dictionary
	whiteSpaceSet = [NSCharacterSet whitespaceAndNewlineCharacterSet];
	words = [[NSMutableDictionary alloc] init];

#define FUNC(special,name) addFunction(words, #name);
#include "functionList.h"
#undef FUNC
	
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

- (void)textStorageDidProcessEditing:(NSNotification *)notification
{
    NSTextStorage *textStorage = [notification object];
    NSString *string = [textStorage string];
    NSRange area = [textStorage editedRange];
    unsigned int length = [string length];
    NSRange start, end;
    NSCharacterSet *whiteSpaceSet;
    unsigned int areamax = NSMaxRange(area);
    NSRange found;
    NSString *word;
    	
    // extend our range along word boundaries.
    whiteSpaceSet = [[NSCharacterSet alphanumericCharacterSet]invertedSet];
    start = [string rangeOfCharacterFromSet:whiteSpaceSet
                                    options:NSBackwardsSearch
                                      range:NSMakeRange(0, area.location)];
    if (start.location == NSNotFound) {
        start.location = 0;
    }  else {
        start.location = NSMaxRange(start);
    }
    end = [string rangeOfCharacterFromSet:whiteSpaceSet
                                  options:0
                                    range:NSMakeRange(areamax, length - areamax)];
    if (end.location == NSNotFound)
        end.location = length;
    area = NSMakeRange(start.location, end.location - start.location);
    if (area.length == 0) return; // bail early
    
    // remove the old colors
    [textStorage removeAttribute:NSForegroundColorAttributeName range:area];
	
    // add new colors
    while (area.length) {
        // find the next word
        end = [string rangeOfCharacterFromSet:whiteSpaceSet
                                      options:0
                                        range:area];
        if (end.location == NSNotFound) {
            end = found = area;
        } else {
            found.length = end.location - area.location;
            found.location = area.location;
        }
        word = [string substringWithRange:found];
		
        // color as necessary
        if ([words objectForKey:word] != NULL) {
            [textStorage addAttribute:NSForegroundColorAttributeName
                                value:[NSColor blueColor]
                                range:found];
        }
        
        // adjust our area
        areamax = NSMaxRange(end);
        area.length -= areamax - area.location;
        area.location = areamax;
    }
}

@end
