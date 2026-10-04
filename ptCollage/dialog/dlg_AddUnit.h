typedef struct
{
	int32_t voice_no;
	char    name[ pxtnMAX_TUNEUNITNAME + 1 ];
}
ADDUNITSTRUCT;

INT_PTR CALLBACK
	dlg_AddUnit( HWND hDlg, UINT msg, WPARAM w, LPARAM l );
