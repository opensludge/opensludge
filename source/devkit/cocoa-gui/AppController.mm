#import <sys/stat.h>
#import <unistd.h>

#import "AppController.h"

#include "project.h"
#include "settings.h"
#include "macstuff.h"
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
		int numFiles = 0;
		doNewProject ([path UTF8String], 0, &numFiles);
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
- (IBAction)translationNew:(id)sender
{
	NSDocumentController *docControl = [NSDocumentController sharedDocumentController];
	NSError **err;
	NSDocument *doc = [docControl makeUntitledDocumentOfType:@"SLUDGE Translation file" error:err];
	[docControl addDocument: doc];
	[doc makeWindowControllers];
	[doc showWindows];
}
- (IBAction)zBufferNew:(id)sender
{
	NSDocumentController *docControl = [NSDocumentController sharedDocumentController];
	NSError **err;
	NSDocument *doc = [docControl makeUntitledDocumentOfType:@"SLUDGE zBuffer" error:err];
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

}*/

#ifndef GNUSTEP
OSStatus MyGotoHelpPage (CFStringRef pagePath, CFStringRef anchorName)
{
    CFBundleRef myApplicationBundle = NULL;
    CFStringRef myBookName = NULL;
    CFURLRef myBundleURL;
    FSRef myBundleRef;
    OSStatus err = noErr;
	
    myApplicationBundle = CFBundleGetMainBundle();
	if (myApplicationBundle == NULL) {
		err = fnfErr; goto bail;
	}
		
	myBundleURL = CFBundleCopyBundleURL(myApplicationBundle);
	if (myBundleURL == NULL) {
		err = fnfErr; goto bail;
	}
	
	if (!CFURLGetFSRef(myBundleURL, &myBundleRef)) {
		err = fnfErr; goto bail;
	}
	
	err = AHRegisterHelpBook(&myBundleRef);
	if (err != noErr) {
		fprintf (stderr, "Error registering help book. %d", err);
		goto bail;
	}
							
	myBookName = CFBundleGetValueForInfoDictionaryKey(myApplicationBundle,
														  CFSTR("CFBundleHelpBookName"));
	if (myBookName == NULL) {
		err = fnfErr; goto bail;
	}
				
	if (CFGetTypeID(myBookName) != CFStringGetTypeID()) {
		err = paramErr; goto bail;
	}

	err = AHGotoPage (myBookName, pagePath, anchorName);
					
bail: return err;
			
}
#endif

- (IBAction)helpMenu:(id)sender
{
#ifndef GNUSTEP
	MyGotoHelpPage(NULL, NULL);
#endif
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

void saveIniFile() {
#ifdef GNUSTEP
	chdir (getenv ("HOME"));
	mkdir (".sludge", 0000777);
	chdir (".sludge");
#else
	unsigned char appsupport_path[1024];
	FSRef foundRef;
	
	// Find the preference folder and go there
	FSFindFolder (kUserDomain, kPreferencesFolderType , kDontCreateFolder, &foundRef);	
	FSRefMakePath( &foundRef, appsupport_path, 1024);
	chdir ((char *) appsupport_path);
#endif
	mkdir ("SLUDGE Development Kit", 0000777);
	chdir ("SLUDGE Development Kit");
	
	FILE * fp = fopen ("SLUDGE.ini", "wb");
	
	fprintf (fp, "KillImages=%d\n", programSettings.compilerKillImages);
	fprintf (fp, "WriteStrings=%d\n", programSettings.compilerWriteStrings);
	fprintf (fp, "Verbose=%d\n", programSettings.compilerVerbose);
	fprintf (fp, "SearchSensitive=%d\n", programSettings.searchSensitive);
	fclose (fp);
}

- (IBAction)prefKeepImagesClick:(id)sender
{
	programSettings.compilerKillImages = ! [prefKeepImages state];
	saveIniFile();
}
- (IBAction)prefWriteStringsClick:(id)sender
{
	programSettings.compilerWriteStrings = [prefWriteStrings state];
	saveIniFile();
}
- (IBAction)prefVerboseClick:(id)sender
{
	programSettings.compilerVerbose = [prefVerbose state];
	saveIniFile();
}


@end

void readIniFile () {
#ifdef GNUSTEP
	chdir (getenv ("HOME"));
	mkdir (".sludge", 0000777);
	chdir (".sludge");
#else
	unsigned char appsupport_path[1024];
	FSRef foundRef;

	// Find the preference folder and go there
	FSFindFolder (kUserDomain, kPreferencesFolderType , kDontCreateFolder, &foundRef);	
	FSRefMakePath( &foundRef, appsupport_path, 1024);
	chdir ((char *) appsupport_path);
#endif
	mkdir ("SLUDGE Development Kit", 0000777);
	chdir ("SLUDGE Development Kit");

	FILE * fp = fopen ("SLUDGE.ini", "rb");
	
	programSettings.compilerKillImages = 0;
	programSettings.compilerWriteStrings = 0;
	programSettings.compilerVerbose = 1;
	programSettings.searchSensitive = 0;
		
	if (fp) {
		char lineSoFar[257] = "";
		char secondSoFar[257] = "";
		unsigned char here = 0;
		char readChar = ' ';
		bool keepGoing = true;
		bool doingSecond = false;
		
		do {
			readChar = fgetc (fp);
			if (feof (fp)) {
				readChar = '\n';
				keepGoing = false;
			}
			switch (readChar) {
				case '\n':
				case '\r':

					fprintf (fp, "KillImages=%d\n", programSettings.compilerKillImages);
					fprintf (fp, "WriteStrings=%d\n", programSettings.compilerWriteStrings);
					fprintf (fp, "Verbose=%d\n", programSettings.compilerVerbose);
					fprintf (fp, "SearchSensitive=%d\n", programSettings.searchSensitive);
					
					
					if (doingSecond) {
						if (strcmp (lineSoFar, "KillImages") == 0)
						{
							programSettings.compilerKillImages = stringToInt (secondSoFar);
						}
						else if (strcmp (lineSoFar, "WriteStrings") == 0)
						{
							programSettings.compilerWriteStrings = stringToInt (secondSoFar);
						}
						else if (strcmp (lineSoFar, "Verbose") == 0)
						{
							programSettings.compilerVerbose = stringToInt (secondSoFar);
						}
						else if (strcmp (lineSoFar, "SearchSensitive") == 0)
						{
							programSettings.searchSensitive = stringToInt (secondSoFar);
						}
					}
					here = 0;
					doingSecond = false;
					lineSoFar[0] = 0;
					secondSoFar[0] = 0;
					break;
					
				case '=':
					doingSecond = true;
					here = 0;
					break;
					
				default:
					if (doingSecond) {
						secondSoFar[here ++] = readChar;
						secondSoFar[here] = 0;
					} else {
						lineSoFar[here ++] = readChar;
						lineSoFar[here] = 0;
					}
					break;
			}
		} while (keepGoing);
		
		fclose (fp);
	}
}


int main(int argc, char *argv[])
{
	
	readIniFile ();
	
	return NSApplicationMain (argc, (const char **) argv);
}
