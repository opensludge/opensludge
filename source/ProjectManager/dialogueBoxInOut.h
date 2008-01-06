enum dlgOperation
{
	DIALOGUE_TRANSFER_PUTINTOBOX,
	DIALOGUE_TRANSFER_CHECK,
	DIALOGUE_TRANSFER_STORE
};

#define isChecked(a,b)		(IsDlgButtonChecked (a, b) == BST_CHECKED)

typedef bool (* inOutCallback) (HWND hDlg, dlgOperation operation);

BOOL dialogueBoxHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, inOutCallback callback);

bool dialogueBoxTransferValue_Int (HWND hDlg, int id, int * theInt, dlgOperation operation, const char * description, int min = 0, int max = 65535);
bool dialogueBoxTransferValue_Checkbox (HWND hDlg, int id, bool * theBool, dlgOperation operation);
