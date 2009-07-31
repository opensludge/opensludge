#import <unistd.h>

#import "AppController.h"

#include "Settings.h"
#import "SpriteBank.h"
#import "ProjectDocument.h"

AppController *aC;

/* The main class of the application, the application's delegate */
@implementation AppController

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender
{
	return NO;
}

-(id) init
{
	[super init];
	aC = self;
	return self;
}

- (IBAction)prefsMenu:(id)sender
{
	[preferenceWindow makeKeyAndOrderFront:nil];
	
	[prefKeepImages setState: ! programSettings.compilerKillImages];
	[prefWriteStrings setState: programSettings.compilerWriteStrings];
	[prefVerbose setState: programSettings.compilerVerbose];
}

- (IBAction)menuOpen:(id)sender
{
	NSDocumentController *docControl = [NSDocumentController sharedDocumentController];
	return [docControl openDocument: sender];
}

- (IBAction)newProject:(id)sender
{    
	NSString *path = nil;
	NSSavePanel *savePanel = [ NSSavePanel savePanel ];
	[savePanel setTitle:@"New SLUDGE Project"];
	[savePanel setRequiredFileType:@"slp"];
	
	if ( [ savePanel runModalForDirectory:nil file:nil ] ) {
		path = [ savePanel filename ];
		doNewProject ([path UTF8String]);
	}
	
	if (path) {	
		NSURL *file = [NSURL fileURLWithPath: path];
		NSDocumentController *docControl = [NSDocumentController sharedDocumentController];
		NSError **err;
		NSDocument *project = [docControl makeDocumentWithContentsOfURL:file ofType:@"SLUDGE Project file" error:err];
		if (project) {
			[docControl addDocument: project];
			[project makeWindowControllers];
			[project showWindows];
		}
	}
}

- (IBAction)spriteBankNew:(id)sender
{
	NSDocumentController *docControl = [NSDocumentController sharedDocumentController];
	NSError **err;
	NSDocument *doc = [docControl makeUntitledDocumentOfType:@"SLUDGE Sprite Bank" error:err];
	[docControl addDocument: doc];
	[doc makeWindowControllers];
	[doc showWindows];
}
- (IBAction)floorNew:(id)sender
{
	NSDocumentController *docControl = [NSDocumentController sharedDocumentController];
	NSError **err;
	NSDocument *doc = [docControl makeUntitledDocumentOfType:@"SLUDGE Floor" error:err];
	[docControl addDocument: doc];
	[doc makeWindowControllers];
	[doc showWindows];
}

- (IBAction)compileMenu:(id)sender
{
	[[[NSDocumentController sharedDocumentController] currentDocument] compile];
}
- (IBAction)projectPrefsMenu:(id)sender{
	[[[NSDocumentController sharedDocumentController] currentDocument] showProjectPrefs];
}



/*
OSStatus RegisterMyHelpBook(void)
{
    CFBundleRef myApplicationBundle;
    CFURLRef myBundleURL;
    FSRef myBundleRef;
    OSStatus err = noErr;
	
    myApplicationBundle = NULL;
    myBundleURL = NULL;
	
    myApplicationBundle = CFBundleGetMainBundle();// 1
		if (myApplicationBundle == NULL) {err = fnfErr; goto bail;}
		
	myBundleURL = CFBundleCopyBundleURL(myApplicationBundle);// 2
		if (myBundleURL == NULL) {err = fnfErr; goto bail;}
			
	if (!CFURLGetFSRef(myBundleURL, &myBundleRef)) 
		err = fnfErr;// 3
				
	if (err == noErr) err = AHRegisterHelpBook(&myBundleRef);// 4
		return err;
					
bail: return err;

}
*/

OSStatus MyGotoHelpPage (CFStringRef pagePath, CFStringRef anchorName)
{
    CFBundleRef myApplicationBundle = NULL;
    CFStringRef myBookName = NULL;
    OSStatus err = noErr;
	
    myApplicationBundle = CFBundleGetMainBundle();// 1
		if (myApplicationBundle == NULL) {err = fnfErr; goto bail;}// 2
		
		
		
		myBookName = CFBundleGetValueForInfoDictionaryKey(// 3
														  myApplicationBundle,
														  CFSTR("CFBundleHelpBookName"));
		if (myBookName == NULL) {err = fnfErr; goto bail;}
				
		if (CFGetTypeID(myBookName) != CFStringGetTypeID()) {// 4
			err = paramErr;
		}
		
		if (err == noErr)
			err = AHGotoPage (myBookName, pagePath, anchorName);// 5
			return err;
			
bail: return err;
			
}

- (IBAction)helpMenu:(id)sender
{
	MyGotoHelpPage(NULL, NULL);
}

- (BOOL)validateMenuItem:(id <NSMenuItem>)menuItem {
	if ([menuItem tag] == 1000)  {
		// This is project stuff
		if (! [[[[NSDocumentController sharedDocumentController] currentDocument] fileType] isEqualToString:@"SLUDGE Project file"])
			return false;
	}
	return true;
}


/* Called when the internal event loop has just started running */
- (void) applicationDidFinishLaunching: (NSNotification *) note
{

}




- (IBAction)prefKeepImagesClick:(id)sender
{
	programSettings.compilerKillImages = ! [prefKeepImages state];
}
- (IBAction)prefWriteStringsClick:(id)sender
{
	programSettings.compilerWriteStrings = [prefWriteStrings state];
}
- (IBAction)prefVerboseClick:(id)sender
{
	programSettings.compilerVerbose = [prefVerbose state];
}


@end

const char * getTempDir () {
	return [NSTemporaryDirectory() UTF8String];
}


bool errorBox (const char * head, const char * msg) {
	NSRunAlertPanel ([NSString stringWithUTF8String: head], [NSString stringWithUTF8String: msg], NULL, NULL, NULL);
	return false;
}

int main(int argc, char *argv[])
{
	return NSApplicationMain (argc, (const char **) argv);
}
