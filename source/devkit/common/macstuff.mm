#ifdef GNUSTEP
extern "C"
{
#import <Cocoa/Cocoa.h>
}
#else
#import <Cocoa/Cocoa.h>
#endif

#import "macstuff.h"

const char * getTempDir () {
	return [NSTemporaryDirectory() UTF8String];
}

bool askAQuestion (const char * head, const char * msg) {
#ifdef __APPLE__	//FIXME: Should not be commented out.
	if (! NSRunAlertPanel ([NSString stringWithUTF8String: head], [NSString stringWithUTF8String: msg], @"Yes", @"No", NULL) == NSAlertDefaultReturn)
		return false;
#endif
	return true;
}


bool errorBox (const char * head, const char * msg) {
#ifdef __APPLE__	//FIXME: Should not be commented out.
	NSRunAlertPanel ([NSString stringWithUTF8String: head], [NSString stringWithUTF8String: msg], NULL, NULL, NULL);
#endif
	return false;
}

unsigned int stringToInt (char * s) {
	int i = 0;
	for (;;) {
		if (*s >= '0' && *s <= '9') {
			i *= 10;
			i += *s - '0';
			s ++;
		} else {
			return i;
		}
	}
}
