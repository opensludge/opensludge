#ifdef GNUSTEP
extern "C"
{
#import <Cocoa/Cocoa.h>
}
#else
#import <Cocoa/Cocoa.h>
#endif

@interface AppController : NSObject
{
	
	IBOutlet NSWindow *preferenceWindow;
	IBOutlet NSButton *prefKeepImages;
	IBOutlet NSButton *prefWriteStrings;
	IBOutlet NSButton *prefVerbose;
	
}

- (IBAction)prefsMenu:(id)sender;
- (IBAction)newProject:(id)sender;
- (IBAction)menuOpen:(id)sender;
- (IBAction)helpMenu:(id)sender;

- (IBAction)compileMenu:(id)sender;
- (IBAction)projectPrefsMenu:(id)sender;

- (IBAction)spriteBankNew:(id)sender;
- (IBAction)floorNew:(id)sender;
- (IBAction)translationNew:(id)sender;
- (IBAction)zBufferNew:(id)sender;

- (IBAction)prefKeepImagesClick:(id)sender;
- (IBAction)prefWriteStringsClick:(id)sender;
- (IBAction)prefVerboseClick:(id)sender;
@end