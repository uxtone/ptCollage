
#include <pxwWindowRect.h>
#include <pxwFilePath.h>

#include "../../Generic/Japanese.h"

#include "../resource.h"
#include "../NoiseTable.h"


void dlg_Noise_Design_CopyUnit( int32_t from, int32_t to );


//コールバック
INT_PTR CALLBACK
dlg_NoiseDesign_Copy( HWND hDlg, UINT msg, WPARAM w, LPARAM l )
{
	switch( msg )
	{
	//ダイアログ起動
	case WM_INITDIALOG:

		pxwWindowRect_center( hDlg );
		{
			int32_t i;
			TCHAR *unit_name[MAX_NOISETABLEUNIT] = { _T("1"), _T("2"), _T("3"), _T("4") };

			for( i = 0; i < MAX_NOISETABLEUNIT; i++ ) SendDlgItemMessage( hDlg, IDC_COMBO_UNIT_FROM, CB_ADDSTRING, 0, (LPARAM)unit_name[i] );
			for( i = 0; i < MAX_NOISETABLEUNIT; i++ ) SendDlgItemMessage( hDlg, IDC_COMBO_UNIT_TO  , CB_ADDSTRING, 0, (LPARAM)unit_name[i] );
			SendDlgItemMessage( hDlg, IDC_COMBO_UNIT_FROM, CB_SETCURSEL, (WPARAM)l, 0 );
			SendDlgItemMessage( hDlg, IDC_COMBO_UNIT_TO  , CB_SETCURSEL, (WPARAM)l, 0 );

			Japanese_DialogItem_Change( hDlg );
			if( Japanese_Is() ) SetWindowText( hDlg, _T("レイヤーのコピー") );
		}
		break;

	//ボタンクリック
	case WM_COMMAND:

		switch( LOWORD( w ) )
		{
		case IDOK:
		{
			int32_t from, to;
			from = SendDlgItemMessage( hDlg, IDC_COMBO_UNIT_FROM, CB_GETCURSEL, 0, 0 );
			to   = SendDlgItemMessage( hDlg, IDC_COMBO_UNIT_TO  , CB_GETCURSEL, 0, 0 );
			dlg_Noise_Design_CopyUnit( from, to );
		}
			EndDialog( hDlg, true  );
			break;

		case IDCANCEL:
			EndDialog( hDlg, false );
			break;
		}
		break;

	default: return false;

	}
	return true;
}
