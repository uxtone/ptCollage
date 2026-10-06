
#include <pxwWindowRect.h>


#include <pxwFilePath.h>

#include "../../Generic/Japanese.h"

#include "../resource.h"


#include "../../ptConfig/ptConfig_DlgCtrl.h"
#include "../../ptConfig/ptConfig.h"

static void _InitDialog( HWND hDlg )
{
	ptConfig_cmb_quality_init( hDlg, IDC_COMBO_AT_CHANNEL, IDC_COMBO_AT_SPS, Japanese_Is() );
}

static void _SetParameter( HWND hDlg, ptConfig *p_c )
{
	{
		uxSS<10> str;
		ux_sprintf_s( str, 10, "%0.2f", p_c->strm->buf_sec );
		SetDlgItemText( hDlg, IDC_AT_BUFFER, uxT( str ) );
	}

	ptConfig_cmb_quality_set( hDlg,
							  IDC_COMBO_AT_CHANNEL, p_c->strm->ch_num,
							  IDC_COMBO_AT_SPS    , p_c->strm->sps );
}

static bool _GetInputParameter( HWND hDlg, ptConfig *p_c )
{
	p_c->set_default();

	uxSS<10> str; GetDlgItemText( hDlg, IDC_AT_BUFFER, uxTOut( str ), 10 ); p_c->strm->buf_sec = (float)ux_S::to_double( str );

	ptConfig_cmb_quality_get( hDlg,
							  IDC_COMBO_AT_CHANNEL, &p_c->strm->ch_num,
							  IDC_COMBO_AT_SPS    , &p_c->strm->sps );

	return true;
}

static bool _CheckParameter( HWND hDlg, const ptConfig *p_c )
{
	if( p_c->strm->buf_sec < 0.01f || p_c->strm->buf_sec > 1.00f )
	{
		MessageBox( hDlg, uxT( "Sound Buffer :\r\nmin 0.01sec / max 1.00sec" ), uxT( "error" ), MB_OK );
		return false;
	}
	return true;
}

//コールバック
INT_PTR CALLBACK
dlg_Config_Procedure( HWND hDlg, UINT msg, WPARAM w, LPARAM l )
{
	static ptConfig* _p_cfg = NULL;

	switch( msg ){

	//ダイアログ起動
	case WM_INITDIALOG:

		_p_cfg = (ptConfig*)l;

		pxwWindowRect_center(       hDlg );
		Japanese_DialogItem_Change( hDlg );
		_InitDialog(   hDlg );
		_SetParameter( hDlg, _p_cfg );

		break;

	//ボタンクリック
	case WM_COMMAND:

		switch( LOWORD( w ) ){
		case IDOK:
			if( _GetInputParameter( hDlg, _p_cfg ) && _CheckParameter( hDlg, _p_cfg ) )
			{
				EndDialog( hDlg, true );
			}
			break;

		case IDCANCEL:
			EndDialog( hDlg, false );
			break;

		case IDC_DEFAULT:
			_p_cfg->set_default();
			_SetParameter( hDlg, _p_cfg );
			break;
		}
		break;

	default: return false;

	}
	return true;
}
