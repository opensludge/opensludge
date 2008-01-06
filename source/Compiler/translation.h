extern int numberOfValidTranslations;

void registerTranslationFile (char * filename);
BOOL addAllTranslationData (stringArray * theSA, FILE * mainFile);
void addTranslationIDTable (FILE * mainFile);
