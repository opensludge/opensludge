enum whichPerc {P_TOP, P_BOTTOM};


void compileWindow ();
void percRect (int i, whichPerc whichBox = P_BOTTOM);

#define COMPMESS(id,m,w,l) SendDlgItemMessage(compWin,id,m,(WPARAM)w,(LPARAM)l)

