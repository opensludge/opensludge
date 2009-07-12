#ifndef HWND
#define HWND int
#endif

enum typeOfFile {FILETYPE_UNKNOWN, FILETYPE_SCRIPT, FILETYPE_CONST, FILETYPE_TRANS};

void gotoSourceRoot ();
void runProg (char * prog, char * flags, char * filename);
void editFile (char * me, HWND hDlg);
void gotoSite (HWND hDlg, char * url);

void checkForUpgrades (HWND h);
void launchOtherKitProgram (int i, HWND h);
void verifyOtherKitPrograms ();

void cleanUpFiles ();