/*   AppController.m - main entry point for our Cocoa-ized SDL app
       Initial Version: Darrell Walisser <dwaliss1@purdue.edu>
       Non-NIB-Code & other changes: Max Horn <max@quendi.de>

    Feel free to customize this file to suit your needs
*/

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
@end
