bool saveGame (char * fname);
bool loadGame (char * fname);
loadedFunction * loadFunction (FILE * fp);
void saveFunction (loadedFunction * fun, FILE * fp);
