#ifdef WIN32

#include <windows.h>
#include "winstuff.h"
#include "winterface.h"
#include "backdrop.h"
#include "zbuffer.h"

int getRegInt (char * complete, int def) {
	HKEY gotcha;
	int r;
	int grab;
	unsigned long si = 4;
	
	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Software\\Hungry Software\\SLUDGE Compiler", 0, KEY_READ, & gotcha) != ERROR_SUCCESS) return def;
	r = RegQueryValueEx (gotcha, complete, NULL, NULL, (unsigned char *) & grab, & si);
	RegCloseKey (gotcha);
		
	return (r == ERROR_SUCCESS) ? grab : def;
}

int WINWIDTH = getRegInt ("utilityWidth", 640);
int WINHEIGHT = getRegInt ("utilityHeight", 480);

extern int HORZ_RES, VERT_RES, xShift, yShift;

//extern inputType input;

HINSTANCE hInst;  				// Handle of the main instance
HWND hMainWindow;
myBitmapInfo screenBMPInfo; // bitmap information/palette
HBITMAP videoBuffer;        // handle to the buffer bitmap
HDC bufferDC;               // handle to the memory DC containing the
							// buffer bitmap
unsigned short int * screen;
unsigned short int * back;

const char * windowName = "SLUDGE Z-Buffer Maker";
const char * className = "SLUDGE_ZBUFFMAKER_32";

int mouseX = WINWIDTH/2, mouseY = WINHEIGHT/2;
char * loadedFile = NULL;
bool draggingRight = false, speedScroll = false;
int startLineX, startLineY;

//----------------------------------------------

void alert (char * txt) {
	MessageBox (NULL, txt, "Z-Buffer Maker", MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
}

bool fatal (char * a, char * b = NULL) {
	alert (a);
	b;
	return false;
}

bool InitInstance (HINSTANCE hInstance, int nCmdShow) {
		  HWND hWnd;
		  RECT clientRect;

		  clientRect.top=100;
		  clientRect.left=100;
		  clientRect.right=100 + WINWIDTH;
		  clientRect.bottom=100 + WINHEIGHT;

		  AdjustWindowRect (& clientRect, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, true);

		  hInst = hInstance; // Store instance handle in our global variable
		  hWnd = CreateWindow(className, windowName, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
									 clientRect.left,
									 clientRect.top,
									 clientRect.right - clientRect.left,
									 clientRect.bottom - clientRect.top,
									 NULL, 
									 LoadMenu (hInst, MAKEINTRESOURCE(1)),
									 hInstance, NULL);

		  hMainWindow = hWnd;

		  if (! hWnd) {
					 return (false);
		  }

		  ShowWindow (hWnd, nCmdShow);
		  UpdateWindow (hWnd);

		  return (true);
}

HCURSOR normalCursor, moverCursor;

bool initVideo(HWND hWnd) {
	int i;
	HDC hdc;

	// Fill in bitmap header information

	screenBMPInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	screenBMPInfo.bmiHeader.biWidth=WINWIDTH;
	screenBMPInfo.bmiHeader.biHeight=-WINHEIGHT;
	screenBMPInfo.bmiHeader.biPlanes=1;
	screenBMPInfo.bmiHeader.biBitCount=16;
	screenBMPInfo.bmiHeader.biCompression=BI_BITFIELDS;
	screenBMPInfo.bmiHeader.biSizeImage=0;
	screenBMPInfo.bmiHeader.biXPelsPerMeter=0;
	screenBMPInfo.bmiHeader.biYPelsPerMeter=0;
	screenBMPInfo.bmiHeader.biClrUsed=256;
	screenBMPInfo.bmiHeader.biClrImportant=0;

	screenBMPInfo.bitMask[0]=0xF800;
	screenBMPInfo.bitMask[1]=0x07E0;
	screenBMPInfo.bitMask[2]=0x001F;

	hdc=GetDC(hWnd);

	// Attempt to create a memory DC for the buffer
	bufferDC=CreateCompatibleDC(hdc);

	if(bufferDC==NULL) return fatal ("Could not create compatible memory DC");

	// Get pointer and handle to buffer DIB
	screen=NULL;
	videoBuffer=CreateDIBSection(hdc,(BITMAPINFO *)&screenBMPInfo,
										  DIB_RGB_COLORS,(void **)&screen,NULL,0);

	// Select the buffer DIB into the memory device context
	i=(int)SelectObject(bufferDC,videoBuffer);

	if((i==NULL) || (i==GDI_ERROR)) {
		MessageBox(hWnd,"Could not select buffer DIB into device context",
					  "initVideo(...) error",MB_ICONSTOP | MB_OK);
		return(false);
	};
	ReleaseDC (hWnd, hdc);

	return(true);
}

bool CenterWindow (HWND hwndChild, HWND hwndParent) {
		  RECT    rChild, rParent, rWorkArea;
		  int     wChild, hChild, wParent, hParent;
		  int     xNew, yNew;
		  bool    bResult;

		  // Get the Height and Width of the child window
		  GetWindowRect (hwndChild, &rChild);
		  wChild = rChild.right - rChild.left;
		  hChild = rChild.bottom - rChild.top;

		  // Get the Height and Width of the parent window
		  GetWindowRect (hwndParent, &rParent);
		  wParent = rParent.right - rParent.left;
		  hParent = rParent.bottom - rParent.top;

		  // Get the limits of the 'workarea'
		  bResult = SystemParametersInfo(
					 SPI_GETWORKAREA,        // system parameter to query or set
					 sizeof(RECT),
					 &rWorkArea,
					 0);
		  if (!bResult) {
					 rWorkArea.left = rWorkArea.top = 0;
					 rWorkArea.right = GetSystemMetrics(SM_CXSCREEN);
					 rWorkArea.bottom = GetSystemMetrics(SM_CYSCREEN);
		  }

		  // Calculate new X position, then adjust for workarea
		  xNew = rParent.left + ((wParent - wChild) /2);
		  if (xNew < rWorkArea.left) {
					 xNew = rWorkArea.left;
		  } else if ((xNew+wChild) > rWorkArea.right) {
					 xNew = rWorkArea.right - wChild;
		  }

		  // Calculate new Y position, then adjust for workarea
		  yNew = rParent.top  + ((hParent - hChild) /2);
		  if (yNew < rWorkArea.top) {
					 yNew = rWorkArea.top;
		  } else if ((yNew+hChild) > rWorkArea.bottom) {
					 yNew = rWorkArea.bottom - hChild;
		  }

		  // Set it, and return
		  return SetWindowPos (hwndChild, NULL, xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

HMENU myMenu;
int currentMode = 0;
/*
void changeMode (int newMode) {
	if (currentMode) CheckMenuItem (myMenu, currentMode, MF_UNCHECKED);
	CheckMenuItem (myMenu, newMode, MF_CHECKED);
	currentMode = newMode;
}*/

void fixExtension (char * buff, char * ext) {
	int a = strlen (buff);
	if (buff[a - 4] != '.' ||
		(tolower (buff[a - 3]) != ext[0]) ||
		(tolower (buff[a - 2]) != ext[1]) ||
		(tolower (buff[a - 1]) != ext[2])) {
		buff[a] = '.';
		buff[a + 1] = ext[0];
		buff[a + 2] = ext[1];
		buff[a + 3] = ext[2];
		buff[a + 4] = NULL;
	}
}

void fixShift () {
	if (xShift < 0) xShift = 0;
	if (yShift < 0) yShift = 0;
	if (xShift > HORZ_RES - WINWIDTH) xShift = HORZ_RES - WINWIDTH;
	if (yShift > VERT_RES - WINHEIGHT) yShift = VERT_RES - WINHEIGHT;
}

void toggleMenuOption (int theOption, bool & changeMe) {
	changeMe = ! changeMe;
	CheckMenuItem (myMenu, theOption, changeMe ? MF_CHECKED : MF_UNCHECKED);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
//	int i;
	static OPENFILENAME ofn;
	static char path[MAX_PATH];
	char file[MAX_PATH];

	switch (message) {
		case WM_CREATE:
		if (! initVideo (hWnd)) fatal ("Can't create video controller");
		CenterWindow (hWnd, GetDesktopWindow());
		backDropToScreenBuffer ();
		myMenu = LoadMenu (hInst, MAKEINTRESOURCE(1));
		SetMenu (hWnd, myMenu);
		ShowWindow (hWnd, SW_RESTORE);
		GetCurrentDirectory(MAX_PATH,path);
		memset(&ofn,0,sizeof(ofn));
		ofn.lStructSize=sizeof(ofn);
		ofn.hwndOwner=hWnd;
		ofn.hInstance=hInst;
		ofn.nMaxFile=MAX_PATH;
		ofn.lpstrInitialDir=path;
		ofn.Flags=OFN_HIDEREADONLY|OFN_EXPLORER;
		break;

		case WM_COMMAND:
		switch (LOWORD (wParam)) {
/*			case ID_IMPORT:
			file[0]=NULL;
			ofn.lpstrFilter="TGA image files (*.TGA)\0*.tga\0\0";
			ofn.lpstrFile=file;
			if (GetOpenFileName(&ofn)) {
				memcpy(path,file,ofn.nFileOffset);
				path[ofn.nFileOffset-1]=0;
				loadBackDrop (file);
				processZBufferData ();
				backDropToScreenBuffer ();
				delete loadedFile;
				loadedFile = NULL;
			}
			break;
*/
			case ID_SAVE:
			if (loadedFile) {
				saveZBufferFile (loadedFile);
				break;
			}

			case ID_SAVE_AS:
			file[0]=NULL;
			if (loadedFile) strcpy (file, loadedFile);
			ofn.lpstrFilter="SLUDGE Z-buffer files (*.ZBU)\0*.zbu\0TGA image files (*.TGA)\0*.tga\0\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFile=file;
			if (GetSaveFileName(&ofn)) {
				memcpy(path,file,ofn.nFileOffset);
				path[ofn.nFileOffset-1]=0;
				if (ofn.nFilterIndex == 1) {
					fixExtension (file, "zbu");
					saveZBufferFile (file);
					delete loadedFile;
					loadedFile = copyString (file);
				} else {
					fixExtension (file, "tga");
					if (! saveBackdropToTGA (file)) {
						alert ("Couldn't save to TGA file");
					}
				}
			}
			break;

			case ID_OPEN_ZBUFFER:
			file[0]=NULL;
			ofn.lpstrFilter="SLUDGE Z-buffer files (*.ZBU)\0*.zbu\0TGA image files (*.TGA)\0*.tga\0\0";
			ofn.lpstrFile=file;
			ofn.nFilterIndex = 1;
			if (GetOpenFileName(&ofn)) {
				memcpy(path,file,ofn.nFileOffset);
				path[ofn.nFileOffset-1]=0;
				if (ofn.nFilterIndex == 1) {
					loadZBufferFile (file);
					delete loadedFile;
					loadedFile = copyString (file);
				} else {
					delete loadedFile;
					loadedFile = NULL;
					if (loadBackDrop (file)) {
						if (processZBufferData ()) {
						} else {
							alert ("Too many colours... can't use this TGA file as a z-buffer image.");
						}
					} else {
						alert ("Can't load TGA file.");
					}
				}
				backDropToScreenBuffer ();
			}
			break;

			case ID_EXIT:
			PostQuitMessage (0);
			break;
			
			case ID_VIEW_SPEEDSCROLL:
			toggleMenuOption (ID_VIEW_SPEEDSCROLL, speedScroll);
			break;
		}
		break;

		case WM_PAINT:
		backDropToScreenBuffer ();
		return (DefWindowProc(hWnd, message, wParam, lParam));

		case WM_LBUTTONDOWN:
		if (! draggingRight) {
			if (setZBufferClick (mouseX + xShift, mouseY + yShift))
				DialogBox(hInst, MAKEINTRESOURCE(LAYERSETTINGS), hWnd, (DLGPROC)LayerSettingsFunc);
		}
		return 1;
		
		case WM_RBUTTONDOWN:
		startLineX = mouseX;
		startLineY = mouseY;
		SetCursor (moverCursor);
		draggingRight = true;
		break;

		case WM_RBUTTONUP:
		if (draggingRight) {
			draggingRight = false;
			SetCursor (normalCursor);
		}
		break;
		
		case WM_MOUSEMOVE:
		mouseX = LOWORD(lParam);
		mouseY = HIWORD(lParam);
		if (draggingRight) {
			xShift -= mouseX - startLineX;
			yShift -= mouseY - startLineY;
			if (! speedScroll) {
				startLineX = mouseX;
				startLineY = mouseY;
			}
			fixShift ();
			SetCursor (moverCursor);
			backDropToScreenBuffer ();
		}
		return 1;

		case WM_DESTROY:
		DeleteDC(bufferDC);
		PostQuitMessage(0);
		break;

		default:
		return (DefWindowProc(hWnd, message, wParam, lParam));
	}

	return (0);
}

bool InitApplication(HINSTANCE hInstance) {
	 WNDCLASS  wc;
//	 HWND      hwnd;

	normalCursor	= LoadCursor (hInstance, MAKEINTRESOURCE (103));
	moverCursor		= LoadCursor (hInstance, MAKEINTRESOURCE (104));

	 wc.style         = CS_OWNDC;
	 wc.lpfnWndProc   = (WNDPROC) WndProc;
	 wc.cbClsExtra    = 0;
	 wc.cbWndExtra    = 0;
	 wc.hInstance     = hInstance;
	 wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(10));
	 wc.hCursor       = normalCursor;
	 wc.hbrBackground = (HBRUSH__ *) GetStockObject (BLACK_BRUSH);
	 wc.lpszMenuName  = NULL;
	 wc.lpszClassName = className;

	 return RegisterClass (& wc);
}

#endif