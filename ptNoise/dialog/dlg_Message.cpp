#include <uxStr.h>

#include <pxwWindowRect.h>

#include "../resource.h"
#include "dlg_Message.h"


INT_PTR CALLBACK
dlg_Message( HWND hWnd, UINT msg, WPARAM w, LPARAM l )
{
	switch( msg ){

	//ダイアログ起動
	case WM_INITDIALOG:

		SetDlgItemText( hWnd, IDC_MESSAGE, uxT( ( (MESSAGEDIALOGSTRUCT *)l )->p_message ) );
		SetWindowText(  hWnd,              uxT( ( (MESSAGEDIALOGSTRUCT *)l )->p_title )   );
		pxwWindowRect_center( hWnd );
		return 1;

	//ボタンクリック
	case WM_COMMAND:

		switch( LOWORD( w ) ){
		//OK
		case IDOK:
			EndDialog( hWnd, true );
			break;
		}
	default: return false;

	}
	return true;
}
