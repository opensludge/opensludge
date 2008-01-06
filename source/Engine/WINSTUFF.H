struct myBitmapInfo {
	BITMAPINFOHEADER bmiHeader;
	DWORD bitMask[3];
};

void setWindowName (const char * tx);
BOOL InitApplication (HINSTANCE hInstance);
BOOL InitInstance (HINSTANCE hInstance, const char *);
