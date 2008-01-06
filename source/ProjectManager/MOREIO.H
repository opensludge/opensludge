unsigned int get2bytes (FILE * fp);
char * readString (FILE * fp);
void put2bytes (unsigned int numtoput, FILE * fp);
void put4bytes (long int i, FILE * fp);
void writeString (const char * txt, FILE * fp);
char * readText (FILE * fp);
char * grabWholeFile (char * theName);

