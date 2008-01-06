BOOL saveGame (char * fname);
BOOL loadGame (char * fname);
loadedFunction * loadFunction (FILE * fp);
void saveFunction (loadedFunction * fun, FILE * fp);
