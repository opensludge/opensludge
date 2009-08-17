extern int numberOfValidTranslations;

void registerTranslationFile (char * filename);
bool addAllTranslationData (stringArray * theSA, FILE * mainFile);
void addTranslationIDTable (FILE * mainFile, char * name);

void clearTranslations ();
