#include <stdio.h>
#include "winterface.h"
#include "zbuffer.h"
#include "moreio.h"
#include "backdrop.h"

extern unsigned short int * * backDropImage;
extern int HORZ_RES, VERT_RES;
zPanel panel[16];
int numPanels = 0;

bool processZBufferData () {
	int n, x, y;
	numPanels = 0;

	for (y = 0; y < VERT_RES; y ++) {
		for (x = 0; x < HORZ_RES; x ++) {
			for (n = 0; n < numPanels; n ++) {
				if (panel[n].theColour == backDropImage[y][x]) break;
			}
			if (n == numPanels) {
				if (n < 16) {
					panel[n].theColour = backDropImage[y][x];
					numPanels ++;
					panel[n].yCutOff = 0;
				} else return false;
			}
			if (panel[n].theColour) panel[n].yCutOff = y;
		}
	}
	return true;
}

bool loadZBufferFile (char * name) {
	int n, x, y, zbWidth, zbHeight;
	unsigned long stillToGo = 0;
	
	FILE * fp = fopen (name, "rb");
	if (! fp) return false;
	if (fgetc (fp) != 'S') { fclose (fp); return false; }
	if (fgetc (fp) != 'z') { fclose (fp); return false; }
	if (fgetc (fp) != 'b') { fclose (fp); return false; }
	switch (fgetc (fp)) {
		case 0:
		zbWidth = 640;
		zbHeight = 480;
		break;
		
		case 1:
		zbWidth = get2bytes (fp);
		zbHeight = get2bytes (fp);
		break;
		
		default:
		fclose (fp);
		return false;
	}

	numPanels = fgetc (fp);
	for (n = 0; n < numPanels; n ++) {
		panel[n].yCutOff = get2bytes (fp);
		panel[n].theColour = 0x1111 * n;
	}
	
	if (! initBackDrop (zbWidth, zbHeight)) return false;
	
	for (y = 0; y < zbHeight; y ++) {
		for (x = 0; x < zbWidth; x ++) {
			if (stillToGo == 0) {
				n = fgetc (fp);
				stillToGo = n >> 4;
				if (stillToGo == 15) stillToGo = get2bytes (fp) + 16l;
				else stillToGo ++;
				n &= 15;
			}
			backDropImage[y][x] = panel[n].theColour;
			stillToGo --;
		}
	}
	fclose (fp);
	return true;
}

int editLayerNum = 0;

bool setZBufferClick (int x, int y) {
	for (editLayerNum = 0; editLayerNum < numPanels; editLayerNum ++) {
		if (panel[editLayerNum].theColour == backDropImage[y][x]) break;
	}
	return (editLayerNum < numPanels);
}

void saveZBufferFile (char * name) {
	FILE * fp = fopen (name, "wb");
	int x, y, n;
	unsigned long totalPixels = VERT_RES;
	totalPixels *= HORZ_RES;
	unsigned long thisPixel = 0;
	unsigned long countPixels = 0;
	unsigned short thisColour;

	fputc ('S', fp);
	fputc ('z', fp);
	fputc ('b', fp);
	fputc (1, fp);
	put2bytes (HORZ_RES, fp);
	put2bytes (VERT_RES, fp);
	fputc (numPanels, fp);
	for (n = 0; n < numPanels; n ++) {
		put2bytes (panel[n].yCutOff, fp);
	}
	
	while (thisPixel < totalPixels) {
		x = thisPixel % HORZ_RES;
		y = thisPixel / HORZ_RES;
		thisColour = backDropImage[y][x];

		// Find how many pixels of the same colour follow this

		countPixels = thisPixel + 1;
		while (thisColour == backDropImage[countPixels / HORZ_RES][countPixels % HORZ_RES]) {
			countPixels ++;
			if (countPixels == totalPixels) break;
			if (countPixels - thisPixel == 65551) break;
		}
		countPixels -= thisPixel;
		
		// Find which layer the pixels belong to by colour
		
		for (n = 0; n < numPanels; n ++) {
			if (panel[n].theColour == backDropImage[y][x]) break;
		}
		if (n > 15) n = 0;
		if (countPixels < 16) {
			fputc (n + ((unsigned char) ((countPixels - 1) << 4)), fp);
		} else {
			fputc (n + 240, fp);
			put2bytes (countPixels - 16, fp);
		}
		thisPixel += countPixels;
	}
	fclose (fp);
}

#ifdef WIN32

LRESULT CALLBACK LayerSettingsFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message) {
        case WM_INITDIALOG:
        {
        	char buff[256];
			ShowWindow (hDlg, SW_HIDE);
			sprintf (buff, "There are %i layers...", numPanels);
		    SetWindowText (GetDlgItem(hDlg, ID_LAYERTOTAL), buff);
			sprintf (buff, "This is layer %i.", editLayerNum);
		    SetWindowText (GetDlgItem(hDlg, ID_LAYERNUM), buff);
			sprintf (buff, "%i", panel[editLayerNum].yCutOff);
		    SetWindowText (GetDlgItem(hDlg, ID_LAYERCUTOFF), buff);
			ShowWindow (hDlg, SW_SHOW);
			return (true);
		}

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
				bool worked;
				int grabbed = GetDlgItemInt (hDlg, ID_LAYERCUTOFF, & worked, false);
				if (worked) {
					panel[editLayerNum].yCutOff = grabbed;
					EndDialog(hDlg, true);
				}
				return (true);
			}
			break;
	}

    return false;
}

#endif
