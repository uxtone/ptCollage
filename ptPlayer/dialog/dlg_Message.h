#include <pxStdDef.h>
typedef struct
{
	const uxDS p_message;
	const uxDS p_title;
}
MESSAGEDIALOGSTRUCT;

INT_PTR CALLBACK
dlg_Message( HWND hWnd, UINT msg, WPARAM w, LPARAM l );
