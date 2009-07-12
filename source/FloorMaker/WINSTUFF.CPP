#ifdef WIN32

#include <windows.h>
#include "winstuff.h"
#include "winterface.h"
#include "backdrop.h"
#include "line.h"
#include "data.h"
#include "moreio.h"

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

#define SHOWFLOOR 0x789A
#define HIDEFLOOR (0x10000 - SHOWFLOOR)
#define SHOWSPLIT 0xA483
#define HIDESPLIT (0x10000 - SHOWSPLIT)
#define SHOWMOVE 0xF268
#define HIDEMOVE (0x10000 - SHOWMOVE)

//extern inputType input;

HINSTANCE hInst;  				// Handle of the main instance
HWND hMainWindow;
myBitmapInfo screenBMPInfo; // bitmap information/palette
HBITMAP videoBuffer;        // handle to the buffer bitmap
HDC bufferDC;               // handle to the memory DC containing the
							// buffer bitmap
unsigned short int * screen;
unsigned short int * back;

const char * windowName = "SLUDGE Floor Maker";
const char * className = "SLUDGE_FLOOR32";
char * loadedFile = NULL;

int mouseX = WINWIDTH / 2, mouseY = WINHEIGHT / 2, screenMouseX = WINWIDTH/2, screenMouseY = WINHEIGHT/2;
int lastMouseX = 320, lastMouseY = 200;
int startLineX = 320, startLineY = 200;
bool draggingSplit = false;
bool draggingRight = false;
bool markVertices = false, speedScroll = false;
bool notSaved = false;

HCURSOR blokeyCursor;
HCURSOR snapCursor;
HCURSOR noWayCursor;
HCURSOR arrowsCursor;
HCURSOR moverCursor;

HMENU myMenu;
int currentMode = 0;

//----------------------------------------------

void alert (char * txt) {
	MessageBox (hMainWindow, txt, windowName, MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
}

void setLoadedFile (char * t) {
	if (loadedFile) delete loadedFile;
	loadedFile = t ? copyString (t) : NULL;
}

bool ask (char * txt) {
	return MessageBox (hMainWindow, txt, windowName, MB_YESNO | MB_APPLMODAL | MB_SETFOREGROUND | MB_ICONQUESTION) == IDYES;
}

bool abandon () {
	return notSaved ? ask ("Sure you want to abandon your changes to this floor?") : true;
}

bool fatal (char * a, char * b = NULL) {
	alert (a);
	b;
	return false;
}

void updateTitle () {
	char buff[300];
	int a = 0;
	if (loadedFile) {
		int b = 0;
		while (loadedFile[b]) {
			if (loadedFile[b] == '\\') a = b + 1;
			b ++;
		}
	}
	sprintf (buff, "SLUDGE Floor Maker: %s%s", notSaved ? "* " : "",
		loadedFile ? loadedFile + a : "Untitled");
	SetWindowText (hMainWindow, buff);
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
									 myMenu,
									 hInstance, NULL);

		  hMainWindow = hWnd;

		  if (! hWnd) return (false);

		  ShowWindow (hWnd, nCmdShow);
		  UpdateWindow (hWnd);

		  return (true);
}

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

		blokeyCursor	= LoadCursor (hInst, MAKEINTRESOURCE (100));
		snapCursor		= LoadCursor (hInst, MAKEINTRESOURCE (101));
		noWayCursor		= LoadCursor (hInst, MAKEINTRESOURCE (102));
		arrowsCursor	= LoadCursor (hInst, MAKEINTRESOURCE (103));
		moverCursor		= LoadCursor (hInst, MAKEINTRESOURCE (104));

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

void changeMode (int newMode) {
	if (currentMode) CheckMenuItem (myMenu, currentMode, MF_UNCHECKED);
	CheckMenuItem (myMenu, newMode, MF_CHECKED);
	currentMode = newMode;
}

void toggleMenuOption (int theOption, bool & changeMe) {
	changeMe = ! changeMe;
	CheckMenuItem (myMenu, theOption, changeMe ? MF_CHECKED : MF_UNCHECKED);
}

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

void scrollAtEdges () {
	int h = 2;
	if (screenMouseX > WINWIDTH - 20) {
		xShift += 10;
	} else if (screenMouseX < 20) {
		xShift -= 10;
	} else {
		h --;
	}

	if (screenMouseY > WINHEIGHT - 20) {
		yShift += 10;
	} else if (screenMouseY < 20) {
		yShift -= 10;
	} else {
		h --;
	}
	
	if (h) fixShift ();	
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int i;
	static OPENFILENAME ofn;
	static char path[MAX_PATH];
	char file[MAX_PATH];

	switch (message) {
		case WM_CREATE:
		if (! initVideo (hWnd)) fatal ("Can't create video controller");
		CenterWindow (hWnd, GetDesktopWindow ());
		ShowWindow (hWnd, SW_SHOWNORMAL);
		GetCurrentDirectory(MAX_PATH,path);
		memset (&ofn, 0, sizeof (ofn));
		ofn.lStructSize = sizeof (ofn);
		ofn.hwndOwner = hWnd;
		ofn.hInstance = hInst;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrInitialDir = path;
		ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;
		updateTitle ();
		changeMode (ID_MODE_DEFINE);
		toggleMenuOption (ID_VIEW_MARKVERT, markVertices);
		drawSoFar (SHOWFLOOR);
		backDropToScreenBuffer ();
		break;

		case WM_COMMAND:
		draggingSplit = false;
		switch (LOWORD (wParam)) {
			case ID_MODE_DEFINE:
			case ID_MODE_SPLIT:
			case ID_MODE_MOVE:
			case ID_MODE_KILLVERTEX:
			case ID_MODE_SPLITLINE:
			changeMode (LOWORD (wParam));
			break;
			
			case ID_EXIT:
			case IDCANCEL:
			PostQuitMessage (0);
			break;
			
			case ID_IMAGE_CLEAR:
			blankScreen ();
			drawSoFar (SHOWFLOOR);
			backDropToScreenBuffer ();
			break;

			case ID_FLOOR_NEW:
			setLoadedFile (NULL);
			drawSoFar (HIDEFLOOR);
			backDropToScreenBuffer ();
			noFloor ();
			updateTitle ();
			break;

			case ID_IMAGE_ADD:
			file[0] = NULL;
			ofn.lpstrFilter = "TGA image files (*.TGA)\0*.tga\0\0";
			ofn.lpstrFile = file;
			if (GetOpenFileName (& ofn)) {
				memcpy (path, file, ofn.nFileOffset);
				path[ofn.nFileOffset - 1] = 0;
				loadBackDrop (file);
				drawSoFar (SHOWFLOOR);
				backDropToScreenBuffer ();
			}
			break;
			
			case ID_FLOOR_SAVE:
			if (loadedFile) {
				saveToFile (loadedFile);
				break;
			}
			// No break here!

			case ID_FLOOR_SAVE_AS:
			file[0] = NULL;
			ofn.lpstrFilter = "SLUDGE floor files (*.FLO)\0*.flo\0\0";
			ofn.lpstrFile = file;
			if (GetSaveFileName (& ofn)) {
				memcpy (path, file, ofn.nFileOffset);
				path[ofn.nFileOffset - 1] = 0;
				if (loadedFile) delete loadedFile;
				fixExtension (file, "flo");
				loadedFile = copyString (file);
				saveToFile (file);
				updateTitle ();
			}
			break;
			
			case ID_FLOOR_LOAD:
			file[0]=NULL;
			ofn.lpstrFilter="SLUDGE floor files (*.FLO)\0*.flo\0\0";
			ofn.lpstrFile=file;
			if (GetOpenFileName (& ofn)) {
				memcpy (path, file, ofn.nFileOffset);
				path[ofn.nFileOffset - 1] = 0;
				drawSoFar (HIDEFLOOR);
				loadFromFile (file);
				if (loadedFile) delete loadedFile;
				loadedFile = copyString (file);
				drawSoFar (SHOWFLOOR);
				backDropToScreenBuffer ();
				updateTitle ();
			}
			break;
			
			case ID_VIEW_MARKVERT:
			drawSoFar (HIDEFLOOR);
			toggleMenuOption (ID_VIEW_MARKVERT, markVertices);
			drawSoFar (SHOWFLOOR);
			backDropToScreenBuffer ();
			break;

			case ID_VIEW_SPEEDSCROLL:
			toggleMenuOption (ID_VIEW_SPEEDSCROLL, speedScroll);
			break;
		}
		break;

		case WM_PAINT:
		screenBufferToWindow ();
		return (DefWindowProc(hWnd, message, wParam, lParam));
		break;

		case WM_LBUTTONDOWN:
		switch (currentMode) {
			case ID_MODE_DEFINE:
			{
				bool keepGoing = true;
				drawSoFar (HIDEFLOOR);
				SetCursor (snapCursor);	
				while (keepGoing) {
					keepGoing = false;
					i = addVertex (mouseX, mouseY);
					switch (i) {
						case 0:
						fatal ("Out of memory... can't add vertex");
						break;
						
						case 3:
						alert ("That vertex is already used in this polygon, but isn't the start point. Not added.");
						break;
										
						case 2:
						if (ask ("Can't add another vertex as the floor is already complete... do you want to start a NEW polygon at this point?")) {
							addPoly ();
							keepGoing = true;
						}
						break;
					}
				}
				drawSoFar (SHOWFLOOR);
				backDropToScreenBuffer ();
			}
			break;
	
			case ID_MODE_KILLVERTEX:
			drawSoFar (HIDEFLOOR);
			killVertex (mouseX, mouseY);
			drawSoFar (SHOWFLOOR);
			backDropToScreenBuffer ();
			SetCursor (noWayCursor);
			break;
			
			case ID_MODE_SPLITLINE:
			case ID_MODE_SPLIT:
			SetCursor (snapToClosest (mouseX, mouseY) ? snapCursor : noWayCursor);
			if (snapToClosest (mouseX, mouseY)) {
				draggingSplit = true;
				startLineX = mouseX;
				startLineY = mouseY;
				drawLine (mouseX, mouseY, mouseX, mouseY, SHOWSPLIT);
			}
			break;		

			case ID_MODE_MOVE:
			if (snapToClosest (mouseX, mouseY)) {
				SetCursor (arrowsCursor);
				draggingSplit = true;
				startLineX = mouseX;
				startLineY = mouseY;
				drawLine (mouseX, mouseY, mouseX, mouseY, SHOWMOVE);
			} else {
				SetCursor (noWayCursor);
			}
			break;		
		}
		break;
		
		case WM_RBUTTONDOWN:
		if (! draggingSplit) {
			startLineX = screenMouseX;
			startLineY = screenMouseY;
			SetCursor (moverCursor);
			draggingRight = true;
		}
		break;

		case WM_RBUTTONUP:
		if (draggingRight) {
			draggingRight = false;
		}
		// No break;
		
		case WM_LBUTTONUP:
		switch (currentMode) {
			case ID_MODE_DEFINE:
			SetCursor (snapToClosest (mouseX, mouseY) ? snapCursor : blokeyCursor);
			break;
			
			case ID_MODE_KILLVERTEX:
			SetCursor (snapToClosest (mouseX, mouseY) ? snapCursor : noWayCursor);
			break;
			
			case ID_MODE_SPLIT:
			case ID_MODE_SPLITLINE:
			SetCursor (snapToClosest (mouseX, mouseY) ? snapCursor : noWayCursor);
			if (draggingSplit) {
				drawLine (startLineX, startLineY, mouseX, mouseY, HIDESPLIT);
				drawSoFar (HIDEFLOOR);
				if (currentMode == ID_MODE_SPLIT) {
					splitPoly (startLineX, startLineY, mouseX, mouseY);
				} else {
					splitLine (startLineX, startLineY, mouseX, mouseY);
				}
				drawSoFar (SHOWFLOOR);
				backDropToScreenBuffer ();
				draggingSplit = false;
			}
			break;
			
			case ID_MODE_MOVE:
			if (draggingSplit) {

				drawLine (startLineX, startLineY, mouseX, mouseY, HIDEMOVE);

				// Snap to the closest point unless it's the point we
				// started from...
				
				SetCursor (arrowsCursor);
				drawSoFar (HIDEFLOOR);
				if (! moveVertices (startLineX, startLineY, mouseX, mouseY)) {
					alert ("Sorry - that vertex is already contained in one or more of the polygons you're changing. The point has not been moved.");
				}
				drawSoFar (SHOWFLOOR);
				backDropToScreenBuffer ();
				draggingSplit = false;
			} else {
				SetCursor (snapToClosest (mouseX, mouseY) ? noWayCursor : arrowsCursor);
			}
			break;
		}
		break;
		
		case WM_MOUSEMOVE:
		lastMouseX = mouseX;
		lastMouseY = mouseY;
		screenMouseX = LOWORD(lParam);
		screenMouseY = HIWORD(lParam);
		mouseX = screenMouseX + xShift;
		mouseY = screenMouseY + yShift;
		if (draggingRight) {
			xShift -= screenMouseX - startLineX;
			yShift -= screenMouseY - startLineY;
			if (! speedScroll) {
				startLineX = screenMouseX;
				startLineY = screenMouseY;
			}
			fixShift ();
			SetCursor (moverCursor);
			backDropToScreenBuffer ();
		} else {
			switch (currentMode) {
				case ID_MODE_DEFINE:
				SetCursor (snapToClosest (mouseX, mouseY) ? snapCursor : blokeyCursor);
				break;
			
				case ID_MODE_KILLVERTEX:
				SetCursor (snapToClosest (mouseX, mouseY) ? snapCursor : noWayCursor);
				break;
			
				case ID_MODE_SPLIT:
				case ID_MODE_SPLITLINE:
				SetCursor (snapToClosest (mouseX, mouseY) ? snapCursor : noWayCursor);
				if (draggingSplit) {
					scrollAtEdges ();
					drawLine (startLineX, startLineY, lastMouseX, lastMouseY, HIDESPLIT);
					drawLine (startLineX, startLineY, mouseX, mouseY, SHOWSPLIT);
					backDropToScreenBuffer ();
				}
				break;
			
				case ID_MODE_MOVE:
				if (draggingSplit) {
					scrollAtEdges ();
					bool sn = false;
					int tempX = mouseX, tempY = mouseY;
					if (snapToClosest (tempX, tempY)) {
						if (tempX != startLineX || tempY != startLineY) {
							mouseX = tempX;
							mouseY = tempY;
							sn = true;
						}
					}
					SetCursor (sn ? snapCursor : arrowsCursor);
					drawLine (startLineX, startLineY, lastMouseX, lastMouseY, HIDEMOVE);
					drawLine (startLineX, startLineY, mouseX, mouseY, SHOWMOVE);
					backDropToScreenBuffer ();
				} else {
					SetCursor (snapToClosest (mouseX, mouseY) ? arrowsCursor : noWayCursor);
				}
				break;
			}
		}
		break;

		case WM_CLOSE:
		if (ask ("Sure you want to quit?"))	DestroyWindow (hWnd);
		break;

		case WM_DESTROY:
		DeleteDC(bufferDC);
		PostQuitMessage(0);
		break;

		default:
		return (DefWindowProc(hWnd, message, wParam, lParam));
	};

	return (0);
}

bool InitApplication(HINSTANCE hInstance) {
	 WNDCLASS  wc;

	myMenu = LoadMenu (hInst, MAKEINTRESOURCE(1));

	 wc.style         = CS_OWNDC;
	 wc.lpfnWndProc   = (WNDPROC) WndProc;
	 wc.cbClsExtra    = 0;
	 wc.cbWndExtra    = 0;
	 wc.hInstance     = hInstance;
	 wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(10));
	 wc.hCursor       = NULL;
	 wc.hbrBackground = (HBRUSH__ *) GetStockObject (BLACK_BRUSH);
	 wc.lpszMenuName  = NULL;
	 wc.lpszClassName = className;

	 return RegisterClass (& wc);
}

#endif