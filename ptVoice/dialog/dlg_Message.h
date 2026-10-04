typedef struct
{
	const TCHAR *p_message;
	const TCHAR *p_title;
}
MESSAGEDIALOGSTRUCT;

INT_PTR CALLBACK
dlg_Message( HWND hWnd, UINT msg, WPARAM w, LPARAM l );
