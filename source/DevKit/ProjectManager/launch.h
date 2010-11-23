#ifndef _WIN32
#define HWND int
#endif

void runProg (char * prog, char * flags, char * filename);
void editFile (char * me, HWND hDlg);
void gotoSite (HWND hDlg, char * url);
