/*   SDLMain.m - main entry point for our Cocoa-ized SDL app
       Initial Version: Darrell Walisser <dwaliss1@purdue.edu>

*/

#import "SDL.h"
#import "OSXMain.h"
#import <sys/param.h> /* for MAXPATHLEN */
#import <unistd.h>

#include "platform-dependent.h"
#include "STRINGY.H"

#import "Start.h"

extern char *bundleFolder;


static int    gArgc;
static char  **gArgv;
static bool   gFinderLaunch;
static bool   gCalledAppMainline = false;

static bool usingCocoa = false;

static NSString *getApplicationName(void)
{
    NSDictionary *dict;
    NSString *appName = 0;

    /* Determine the application name */
    dict = (NSDictionary *)CFBundleGetInfoDictionary(CFBundleGetMainBundle());
	//[dict setValue:@"SLUDGE" forKey: @"CFBundleName"];
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
	if (! usingCocoa) {
		/* Post a SDL_QUIT event */
		SDL_Event event;
		event.type = SDL_QUIT;
		SDL_PushEvent(&event);
	} else {
		[super terminate:sender];
	}
}

- (void)sendEvent:(NSEvent *)anEvent {
	// Are we using Cocoa? If not, eat the keypresses unless the Command key is involved.
	if(! usingCocoa && (NSKeyDown == [anEvent type] || NSKeyUp == [anEvent type])) {
		if( [anEvent modifierFlags] & NSCommandKeyMask) {
			// But we handle Command+F, so don't pass that on.
			if (! [[anEvent charactersIgnoringModifiers] isEqualToString: @"f"])
				[super sendEvent: anEvent];
		}
	} else
		[super sendEvent: anEvent];
}

@end

/* The main class of the application, the application's delegate */
@implementation SDLMain

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
	
    aRange = [[aMenu title] rangeOfString:@"Sludge"];
    if (aRange.length != 0)
        [aMenu setTitle: [[aMenu title] stringByReplacingRange:aRange with:appName]];
	
	
    enumerator = [[aMenu itemArray] objectEnumerator];
    while ((menuItem = [enumerator nextObject]))
    {
        aRange = [[menuItem title] rangeOfString:@"Sludge"];
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
- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
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
	
	NSBundle *bundle;
	bundle = [NSBundle bundleForClass: [self class]];
	bundleFolder = joinStrings ([[bundle resourcePath] UTF8String], "/");
	
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
    buffer = (unichar *) NSAllocateMemoryPages(bufferSize*sizeof(unichar));
    
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
    NSApplicationMain (argc, (const char **) argv);
    return 0;
}

// ************************************************************************//
//
//   And here we have the Mac-versions of the platform specific functions  //
//
// ************************************************************************//


int showSetupWindow() {
	int result;
	
	usingCocoa = true;

	StartController *start;
	start = [[StartController alloc] init];
	
	result = [[SDLApplication sharedApplication] runModalForWindow: [start window]];
	
	[start release];
		
	usingCocoa = false;
	
	return result;
}


char * grabFileName () {
	
	int result;
	
	usingCocoa = true;

    NSArray *fileTypes = [NSArray arrayWithObject:@"slg"];
    NSOpenPanel *oPanel = [NSOpenPanel openPanel];
	 
    [oPanel setAllowsMultipleSelection:NO];
    result = [oPanel runModalForDirectory:NSHomeDirectory()
									 file:nil types:fileTypes];
	
	usingCocoa = false;
	
	if( result == NSOKButton && [[oPanel filenames] count] > 0 ) {
		NSString *filename = [[oPanel filenames] objectAtIndex: 0];
		return copyString( [filename UTF8String] );
    }
	
	return NULL;
	
}


void msgBox (const char * head, const char * msg) {
// stringWithUTF8String?
	usingCocoa = true;
	NSRunAlertPanel ([NSString stringWithCString: head encoding: NSUTF8StringEncoding], [NSString stringWithCString: msg encoding: NSUTF8StringEncoding], NULL, NULL, NULL);
	usingCocoa = false;
}

int msgBoxQuestion (const char * head, const char * msg) {
	usingCocoa = true;
	if (NSRunAlertPanel ([NSString stringWithCString: head encoding: NSUTF8StringEncoding], [NSString stringWithCString: msg encoding: NSWindowsCP1250StringEncoding], @"No", @"Yes", NULL) == NSAlertDefaultReturn) {
		usingCocoa = false;
		return false;
	}
	usingCocoa = false;	
	return true;
}

void changeToUserDir () {
	char appsupport_path[1024];
	FSRef foundRef;
	
	// Find the Application Support Folder  (or should it be kPreferencesFolderType?) and go there
	FSFindFolder (kUserDomain, kApplicationSupportFolderType , kDontCreateFolder, &foundRef);
	FSRefMakePath( &foundRef, (Uint8 *) appsupport_path, 1024);
	chdir (appsupport_path);
}

uint32_t launch(char * file) {
	if (file[0] == 'h' &&
		file[1] == 't' &&
		file[2] == 't' &&
		file[3] == 'p' &&
		file[4] == ':') {
		
		if ([[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString: [NSString stringWithUTF8String: file]]])
			return 69;
		
	} else {
		
		if ([[NSWorkspace sharedWorkspace] openFile: [NSString stringWithUTF8String: file]])
			return 69;
	}	
	return 0;
}

bool defaultUserFullScreen() {
	return true;
}
