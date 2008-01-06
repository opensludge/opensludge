BOOL dumpFiles (FILE *, stringArray * & theSA);
BOOL saveStrings (FILE *, FILE *, stringArray *);
BOOL dumpFileInto (FILE * writer, char * thisFile);
int getFileType (char * filename);

enum {FILETYPE_UNKNOWN,
	  FILETYPE_MIDI,
	  FILETYPE_AUDIO,
	  FILETYPE_TGA,
	  FILETYPE_FLOOR,
	  FILETYPE_RAW};

#define audioFile(n) 		(getFileType(n) == FILETYPE_AUDIO)