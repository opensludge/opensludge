//
//  ScriptDocument.h
//  Sludge Dev Kit
//
//  Created by Rikard Peterson on 2009-08-23.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "SLUDGE Document.h"


@interface ScriptDocument : SLUDGE_Document {
	IBOutlet NSTextView *text;
	
	
	NSDictionary *commandsColour, *commentsColour, *keywordsColour, *stringsColour, *variablesColour;
	
	NSMutableDictionary *builtinWords;
	NSMutableDictionary *keyWords;
/*	
	//NSString *completeString;
	NSString *searchString;
	
	NSLayoutManager * firstLayoutManager;
	NSRect visibleRect;
	NSRange visibleRange;
	NSRange rangeOfLine;
	
	int beginningOfFirstVisibleLine;
	int endOfLastVisibleLine;
	int beginning, end, endOfLine, index, length, searchStringLength, commandLocation, skipEndCommand, beginLocationInMultiLine, endLocationInMultiLine, searchSyntaxLength, rangeLocation;
	int maxRange, completeStringLength;

	NSCharacterSet *keywordStartCharacterSet, *keywordEndCharacterSet;
	NSString *keywordTestString;
	NSCharacterSet *beginVariable;
	NSCharacterSet *endVariable;
	NSString *firstSingleLineComment;

	NSString *firstString;
	NSString *secondString;
		
	NSScanner *scanner;
	NSScanner *completeDocumentScanner;	
	NSSet *keywords;
	*/
	
// --	

	NSString*						sourceCode;				// Temp. storage for data from file until NIB has been read.
}

- (bool)commentMenu;


@end
