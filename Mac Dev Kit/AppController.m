#import "SDL.h"
#import "AppController.h"
#import "ProjectController.h"

#include "Project.hpp"
//#include "Compiler.hpp"


#import <sys/param.h> /* for MAXPATHLEN */
#import <unistd.h>


static int    gArgc;
static char  **gArgv;
static bool   gFinderLaunch;
static bool   gCalledAppMainline = false;

static bool menusActivated = false;

static NSString *getApplicationName(void)
{
    NSDictionary *dict;
    NSString *appName = 0;

    /* Determine the application name */
    dict = (NSDictionary *)CFBundleGetInfoDictionary(CFBundleGetMainBundle());
    if (dict)
        appName = [dict objectForKey: @"CFBundleName"];
    
    if (![appName length])
        appName = [[NSProcessInfo processInfo] processName];

    return appName;
}

/* A helper category for NSString */
@interface NSString (ReplaceSubString)
- (NSString *)stringByReplacingRange:(NSRange)aRange with:(NSString *)aString;
@end

@interface SDLApplication : NSApplication
@end

@implementation SDLApplication
/* Invoked from the Quit menu item */
- (void)terminate:(id)sender
{
	[super terminate:sender];
}
@end

AppController *aC;

/* The main class of the application, the application's delegate */
@implementation AppController

-(id) init
{
	[super init];
	aC = self;
	return self;
}

- (IBAction)prefsMenu:(id)sender
{
    printf ("prefs menu\n");
}



- (IBAction)newProject:(id)sender
{    
    NSString *path = nil;
    NSSavePanel *savePanel = [ NSSavePanel savePanel ];
	[savePanel setTitle:@"New SLUDGE Project"];
	[savePanel setRequiredFileType:@"slp"];
    
    if ( [ savePanel runModalForDirectory:nil
									 file:nil ] ) {
		
        path = [ savePanel filename ];
		doNewProject ([path UTF8String]);
    }
	if (path) {
		[projectWindow makeKeyAndOrderFront:nil];
	}
}

- (IBAction)openProject:(id)sender
{
    NSString *path = nil;
    NSOpenPanel *openPanel = [ NSOpenPanel openPanel ];
 
    NSArray *fileTypes = [NSArray arrayWithObject:@"slp"];
	
    if ( [ openPanel runModalForDirectory:nil
             file:nil types:fileTypes ] ) {
             
        path = [ [ openPanel filenames ] objectAtIndex:0 ];
		loadProject ([path UTF8String]);
    }
	if (path) {
		[projectWindow makeKeyAndOrderFront:nil];
	}
}

- (IBAction)saveProject:(id)sender
{
	saveProject (NULL);
}

- (IBAction)saveProjectAs:(id)sender
{
    NSString *path = nil;
    NSSavePanel *savePanel = [ NSSavePanel savePanel ];
	[savePanel setTitle:@"New SLUDGE Project"];
	[savePanel setRequiredFileType:@"slp"];
    
    if ( [ savePanel runModalForDirectory:nil
									 file:nil ] ) {
		
        path = [ savePanel filename ];
		saveProject ([path UTF8String]);
    }
	if (path) {
		[projectWindow makeKeyAndOrderFront:nil];
	}
}

- (IBAction)compileMenu:(id)sender
{
	compileEverything();
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
	if ([menuItem tag] == 1000) 
		return menusActivated;
	return true;
}


/* Set the working directory to the .app's parent directory */
- (void) setupWorkingDirectory:(bool)shouldChdir
{
    if (shouldChdir)
    {
        char parentdir[MAXPATHLEN];
		CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
		CFURLRef url2 = CFURLCreateCopyDeletingLastPathComponent(0, url);
		if (CFURLGetFileSystemRepresentation(url2, true, (UInt8 *)parentdir, MAXPATHLEN)) {
	        assert ( chdir (parentdir) == 0 );   /* chdir to the binary app's parent */
		}
		CFRelease(url);
		CFRelease(url2);
	}

}


/* Fix menu to contain the real app name instead of "SDL App" */
- (void)fixMenu:(NSMenu *)aMenu withAppName:(NSString *)appName
{
    NSRange aRange;
    NSEnumerator *enumerator;
    NSMenuItem *menuItem;

    aRange = [[aMenu title] rangeOfString:@"SDL App"];
    if (aRange.length != 0)
        [aMenu setTitle: [[aMenu title] stringByReplacingRange:aRange with:appName]];

    enumerator = [[aMenu itemArray] objectEnumerator];
    while ((menuItem = [enumerator nextObject]))
    {
        aRange = [[menuItem title] rangeOfString:@"SDL App"];
        if (aRange.length != 0)
            [menuItem setTitle: [[menuItem title] stringByReplacingRange:aRange with:appName]];
        if ([menuItem hasSubmenu])
            [self fixMenu:[menuItem submenu] withAppName:appName];
    }
    [ aMenu sizeToFit ];
}



/*
 * Catch document open requests...this lets us notice files when the app
 *  was launched by double-clicking a document, or when a document was
 *  dragged/dropped on the app's icon. You need to have a
 *  CFBundleDocumentsType section in your Info.plist to get this message,
 *  apparently.
 *
 * Files are added to gArgv, so to the app, they'll look like command line
 *  arguments. Previously, apps launched from the finder had nothing but
 *  an argv[0].
 *
 * This message may be received multiple times to open several docs on launch.
 *
 * This message is ignored once the app's mainline has been called.
 */
- (bool)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
    const char *temparg;
    size_t arglen;
    char *arg;
    char **newargv;

    if (!gFinderLaunch)  /* MacOS is passing command line args. */
        return false;

    if (gCalledAppMainline)  /* app has started, ignore this document. */
        return false;

    temparg = [filename UTF8String];
    arglen = SDL_strlen(temparg) + 1;
    arg = (char *) SDL_malloc(arglen);
    if (arg == NULL)
        return false;

    newargv = (char **) realloc(gArgv, sizeof (char *) * (gArgc + 2));
    if (newargv == NULL)
    {
        SDL_free(arg);
        return false;
    }
    gArgv = newargv;

    SDL_strlcpy(arg, temparg, arglen);
    gArgv[gArgc++] = arg;
    gArgv[gArgc] = NULL;
    return true;
}


/* Called when the internal event loop has just started running */
- (void) applicationDidFinishLaunching: (NSNotification *) note
{
    int status;

    /* Set the working directory to the .app's parent directory */
    [self setupWorkingDirectory:gFinderLaunch];

    /* Set the main menu to contain the real app name instead of "SDL App" */
    [self fixMenu:[NSApp mainMenu] withAppName:getApplicationName()];

    /* Hand off to main application code */
    gCalledAppMainline = true;
    status = SDL_main (gArgc, gArgv);

    /* We're done, thank you for playing */
    exit(status);
}



- (void) updateFileListing {
	[projectFiles noteNumberOfRowsChanged];
}

// This is the project file list!
- (int)numberOfRowsInTableView:(NSTableView *)tv
{
	return fileListNum; 
}

- (id)tableView:(NSTableView *)tv
	objectValueForTableColumn:(NSTableColumn *)tableColumn
			row:(int)row
{
	NSString *v = [NSString stringWithUTF8String:getFileFromList(row)];
	return v;
}


@end

void updateFileListing() {
	[aC updateFileListing];
}

void activateMenus (bool on) {
	menusActivated = on;
}

const char * getTempDir () {
	return [NSTemporaryDirectory() UTF8String];
}



@implementation NSString (ReplaceSubString)

- (NSString *)stringByReplacingRange:(NSRange)aRange with:(NSString *)aString
{
    unsigned int bufferSize;
    unsigned int selfLen = [self length];
    unsigned int aStringLen = [aString length];
    unichar *buffer;
    NSRange localRange;
    NSString *result;

    bufferSize = selfLen + aStringLen - aRange.length;
    buffer = NSAllocateMemoryPages(bufferSize*sizeof(unichar));
    
    /* Get first part into buffer */
    localRange.location = 0;
    localRange.length = aRange.location;
    [self getCharacters:buffer range:localRange];
    
    /* Get middle part into buffer */
    localRange.location = 0;
    localRange.length = aStringLen;
    [aString getCharacters:(buffer+aRange.location) range:localRange];
     
    /* Get last part into buffer */
    localRange.location = aRange.location + aRange.length;
    localRange.length = selfLen - localRange.location;
    [self getCharacters:(buffer+aRange.location+aStringLen) range:localRange];
    
    /* Build output string */
    result = [NSString stringWithCharacters:buffer length:bufferSize];
    
    NSDeallocateMemoryPages(buffer, bufferSize);
    
    return result;
}

@end






#ifdef main
#  undef main
#endif


/* Main entry point to executable - should *not* be SDL_main! */
int main (int argc, char *argv[])
{
    /* Copy the arguments into a global variable */
    /* This is passed if we are launched by double-clicking */
    if ( argc >= 2 && strncmp (argv[1], "-psn", 4) == 0 ) {
        gArgv = (char **) SDL_malloc(sizeof (char *) * 2);
        gArgv[0] = argv[0];
        gArgv[1] = NULL;
        gArgc = 1;
        gFinderLaunch = YES;
    } else {
        int i;
        gArgc = argc;
        gArgv = (char **) SDL_malloc(sizeof (char *) * (argc+1));
        for (i = 0; i <= argc; i++)
            gArgv[i] = argv[i];
        gFinderLaunch = NO;
    }

    [SDLApplication poseAsClass:[NSApplication class]];
    NSApplicationMain (argc, argv);

    return 0;
}
