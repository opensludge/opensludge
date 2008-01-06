#include "allfiles.h"

#include "winstuff.h"
#include "newfatal.h"
#include "sprites.h"
#include "sprbanks.h"
#include "fonttext.h"
#include "backdrop.h"
#include "sludger.h"
#include "cursors.h"
#include "objtypes.h"
#include "region.h"
#include "people.h"
#include "talk.h"
#include "vid.h"
#include "direct.h"
#include "sound.h"
#include "colours.h"
#include "specialSettings.h"

extern inputType input;
extern int sceneWidth, sceneHeight;
extern unsigned short int * * backDropImage;
extern int specialSettings;

HINSTANCE hInst;  				// Handle of the main instance
HWND hMainWindow;
myBitmapInfo screenBMPInfo;   // bitmap information/palette
HBITMAP videoBuffer;          // handle to the buffer bitmap
HDC bufferDC;                 // handle to the memory DC containing the
							  // buffer bitmap

BOOL veryBusyIndeed = FALSE;
unsigned short int * screen;
//unsigned short int * back;
int winWidth, winHeight;
BOOL runningFullScreen = FALSE;
//byte frameRate;

char * className = "SLUDGE_ENGINE_32";
HICON customIcon = NULL;

HICON makeCustomIcon (HINSTANCE ins) {
	int iconSize = sceneWidth * sceneHeight;
	int pip = 0, pip3 = 0;

	unsigned char * colourBits = new unsigned char[iconSize * 4];
	byte * outlineBits = new byte[(iconSize +7)/ 8];

	for (int y = 0; y < (iconSize+7) / 8; y ++)
		outlineBits[y] = 255;

	for (int y = 0; y < sceneHeight; y ++) {
		for (int x = 0; x < sceneWidth; x ++) {
			unsigned short c = backDropImage[y][x];
			outlineBits[pip/8] <<= 1;
			if (c != 63519) {
				colourBits[pip3++] = blueValue (c);
				colourBits[pip3++] = greenValue (c);
				colourBits[pip3++] = redValue (c);
				colourBits[pip3++] = 0;
			} else {
				colourBits[pip3++] = 0;
				colourBits[pip3++] = 0;
				colourBits[pip3++] = 0;
				colourBits[pip3++] = 0;
				outlineBits[pip/8] += 1;
			}
			pip ++;
		}
	}
	return CreateIcon (ins, sceneWidth, sceneHeight, 1, 32, outlineBits, (byte *) colourBits);
}

BOOL InitInstance (HINSTANCE hInstance, const char * windowName) {
	HWND hWnd;
	RECT clientRect;
	DWORD style;
	DWORD exStyle = 0;

	clientRect.top = 0;
	clientRect.left = 0;
	clientRect.right = winWidth;
	clientRect.bottom = winHeight;
	
	if (runningFullScreen) {
		style = WS_POPUP;
		exStyle = WS_EX_TOPMOST;
	} else {
	  if (! AdjustWindowRect (& clientRect, /*WS_THICKFRAME |*/ WS_CAPTION, FALSE)) {
			return FALSE;
	  }
	  style = WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION;
	}
		  hInst = hInstance; // Store instance handle in our global variable
		  hWnd = CreateWindowEx(exStyle, className, windowName, /*WS_THICKFRAME |*/ style,
									 clientRect.left,
									 clientRect.top,
									 clientRect.right - clientRect.left,
									 clientRect.bottom - clientRect.top,
									 NULL, NULL, hInstance, NULL);

		  hMainWindow = hWnd;

		  if (! hWnd) {
					 return (FALSE);
		  }

//		  SetClassLong(hWnd, GCL_HICON, (LONG) LoadIcon(hInst, MAKEINTRESOURCE(105)));

		  UpdateWindow (hWnd);
		  return TRUE;
}
		 
void showSLUDGE (HINSTANCE hInstance, HWND hWnd, int theID, int offsetY) {

	HDC hSrcDC;           // source DC - memory device context
	HBITMAP hbitmap;      // handle to the bitmap resource
	BITMAP bmp;           // structure for bitmap info
	int nHeight, nWidth;  // bitmap dimensions
	
	HDC hDestDC = GetDC (hWnd);

	// first load the bitmap resource
	hbitmap = (HBITMAP) LoadImage (hInstance, MAKEINTRESOURCE (theID),
                                    IMAGE_BITMAP, 0, 0,
                                    LR_CREATEDIBSECTION);
	if (hbitmap) {

		// create a DC for the bitmap to use
		hSrcDC = CreateCompatibleDC (NULL);
		if (hSrcDC) {

			// select the bitmap into the DC
			if (SelectObject (hSrcDC, hbitmap)) {

				// get image dimensions
				if (GetObject (hbitmap, sizeof (BITMAP), & bmp)) {

					nWidth = bmp.bmWidth;
					nHeight = bmp.bmHeight;

					// copy image from one DC to the other
					BitBlt (hDestDC, (winWidth - nWidth) >> 1, ((winHeight - nHeight) >> 1) + offsetY,
							nWidth, nHeight, hSrcDC, 0, 0, SRCCOPY);
				}
			}
			
			DeleteDC (hSrcDC);
		}
	}
	
	DeleteDC (hDestDC);
}

BOOL initVideo (HWND hWnd) {
	int i;
	HDC hdc;

	// Fill in bitmap header information

	screenBMPInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	screenBMPInfo.bmiHeader.biWidth=winWidth;
	screenBMPInfo.bmiHeader.biHeight=-winHeight;
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
		return(FALSE);
	};
	ReleaseDC(hWnd, hdc);

	return(TRUE);
}

BOOL CenterWindow (HWND hwndChild, HWND hwndParent) {
  int     xNew = 0, yNew = 0;

  if (! runningFullScreen) {
		  RECT    rChild, rParent, rWorkArea;
		  int     wChild, hChild, wParent, hParent;
		  BOOL    bResult;
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
		  
			if ((yNew+hChild) > rWorkArea.bottom) {
					 yNew = rWorkArea.bottom - hChild;
		  } else if (yNew < rWorkArea.top) {
					 yNew = rWorkArea.top;
		  }
	}
	// Set it, and return
	return SetWindowPos (hwndChild, NULL, xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

extern variableStack * noStack;

void centreMouse (HWND hWnd) {
	POINT wherePoint;

	input.justMoved = TRUE;
	wherePoint.x = input.mouseX = winWidth >> 1;
	wherePoint.y = input.mouseY = winHeight >> 1;
	
	if (ClientToScreen (hWnd, & wherePoint)) {
		SetCursorPos (wherePoint.x, wherePoint.y);
//		SetCursorPos (10, 10);
	}
}

void tick () {
	HDC hdc;

	if (! veryBusyIndeed) {
		walkAllPeople ();
		if (! handleInput ()) return;
		sludgeDisplay ();
	}

	hdc = GetDC (hMainWindow);
	BitBlt (hdc, 0, 0, winWidth, winHeight, bufferDC, 0, 0, SRCCOPY);
	ReleaseDC (hMainWindow, hdc);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_SYSCOMMAND:
	    if (! IsIconic(hWnd)) {			// Thankyou, Rachel, for finding this bit
			switch (wParam) {			// in someone else's code, taking the piss
				case SC_SCREENSAVE:		// and thereby bringing it to my attention
				case SC_MONITORPOWER:
				return 0;				// As soon as one of us can find her a few
			}							// Bezier curve algorithms she'll have what
		}								// she was actually looking for...
		
		return (DefWindowProc(hWnd, message, wParam, lParam));
		
		case WM_CREATE:
		if (! (specialSettings & SPECIAL_SILENT)) initSoundStuff (hWnd);
		registerWindowForFatal (hWnd);
		if (! initVideo (hWnd)) fatal ("Can't create video controller");

		startNewFunctionNum (0, 0, NULL, noStack);

		if (specialSettings & SPECIAL_INVISIBLE) {
			ShowWindow (hWnd, SW_SHOWMINIMIZED);
		} else {
			CenterWindow (hWnd, GetDesktopWindow());
			centreMouse (hWnd);
			ShowWindow (hWnd, SW_SHOWNORMAL);
		}

		if (  (specialSettings & SPECIAL_HIDELOGO))  	showSLUDGE (hInst, hWnd, 106, -8);
		if (! (specialSettings & SPECIAL_HIDELOADING))	showSLUDGE (hInst, hWnd, 107, 49);
		break;

		// Input! Input!

		case WM_LBUTTONDOWN:
		input.leftClick = TRUE;
		break;
		
		case WM_RBUTTONDOWN:
		input.rightClick = TRUE;
/*		{
			extern int cameraX;
			char aa[20];
			sprintf (aa, "%d", backDropImage[input.mouseY][input.mouseX + cameraX]);
			warning (aa);
		}						*/
		break;

		case WM_LBUTTONUP:
		input.leftRelease = TRUE;
		break;

		case WM_RBUTTONUP:
		input.rightRelease = TRUE;
		break;

		case WM_CHAR:
		input.keyPressed = (int) wParam;
		break;
		
		case WM_SYSKEYDOWN:
		input.keyPressed = 1000 + (int) wParam;
		break;

		case WM_KEYDOWN:
		input.keyPressed = 2000 + (int) wParam;
		break;

		case WM_MOUSEMOVE:
		if (LOWORD(lParam) < winWidth && HIWORD(lParam) < winHeight)
		{
			input.justMoved = TRUE;
			input.mouseX = LOWORD(lParam);
			input.mouseY = HIWORD(lParam);
		}
		break;

		// Tick tick tick...

/*
		case WM_TIMER:
		tick ();
		break;
*/
		case WM_ACTIVATE:
		if (runningFullScreen) {
			if (LOWORD (wParam) == WA_INACTIVE) {
				oldScreenSize ();
				ShowWindow (hWnd, SW_MINIMIZE);
			} else {
				SetWindowPos (hWnd, NULL, 0, 0, winWidth, winHeight, SWP_NOZORDER);
				ShowWindow (hWnd, SW_SHOWNORMAL);
				fullScreenMe ();		
			}
		}
		break;

		case WM_CLOSE:
		if (runningFullScreen) {
			DestroyWindow (hWnd);
		} else {
			veryBusyIndeed = TRUE;
			if (MessageBox (hWnd, getNumberedString(2), getNumberedString(1), MB_YESNO | MB_SETFOREGROUND | MB_APPLMODAL | MB_ICONQUESTION) == IDNO) {
				veryBusyIndeed = FALSE;
			} else {
				DestroyWindow (hWnd);
			}
		}

		break;

//		case MM_MCINOTIFY:
//		MessageBox (hWnd, "Music has finished!", "SLUDGE", MB_SETFOREGROUND | MB_APPLMODAL);
//		restartMusic ();
//		break;

		case WM_DESTROY:
		if (runningFullScreen) oldScreenSize ();
		DeleteDC(bufferDC);
		PostQuitMessage(0);
		break;
		
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint (hMainWindow, &ps);
			BitBlt (hdc, 0, 0, winWidth, winHeight, bufferDC, 0, 0, SRCCOPY);
			EndPaint (hMainWindow, &ps);
		}
		break;

		default:
		return (DefWindowProc(hWnd, message, wParam, lParam));
	};

	return (0);
}

BOOL InitApplication(HINSTANCE hInstance) {
	WNDCLASS  wc;
	HWND      hwnd;
	
	hwnd = FindWindow (className, NULL);
	if (hwnd) {	 	
		className = "SLUDGE_ENGINE_B_32";
 		hwnd = FindWindow (className, NULL);
	}
	if (hwnd) {
		fatal ("The SLUDGE engine is already running twice... and there's no way I'm going to run 3 SLUDGE games at once.");
		// We found another version of ourself. Lets defer to it:
		if (IsIconic(hwnd)) ShowWindow(hwnd, SW_RESTORE);
		SetForegroundWindow (hwnd);
		return FALSE;
	};

	switch (specialSettings & (SPECIAL_MOUSE_1 | SPECIAL_MOUSE_2)) {
		case 0:					// Invisible
		wc.hCursor       = LoadCursor (hInstance, MAKEINTRESOURCE(2196));
		break;
		
		case SPECIAL_MOUSE_1:			// Crosshair
		wc.hCursor       = LoadCursor (hInstance, MAKEINTRESOURCE(2195));
		break;
		
		case SPECIAL_MOUSE_2:			// Standard
		wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
		break;
		
		case SPECIAL_MOUSE_1 | SPECIAL_MOUSE_2:	// Big arrow
		wc.hCursor		 = LoadCursor (hInstance, MAKEINTRESOURCE(2197));
		break;
	}

	wc.style         = CS_OWNDC;
	wc.lpfnWndProc   = (WNDPROC) WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon		  = customIcon ? customIcon : LoadIcon(hInstance, MAKEINTRESOURCE(105));
	wc.hbrBackground = (HBRUSH__ *) GetStockObject (BLACK_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = className;

	return RegisterClass (& wc);
}

