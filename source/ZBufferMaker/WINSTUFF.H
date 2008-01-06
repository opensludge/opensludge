struct myBitmapInfo {
	BITMAPINFOHEADER bmiHeader;
	DWORD bitMask[3];
};

BOOL InitApplication (HINSTANCE hInstance);
BOOL InitInstance (HINSTANCE hInstance, int nCmdShow);
char * copyString (char * incoming);