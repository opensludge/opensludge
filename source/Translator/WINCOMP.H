#ifndef _WIN32
#define HWND int
#endif

#define MESS(id,m,w,l) SendDlgItemMessage(mainWin,id,m,(WPARAM)w,(LPARAM)l)

char * getFileFromBox (int i);
void drawLogo (HWND hDlg, int pic, int x, int y);
