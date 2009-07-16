#import <Cocoa/Cocoa.h>

@interface AppController : NSObject
{
	IBOutlet NSWindow *projectWindow;
	IBOutlet NSTableView *projectFiles;
}

- (IBAction)prefsMenu:(id)sender;
- (IBAction)newProject:(id)sender;
- (IBAction)openProject:(id)sender;
- (IBAction)saveProject:(id)sender;
- (IBAction)saveProjectAs:(id)sender;
- (IBAction)helpMenu:(id)sender;

- (IBAction)compileMenu:(id)sender;
@end
