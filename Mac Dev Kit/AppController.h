#import <Cocoa/Cocoa.h>

@interface AppController : NSObject
{
	IBOutlet NSWindow *projectWindow;
	IBOutlet NSTableView *projectFiles;
	
	IBOutlet NSWindow *preferenceWindow;
	IBOutlet NSButton *prefKeepImages;
	IBOutlet NSButton *prefWriteStrings;
	IBOutlet NSButton *prefVerbose;
}

- (IBAction)prefsMenu:(id)sender;
- (IBAction)newProject:(id)sender;
- (IBAction)openProject:(id)sender;
- (IBAction)closeProject:(id)sender;
- (IBAction)saveProject:(id)sender;
- (IBAction)saveProjectAs:(id)sender;
- (IBAction)helpMenu:(id)sender;

- (IBAction)compileMenu:(id)sender;

- (IBAction)prefKeepImagesClick:(id)sender;
- (IBAction)prefWriteStringsClick:(id)sender;
- (IBAction)prefVerboseClick:(id)sender;
@end
