struct zPanel {
	unsigned short theColour;
	unsigned short yCutOff;
};

BOOL processZBufferData ();
void saveZBufferFile (char * name);
BOOL loadZBufferFile (char * name);
LRESULT CALLBACK LayerSettingsFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL setZBufferClick (int x, int y);