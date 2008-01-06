#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "winterfa.h"
#include "moreio.h"
#include "messbox.h"
#include "wincomp.h"
#include "wintext.h"
#include "registry.h"
#include "settings.h"
#include "splitter.h"
#include "regbox.h"
#include "launch.h"
#include "dialogueBoxInOut.h"

// NASTY!
void setChanged (BOOL newVal);

BOOL dialogueBoxHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, inOutCallback callback)
{
	switch (message) {
        case WM_INITDIALOG:
        {
        	callback (hDlg, DIALOGUE_TRANSFER_PUTINTOBOX);
			return (TRUE);
		}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
				{
					if (! callback (hDlg, DIALOGUE_TRANSFER_CHECK))
					{
						return TRUE;
					}
					
					callback (hDlg, DIALOGUE_TRANSFER_STORE);
					setChanged (TRUE);
				}

				case IDCANCEL:
				EndDialog(hDlg, TRUE);
				return (TRUE);
			}
			break;
	}

    return FALSE;
}

bool dialogueBoxTransferValue_Int (HWND hDlg, int id, int * theInt, dlgOperation operation, const char * description, int min, int max)
{
	int success = true;

	switch (operation)
	{
		case DIALOGUE_TRANSFER_PUTINTOBOX:
	   	SetDlgItemInt (hDlg, id, *theInt, 0);
	   	break;
	   	
	   	case DIALOGUE_TRANSFER_CHECK:
	   	{
	   		unsigned int blah = GetDlgItemInt (hDlg, id, &success, 0);
	   		if (! success || blah < min || blah > max)
	   		{
	   			char blah[255];
	   			sprintf (blah, "%s should be an integer between %d and %d", description, min, max);
				messageBox (er, blah);
	   			success = false;
	   		}
	   	}
   		break;
   		
   		case DIALOGUE_TRANSFER_STORE:
   		*theInt = GetDlgItemInt (hDlg, id, &success, 0);
   		break;
	}
	
	return success;
}

bool dialogueBoxTransferValue_Checkbox (HWND hDlg, int id, bool * theBool, dlgOperation operation)
{
	switch (operation)
	{
		case DIALOGUE_TRANSFER_PUTINTOBOX:
		CheckDlgButton (hDlg, id, *theBool ? BST_CHECKED : BST_UNCHECKED);
	   	break;
	   	
		case DIALOGUE_TRANSFER_STORE:
		*theBool = isChecked (hDlg, id);
   		break;
	}
	
	return true;
}
