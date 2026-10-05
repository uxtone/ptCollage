#include <uxStr.h>

#include <pxwWindowRect.h>

#include "../../Generic/Japanese.h"

#include "../resource.h"


INT_PTR CALLBACK
dlg_YesNo( HWND hWnd, UINT msg, WPARAM w, LPARAM l )
{
	switch( msg )
	{
	case WM_INITDIALOG:

		SetDlgItemText( hWnd, IDC_MESSAGE, uxT( (const char*)l ) );
		pxwWindowRect_center( hWnd );
		Japanese_DialogItem_Change( hWnd );
		break;

	case WM_COMMAND:

		switch( LOWORD( w ) )
		{
		case IDCANCEL: EndDialog( hWnd, false ); break;
		case IDOK    : EndDialog( hWnd, true  ); break;
		}

	default: return false;
	}
	return true;
}
