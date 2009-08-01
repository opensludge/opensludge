/*
struct myBitmapInfo {
	BITMAPINFOHEADER bmiHeader;
	DWORD bitMask[3];
};
*/
#ifndef _WIN32
#ifndef HINSTANCE
#define HINSTANCE int
#endif
#endif

void setWindowName (const char * tx);
bool InitApplication (HINSTANCE hInstance);
bool InitInstance (HINSTANCE hInstance, const char *);
char * grabFileName ();
