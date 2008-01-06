#include <windows.h>

#include "sprites.h"
#include "messbox.h"
#include "tga.h"
#include "winterfa.h"

extern unsigned short int * * backDropImage;
extern int VERT_RES, HORZ_RES;
extern spriteBank loadedSprites;
extern BOOL addColoursReal;
extern unsigned short int pal[255];
extern HWND mainWin;

char * fontifyFilename = "";
unsigned int fontifySpaceWidth = 8;
unsigned int fontifyNumGreys = 8;
BOOL fontifyAddSpace = TRUE;

int loadBackDrop (char * fileName);
byte findOrAddColour (unsigned short originalCol);
void updateData ();

void doFontification () {
	if (! loadBackDrop (fontifyFilename)) {
		errorBox ("Can't open file for reading", fontifyFilename);
		return;
	}
	
	if (VERT_RES > 255) {
		errorBox ("Sorry, this image cannot be fontified... maximum height is 255 pixels", fontifyFilename);
	}
	
	int which = 0;
	int fromColumn = 0;
	int yhot = (VERT_RES * 3) / 4;

	if (fontifyNumGreys) {
		for (int a = 0; a < fontifyNumGreys; a ++) {
			int b = (a * 255) / (fontifyNumGreys - 1);
			pal[loadedSprites.myPaletteNum ++] = makeColour (b, b, b);
		}
	} else {
		pal[loadedSprites.myPaletteNum ++] = makeColour (255, 255, 255);
	}

	for (int thisColumn = 0; thisColumn < HORZ_RES; thisColumn ++) {
		int y;
		unsigned short int transparent = backDropImage[0][0];

		// Find out if the column's empty...
		for (y = 0; y < VERT_RES; y ++) {
			if (backDropImage[y][thisColumn] != transparent) break;
		}
		
		int width = (thisColumn - fromColumn);

		// So was it?
		if (y == VERT_RES || width == 255) {

			// Make sure we didn't find a blank column last time
			if (width) {

				// Reserve memory
				byte * toHere = (byte *) new byte [width * VERT_RES];
				byte * dataIfOK = toHere;
				if (toHere) {

					addColoursReal = FALSE;

					// Do this one column at a time
					for (y = 0; y < VERT_RES; y ++) {
						for (int x = 0; x < width; x ++) {
							unsigned short int fromCol = backDropImage[y][x + fromColumn];
							* (toHere ++) = (fromCol == transparent) ? 0 : findOrAddColour (backDropImage[y][x + fromColumn]);
						}
					}
					
					insertSprite(which);
					loadedSprites.sprites[which].width = width;
					loadedSprites.sprites[which].height = VERT_RES;
					loadedSprites.sprites[which].yhot = yhot;
					delete loadedSprites.sprites[which].data;
					loadedSprites.sprites[which].data = dataIfOK;
					trimSprite (which);
					which ++;
				}					
			}
			fromColumn = thisColumn + 1;
		}
	}

	delete backDropImage;

	if (fontifyAddSpace) {
		insertSprite(which);
		loadedSprites.sprites[which].width = fontifySpaceWidth;
	}
	
	updateData ();
	pasteSprite (0);
	paintSpriteArea ();
}

LRESULT CALLBACK fontifyDialogFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message) {
        case WM_INITDIALOG:
        SetDlgItemInt (hDlg, ID_FON_SPACEWIDTH, fontifySpaceWidth, FALSE);
        SetDlgItemInt (hDlg, ID_FON_COLOURS, fontifyNumGreys, FALSE);
       	if (fontifyAddSpace) {
       		CheckDlgButton (hDlg, ID_FON_ADDSPACE, BST_CHECKED);
       	} else {
			EnableWindow (GetDlgItem (hDlg, ID_FON_SPACEWIDTH), FALSE);
       	}
		return (TRUE);
		
		case WM_COMMAND:
		switch (LOWORD(wParam)) {
			case ID_FON_ADDSPACE:
			EnableWindow (GetDlgItem (hDlg, ID_FON_SPACEWIDTH), IsDlgButtonChecked (hDlg, ID_FON_ADDSPACE) == BST_CHECKED);
			break;

			case IDOK:
			BOOL worked;

			unsigned int gotVal = GetDlgItemInt (hDlg, ID_FON_SPACEWIDTH, & worked, FALSE);
			if ((! worked) || (gotVal < 1) || (gotVal > 128)) {
				errorBox ("Fontification error", "Width of space character must be between 1 and 255 inclusive");
				break;
			}

			unsigned int gotVal2 = GetDlgItemInt (hDlg, ID_FON_COLOURS, & worked, FALSE);
			if ((! worked) || (gotVal2 < 1) || (gotVal2 > 128)) {
				errorBox ("Fontification error", "Number of greys must be between 1 and 128 inclusive");
				break;
			}

			fontifySpaceWidth = gotVal;
			fontifyNumGreys = gotVal2;
			fontifyAddSpace = IsDlgButtonChecked (hDlg, ID_FON_ADDSPACE) == BST_CHECKED;
			doFontification ();
			// No break here!

			case IDCANCEL:
			EndDialog(hDlg, TRUE);
			return TRUE;
		}
	}

    return FALSE;
}

void fontify (char * filename, HINSTANCE inst) {
	fontifyFilename = filename;
	DialogBox(inst, MAKEINTRESOURCE(701), mainWin, (DLGPROC) fontifyDialogFunc);
}
