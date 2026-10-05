
#include <pxtnService.h>
#include <uxStr.h>
extern pxtnService *g_pxtn;

#include <pxwWindowRect.h>

#include "../../Generic/Japanese.h"

#include "../resource.h"

#include "dlg_OverDrive.h"


bool IsShiftJIS( unsigned char c );


static void _InitDialog( HWND hDlg )
{
}

static void _SetParameter( HWND hDlg, const EFFECTSTRUCT_OVERDRIVE* p_over )
{
	SetDlgItemInt( hDlg, IDC_GROUP, p_over->group, true );

	uxSS<10> str;
	ux_sprintf_s( str, 10, "%0.1f", p_over->cut ); SetDlgItemText( hDlg, IDC_CUT, uxT( str ) );
	ux_sprintf_s( str, 10, "%0.1f", p_over->amp ); SetDlgItemText( hDlg, IDC_AMP, uxT( str ) );
}

static bool _GetParameter( HWND hDlg, EFFECTSTRUCT_OVERDRIVE* p_over )
{
	uxSS<10> str;

	GetDlgItemText( hDlg, IDC_CUT, uxTOut( str ), 10 ); p_over->cut = (float)_ttof( uxT( str ) );
	GetDlgItemText( hDlg, IDC_AMP, uxTOut( str ), 10 ); p_over->amp = (float)_ttof( uxT( str ) );

	p_over->group = GetDlgItemInt( hDlg, IDC_GROUP, NULL, true );

	return true;

}

static bool _CheckParameter( HWND hDlg, const EFFECTSTRUCT_OVERDRIVE* p_over )
{
	if( p_over->cut > TUNEOVERDRIVE_CUT_MAX || p_over->cut <  TUNEOVERDRIVE_CUT_MIN ){ Japanese_MessageBox( hDlg, "Cut", "error", MB_OK|MB_ICONEXCLAMATION ); return false; }
	if( p_over->amp > TUNEOVERDRIVE_AMP_MAX || p_over->amp <  TUNEOVERDRIVE_AMP_MIN ){ Japanese_MessageBox( hDlg, "Amp", "error", MB_OK|MB_ICONEXCLAMATION ); return false; }
	if( p_over->group >= g_pxtn->Group_Num()                                        ){ Japanese_MessageBox( hDlg, "Group", "error", MB_OK|MB_ICONEXCLAMATION ); return false; }

	return true;
}


//コールバック
INT_PTR CALLBACK
dlg_OverDrive_Procedure( HWND hDlg, UINT msg, WPARAM w, LPARAM l )
{
	static EFFECTSTRUCT_OVERDRIVE* _p_over;

	// IME off
	switch( msg )
	{
	case WM_COMMAND:
		switch( LOWORD( w ) )
		{
		case IDC_CUT  :
		case IDC_GROUP:
		case IDC_PRESS:
		case IDC_AMP  :

			if( HIWORD( w ) == EN_SETFOCUS )
			{
				HIMC hImc = ImmGetContext( hDlg );
				ImmSetOpenStatus( hImc, false );
				ImmReleaseContext( hDlg, hImc );
			}
			break;
		}
		break;
	}


	switch( msg )
	{

	//ダイアログ起動
	case WM_INITDIALOG:

		pxwWindowRect_center( hDlg );
		Japanese_DialogItem_Change( hDlg );
		_InitDialog( hDlg );
		_p_over = (EFFECTSTRUCT_OVERDRIVE*)l;
		_SetParameter( hDlg, _p_over );

		break;

	//ボタンクリック
	case WM_COMMAND:

		switch( LOWORD( w ) )
		{
		case IDOK:

			if( _GetParameter(   hDlg, _p_over ) &&
				_CheckParameter( hDlg, _p_over ) )
			{
				_p_over->b_delete = false;
				EndDialog( hDlg, true );
			}
			break;

		case IDCANCEL:

			EndDialog( hDlg, false );
			break;

		case IDC_DELETE:

			_p_over->b_delete = true;
			EndDialog( hDlg, true );
			break;
		}
		break;

	default: return false;

	}
	return true;
}
