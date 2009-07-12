struct myBitmapInfo {
	BITMAPINFOHEADER bmiHeader;
	DWORD bitMask[3];
};

bool InitApplication (HINSTANCE hInstance);
bool InitInstance (HINSTANCE hInstance, int nCmdShow);
void setLoadedFile (char * t);