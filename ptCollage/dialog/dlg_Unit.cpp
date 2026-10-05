
#include <pxTText.h>

#include <pxtnService.h>
extern pxtnService *g_pxtn;

#include <pxShiftJIS.h>

#include <pxwWindowRect.h>

#include "../../Generic/Japanese.h"

#include "../resource.h"

#include "../UndoEvent.h"
#include "../UnitFocus.h"


// 情報を表示
static void _SetParameter( HWND hDlg, int32_t unit_index )
{
	pxTText tt;

	if( tt.set_sjis_to_t( g_pxtn->Unit_Get( unit_index )->get_name_buf( NULL ) ) )
	{
		SetDlgItemText( hDlg, IDC_NAME, tt.tchr() );
	}

	SetDlgItemInt(  hDlg, IDC_EVENTNUM, g_pxtn->evels->get_Count( (unsigned char)unit_index ), true );
}


static bool _GetParameter( HWND hDlg, int32_t unit_index )
{
	TCHAR name_t[ pxtnMAX_TUNEUNITNAME + 1 ];


	GetDlgItemText( hDlg, IDC_NAME, name_t, pxtnMAX_TUNEUNITNAME+1 );

	pxTText tt; if( !tt.set_TCHAR_to_sjis( name_t ) ) return false;
	int32_t size = 0;
	pxShiftJIS_check_size( tt.sjis(), &size, true );
	g_pxtn->Unit_Get_variable( unit_index )->set_name_buf( tt.sjis(), size );

	return true;
}

//コールバック
INT_PTR CALLBACK
dlg_Unit( HWND hDlg, UINT msg, WPARAM w, LPARAM l )
{
	static int32_t _unit_index;

	switch( msg ){

	//ダイアログ起動
	case WM_INITDIALOG:

		pxwWindowRect_center( hDlg );
		Japanese_DialogItem_Change( hDlg );

		SendDlgItemMessage( hDlg, IDC_NAME, EM_SETLIMITTEXT, pxtnMAX_TUNEUNITNAME, 0 );

		_unit_index = (int32_t)l;

		if( !g_pxtn->Unit_Get( _unit_index ) ){ EndDialog( hDlg, false ); break; }
		_SetParameter( hDlg, _unit_index );

		break;

	//ボタンクリック
	case WM_COMMAND:

		switch( LOWORD( w ) )
		{
		case IDOK:

			if( !_GetParameter( hDlg, _unit_index ) ) break;
			EndDialog( hDlg, true );
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
