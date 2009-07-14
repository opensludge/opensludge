/*   SDLMain.m - main entry point for our Cocoa-ized SDL app
       Initial Version: Darrell Walisser <dwaliss1@purdue.edu>
       Non-NIB-Code & other changes: Max Horn <max@quendi.de>

    Feel free to customize this file to suit your needs
*/

#import "SDL.h"
#import "SDLMain.h"
#import <sys/param.h> /* for MAXPATHLEN */
#import <unistd.h>

/* For some reaon, Apple removed setAppleMenu from the headers in 10.4,
 but the method still is there and works. To avoid warnings, we declare
 it ourselves here. */
@interface NSApplication(SDL_Missing_Methods)
- (void)setAppleMenu:(NSMenu *)menu;
@end


static int    gArgc;
static char  **gArgv;
static bool   gFinderLaunch;
static bool   gCalledAppMainline = false;

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
    /* Post a SDL_QUIT event */
    SDL_Event event;
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
}
@end


/* The main class of the application, the application's delegate */
@implementation SDLMain

- (IBAction)prefsMenu:(id)sender
{
    printf ("prefs menu\n");
}

- (IBAction)newGame:(id)sender
{
    printf ("new game\n");
    
    NSRunAlertPanel (@"Get ready to blow up some... stuff!", 
        @"Click OK to begin total carnage. Click Cancel to prevent total carnage.", 	        		@"OK", @"Cancel", nil);
}

- (IBAction)openGame:(id)sender
{
    NSString *path = nil;
    NSOpenPanel *openPanel = [ NSOpenPanel openPanel ];
    
    if ( [ openPanel runModalForDirectory:nil
             file:@"SavedGame" types:nil ] ) {
             
        path = [ [ openPanel filenames ] objectAtIndex:0 ];
    }
    
    printf ("open game: %s\n", [ path cString ]);
}

- (IBAction)saveGame:(id)sender
{
    NSString *path = nil;
    NSSavePanel *savePanel = [ NSSavePanel savePanel ];
    
    if ( [ savePanel runModalForDirectory:nil
           file:@"SaveGameFile" ] ) {
            
        path = [ savePanel filename ];
    }
    
    printf ("save game: %s\n", [ path cString ]);
}

- (IBAction)saveGameAs:(id)sender
{
    printf ("save game as\n");
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

- (IBAction)help:(id)sender
{
	MyGotoHelpPage(NULL, NULL);
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
@end


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
int main (int argc, char **argv)
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
