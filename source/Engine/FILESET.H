extern FILE * bigDataFile;

void setFileIndices (FILE * fp, int, unsigned int);

unsigned int openFileFromNum (int num);
bool openSubSlice (int num);
bool openObjectSlice (int num);
char * getNumberedString (int value);

void finishAccess ();

