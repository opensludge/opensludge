enum lineType {TYPE_NEW, TYPE_NONE, TYPE_TRANS, TYPE_SEARCH};

struct transLine {
	char * transFrom, * transTo;
	lineType type;
	transLine * next;
};

void loadFile (char * file);
void newFile ();
void updateStringList ();
void autoSelectContent (lineType type);
void selectedString (char * s);
void updateFromProject (char * filename);
void enableTranslationBox (int level, char * s);
void setChanged (BOOL bc);
void commitChanges ();
void saveToFile (char * filename, unsigned int);
BOOL trashProgress (char * doWhat);