extern FILE * bigDataFile;

void setFileIndices (FILE * fp, int, unsigned int);

unsigned int openFileFromNum (int num);
BOOL openSubSlice (int num);
BOOL openObjectSlice (int num);
char * getNumberedString (int value);

void finishAccess ();

