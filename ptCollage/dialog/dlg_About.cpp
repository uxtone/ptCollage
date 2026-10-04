
#include <pxStdDef.h>

#include <pxwWindowRect.h>

#include "../../Generic/Japanese.h"

#include "../resource.h"


TCHAR*  gStrVersion = _T("version.%d.%d.%d.%d - %04d/%02d/%02d");
int32_t GetCompileVersion( int32_t *p1, int32_t *p2, int32_t *p3, int32_t *p4 );
void    GetCompileDate   ( int32_t *year, int32_t *month, int32_t *day     );

INT_PTR CALLBACK
	dlg_About( HWND hDlg, UINT msg, WPARAM w, LPARAM l )
{
	switch( msg )
	{

	//ダイアログ起動
	case WM_INITDIALOG:

		{
			TCHAR   str[64] = {0};
			int32_t y, m, d;
			int32_t v1,v2,v3,v4;
			GetCompileDate( &y, &m, &d );
			GetCompileVersion( &v1, &v2, &v3, &v4 );
			_stprintf_s( str, 64, gStrVersion, v1, v2, v3, v4, y, m, d );
			SetDlgItemText( hDlg, IDC_VERSION, str );
		}

		SetDlgItemText( hDlg, IDC_MESSAGE, _T("beta test") );

		pxwWindowRect_center( hDlg );
		Japanese_DialogItem_Change( hDlg );


		return 1;

	//ボタンクリック
	case WM_COMMAND:

		switch( LOWORD( w ) ){
		//キャンセル
		case IDCANCEL:
			EndDialog( hDlg, false );
			break;
		//OK
		case IDOK:
			EndDialog( hDlg, true );
			break;
		}

		default:return false;

	}
	return true;
}
