struct zPanel {
	unsigned short theColour;
	unsigned short yCutOff;
};

bool processZBufferData ();
void saveZBufferFile (char * name);
bool loadZBufferFile (char * name);
#ifdef WIN32
LRESULT CALLBACK LayerSettingsFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
#endif
bool setZBufferClick (int x, int y);