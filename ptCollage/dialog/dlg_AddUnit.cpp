
#include <pxtnService.h>
extern pxtnService *g_pxtn;

#include <pxwWindowRect.h>
#include <pxwUTF8.h>

// #include <pxTText.h>
#include <uxStr.h>
#include <pxStr.h>
#include <pxMem.h>

#include "../../Generic/Japanese.h"

#include "../resource.h"

#include "dlg_AddUnit.h"

extern HINSTANCE g_hInst;

void _SetUnitName_byCombo( HWND hDlg )
{
	if( !g_pxtn->Woice_Num() ) return;

	int32_t i = (int32_t)SendDlgItemMessage( hDlg, IDC_COMBO_VOICENO, CB_GETCURSEL, 0, 0 );
	int32_t name_size = 0;
	char    name_c[ pxtnMAX_TUNEUNITNAME + 2 + 1 ] = {0};

	{
		const char* p_name_c = g_pxtn->Woice_Get( i )->get_name_buf( &name_size );
		if( !p_name_c ) p_name_c = "no name";
		strcpy( name_c, "u-" );
		strcat( name_c, p_name_c );
	}

	// pxTText tt; if( !tt.set_sjis_to_t( name_c ) ) return;
	uxDS tt; if( !( tt = uxDS_from_sjis( name_c ) ) ) return;
	// SetDlgItemText( hDlg, IDC_NAME, tt.tchr() );
	SetDlgItemText( hDlg, IDC_NAME, uxT( tt ) );
}


static bool _init_dlg_addunit( HWND hDlg, const ADDUNITSTRUCT* p_addunit )
{
	bool b_ret = false;
	int32_t num = g_pxtn->Woice_Num();

	SendDlgItemMessage( hDlg, IDC_NAME, EM_SETLIMITTEXT, pxtnMAX_TUNEUNITNAME, 0 );

	{
		// pxTText tt; if( !tt.set_sjis_to_t( p_addunit->name ) ) goto term;
		uxDS tt; if( !( tt = uxDS_from_sjis( p_addunit->name ) ) ) goto term;
		// SetDlgItemText(     hDlg, IDC_NAME, tt.tchr() );
		SetDlgItemText(     hDlg, IDC_NAME, uxT( tt ) );
	}

	num = g_pxtn->Woice_Num();
	if( !num )
	{
		if( Japanese_Is() ) SendDlgItemMessage( hDlg, IDC_COMBO_VOICENO, CB_ADDSTRING, 0, uxLP( "音源はありません" ) );
		else                SendDlgItemMessage( hDlg, IDC_COMBO_VOICENO, CB_ADDSTRING, 0, uxLP( "no voice" ) );
	}
	else
	{
		for( int32_t w = 0; w < num; w++ )
		{
			// pxTText tt; if( !tt.set_sjis_to_t( g_pxtn->Woice_Get( w )->get_name_buf( NULL ) ) ) goto term;
			uxDS tt; if( !( tt = uxDS_from_sjis( g_pxtn->Woice_Get( w )->get_name_buf( NULL ) ) ) ) goto term;
			// SendDlgItemMessage( hDlg, IDC_COMBO_VOICENO, CB_ADDSTRING, 0, (LPARAM)tt.tchr() );
			SendDlgItemMessage( hDlg, IDC_COMBO_VOICENO, CB_ADDSTRING, 0, uxLP( tt ) );
		}
	}
	SendDlgItemMessage( hDlg, IDC_COMBO_VOICENO, CB_SETCURSEL, 0, 0 );
	_SetUnitName_byCombo( hDlg );

	b_ret = true;
term:
	if( !b_ret )
	{
		MessageBox( hDlg, uxT( "unit name" ), uxT( "Add Unit init Error" ), MB_ICONERROR ); return false;
	}
	return b_ret;
}

static bool _Get( HWND hDlg, ADDUNITSTRUCT* p_addunit )
{
	uxSS<pxtnMAX_TUNEUNITNAME + 1> buf_t = {0};

	GetDlgItemText( hDlg, IDC_NAME, uxTOut( buf_t ), pxtnMAX_TUNEUNITNAME + 1 );
	p_addunit->voice_no = (int32_t)SendDlgItemMessage( hDlg, IDC_COMBO_VOICENO, CB_GETCURSEL, 0, 0 );

	{
		// pxTText tt; if( !tt.set_TCHAR_to_sjis( buf_t ) )
		uxDS tt; if( !( tt = uxDS_to_sjis( buf_t ) ) )
		{
			MessageBox( hDlg, uxT( "unit name" ), uxT( "Add Unit Error" ), MB_ICONERROR ); return false;
		}
		// strcpy( p_addunit->name, tt.sjis() );
		strcpy( p_addunit->name, *tt );
	}

	return true;
}

INT_PTR CALLBACK
dlg_AddUnit( HWND hDlg, UINT msg, WPARAM w, LPARAM l )
{
	static ADDUNITSTRUCT* p_addunit;

	switch( msg ){

	//ダイアログ起動
	case WM_INITDIALOG:

		p_addunit = (ADDUNITSTRUCT*)l;
		Japanese_DialogItem_Change( hDlg );
		pxwWindowRect_center      ( hDlg );
		_init_dlg_addunit         ( hDlg, p_addunit );

		return 1;

	case WM_COMMAND:

		switch( LOWORD(w) )
		{
		case IDOK    : if( _Get( hDlg, p_addunit ) ) EndDialog( hDlg, true ); break;
		case IDCANCEL: EndDialog( hDlg, false ); break;

		case IDC_COMBO_VOICENO:
			if( HIWORD(w) == CBN_SELCHANGE ) _SetUnitName_byCombo( hDlg );
			break;

		}
	default: return false;
	}
	return true;
}
