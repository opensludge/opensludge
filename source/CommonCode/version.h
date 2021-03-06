#define MAJOR_VERSION 2
#define MINOR_VERSION 2
#define RELEASE_VERSION 2
#define BUILD_VERSION 209
#define TEXT_VERSION "2.2.2"
#define WHOLE_VERSION (MAJOR_VERSION * 256 + MINOR_VERSION)	// This version
#define MINIM_VERSION (1 			 * 256 + 2)				// Earliest version of games the engine can run

#define COPYRIGHT_TEXT "\251 Hungry Software and contributors 2000-2014"

#define VERSION(a,b)	(a * 256 + b)

extern int gameVersion;
