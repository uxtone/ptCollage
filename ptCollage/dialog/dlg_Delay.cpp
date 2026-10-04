
#include <pxtnService.h>
extern pxtnService *g_pxtn;

#include <pxwWindowRect.h>

#include "../../Generic/Japanese.h"

#include "../resource.h"

#include "dlg_Delay.h"


bool IsShiftJIS( unsigned char c );


static void _InitDialog( HWND hDlg )
{
	int32_t i;

	// delay
	TCHAR *mode_delay_scale_e[] = { _T("beat"), _T("meas"), _T("second") };
	TCHAR *mode_delay_scale_j[] = { _T("拍"  ), _T("小節"), _T("秒"    ) };

	if( Japanese_Is() ){
		for( i = 0; i < 3; i++ ) SendDlgItemMessage( hDlg, IDC_COMBO_DELAYUNIT,    CB_ADDSTRING, 0, (LPARAM)mode_delay_scale_j[i]     );
	}else{
		for( i = 0; i < 3; i++ ) SendDlgItemMessage( hDlg, IDC_COMBO_DELAYUNIT,    CB_ADDSTRING, 0, (LPARAM)mode_delay_scale_e[i]     );
	}

}

static void _SetParameter( HWND hDlg, const EFFECTSTRUCT_DELAY* p_delay )
{
	TCHAR str[10];

	switch( p_delay->unit )
	{
	case DELAYUNIT_Beat:   SendDlgItemMessage( hDlg, IDC_COMBO_DELAYUNIT, CB_SETCURSEL, 0, 0 ); break;
	case DELAYUNIT_Meas:   SendDlgItemMessage( hDlg, IDC_COMBO_DELAYUNIT, CB_SETCURSEL, 1, 0 ); break;
	case DELAYUNIT_Second: SendDlgItemMessage( hDlg, IDC_COMBO_DELAYUNIT, CB_SETCURSEL, 2, 0 ); break;
	default:                     SendDlgItemMessage( hDlg, IDC_COMBO_DELAYUNIT, CB_SETCURSEL, 0, 0 ); break;
	}

	SetDlgItemInt( hDlg, IDC_GROUP, p_delay->group, true );

	_stprintf_s( str, 10, _T("%0.1f"), p_delay->freq ); SetDlgItemText( hDlg, IDC_DELAYFREQ, str );
	_stprintf_s( str, 10, _T("%0.0f"), p_delay->rate ); SetDlgItemText( hDlg, IDC_DELAYRATE, str );

}

static bool _GetParameter( HWND hDlg, EFFECTSTRUCT_DELAY* p_delay )
{
	TCHAR str[10];

	// delay
	GetDlgItemText( hDlg, IDC_DELAYRATE, str, 10 ); p_delay->rate = (float)_ttof( str );
	GetDlgItemText( hDlg, IDC_DELAYFREQ, str, 10 ); p_delay->freq = (float)_ttof( str );

	if( p_delay->rate < 0 ) p_delay->rate *= -1;
	if( p_delay->freq < 0 ) p_delay->freq *= -1;

	switch( SendDlgItemMessage( hDlg, IDC_COMBO_DELAYUNIT, CB_GETCURSEL, 0, 0 ) )
	{
	case 0: p_delay->unit = DELAYUNIT_Beat;   break;
	case 1: p_delay->unit = DELAYUNIT_Meas;   break;
	case 2: p_delay->unit = DELAYUNIT_Second; break;
	}

	p_delay->group = GetDlgItemInt( hDlg, IDC_GROUP, NULL, true );

	return true;

}

static bool _CheckParameter( HWND hDlg, const EFFECTSTRUCT_DELAY* p_delay )
{
	if( p_delay->freq  > 100                  ){ Japanese_MessageBox( hDlg, _T("Delay Frequency (0 - 100)"), _T("error"), MB_OK|MB_ICONEXCLAMATION ); return false; }
	if( p_delay->rate  > 100                  ){ Japanese_MessageBox( hDlg, _T("Delay Rate (0 - 100)"     ), _T("error"), MB_OK|MB_ICONEXCLAMATION ); return false; }
	if( p_delay->group >= g_pxtn->Group_Num() ){ Japanese_MessageBox( hDlg, _T("group"                    ), _T("error"), MB_OK|MB_ICONEXCLAMATION ); return false; }

	return true;
}


//コールバック
INT_PTR CALLBACK
dlg_Delay_Procedure( HWND hDlg, UINT msg, WPARAM w, LPARAM l )
{
	static EFFECTSTRUCT_DELAY* _p_delay;

	// IME off
	switch( msg )
	{
	case WM_COMMAND:
		switch( LOWORD( w ) )
		{
		case IDC_DELAYRATE:
		case IDC_DELAYFREQ:
		case IDC_GROUP    :

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
		_p_delay = (EFFECTSTRUCT_DELAY*)l;
		_SetParameter( hDlg, _p_delay );

		break;

	//ボタンクリック
	case WM_COMMAND:

		switch( LOWORD( w ) )
		{
		case IDOK:

			if( _GetParameter(   hDlg, _p_delay ) &&
				_CheckParameter( hDlg, _p_delay ) )
			{
				_p_delay->b_delete = false;
				EndDialog( hDlg, true );
			}
			break;

		case IDCANCEL:

			EndDialog( hDlg, false );
			break;

		case IDC_DELETE:

			_p_delay->b_delete = true;
			EndDialog( hDlg, true );
			break;
		}
		break;

	default:return false;

	}
	return true;
}
