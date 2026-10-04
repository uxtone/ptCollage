
#include <pxtnService.h>
extern pxtnService *g_pxtn;

#include <pxwWindowRect.h>
#include <pxwFilePath.h>


#include "../../Generic/Japanese.h"

#include "../resource.h"

#include "../../ptConfig/ptConfig_Build.h"
#include "../../ptConfig/ptConfig_DlgCtrl.h"

#include "dlg_BuildProgress.h"



static void _InitDialog( HWND hDlg )
{
	int32_t i;

	ptConfig_cmb_quality_init( hDlg, IDC_COMBO_CHANNEL, IDC_COMBO_SPS, Japanese_Is() );

	TCHAR *mode_scope_e[  ] = { _T("top - last"      ), _T("appoint by time"   ) };
	TCHAR *mode_scope_j[  ] = { _T("最初から最後まで"), _T("演奏時間で指定する") };

	if( Japanese_Is() )
	{
		for( i = 0; i < 2; i++ ) SendDlgItemMessage( hDlg, IDC_COMBO_PLAYSCOPE, CB_ADDSTRING, 0, (LPARAM)mode_scope_j[  i] );
	}
	else
	{
		for( i = 0; i < 2; i++ ) SendDlgItemMessage( hDlg, IDC_COMBO_PLAYSCOPE, CB_ADDSTRING, 0, (LPARAM)mode_scope_e[  i] );
	}

	{
		TCHAR   str[ 10 ];
		int32_t beat_num, beat_clock;
		float   beat_tempo;
		double  total_sec   = 0;
		double  loop_sec    = 0;
		double  head_sec    = 0;
		int32_t meas_num    = 0;

		g_pxtn->master->Get( &beat_num, &beat_tempo, &beat_clock, &meas_num );

		if( beat_tempo )
		{
			int32_t last_meas = g_pxtn->master->get_last_meas();

			if( !last_meas ) last_meas = meas_num;

			total_sec = (double)meas_num * (double)beat_num * 60 / (double)beat_tempo;
			meas_num  = last_meas - g_pxtn->master->get_repeat_meas();
			loop_sec  = (double)meas_num * (double)beat_num * 60 / (double)beat_tempo;
			head_sec  = total_sec - loop_sec;
		}

		_stprintf_s( str, 10, _T("%0.2f"), (float)total_sec ); SetDlgItemText( hDlg, IDC_TOTALTIME, str );
		_stprintf_s( str, 10, _T("%0.2f"), (float)loop_sec  ); SetDlgItemText( hDlg, IDC_LOOPTIME , str );
		_stprintf_s( str, 10, _T("%0.2f"), (float)head_sec  ); SetDlgItemText( hDlg, IDC_HEADTIME , str );
	}
}

static void _Enable_Scope( HWND hDlg )
{
	bool b;

	switch( SendDlgItemMessage( hDlg, IDC_COMBO_PLAYSCOPE, CB_GETCURSEL, 0, 0 ) )
	{
	case BUILDSCOPEMODE_BYTIME: b = true ; break;
	default:                    b = false;
	}

	EnableWindow( GetDlgItem( hDlg, IDC_PLAYTIME      ), b );
	EnableWindow( GetDlgItem( hDlg, IDC_TEXT_PLAYTIME ), b );
	EnableWindow( GetDlgItem( hDlg, IDC_TEXT_SEC3     ), b );
}

static void _SetParameter( HWND hDlg, const ptConfig_Build *p_c )
{
	ptConfig_cmb_quality_set( hDlg,
		IDC_COMBO_CHANNEL, p_c->strm->ch_num,
		IDC_COMBO_SPS    , p_c->strm->sps   );

	CheckDlgButton( hDlg, IDC_CHECK_UNITMUTE, p_c->b_mute ? 1 : 0 );

	SendDlgItemMessage( hDlg, IDC_COMBO_PLAYSCOPE, CB_SETCURSEL, (WPARAM)p_c->scope_mode, 0 );

	TCHAR str[ 10 ];
	_stprintf_s( str, 10, _T("%0.2f"), p_c->sec_playtime  ); SetDlgItemText( hDlg, IDC_PLAYTIME , str );
	_stprintf_s( str, 10, _T("%0.2f"), p_c->sec_extrafade ); SetDlgItemText( hDlg, IDC_EXTRAFADE, str );
	_stprintf_s( str, 10, _T("%0.2f"), p_c->volume        ); SetDlgItemText( hDlg, IDC_VOLUME   , str );

	_Enable_Scope( hDlg );

}

static bool _GetInputParameter( HWND hDlg, ptConfig_Build *p_c )
{
	p_c->set_default();

	ptConfig_cmb_quality_get( hDlg    ,
		IDC_COMBO_CHANNEL, &p_c->strm->ch_num,
		IDC_COMBO_SPS    , &p_c->strm->sps );

	p_c->b_mute     = IsDlgButtonChecked( hDlg, IDC_CHECK_UNITMUTE ) ? true : false;
	p_c->scope_mode = (BUILDSCOPEMODE) SendDlgItemMessage( hDlg, IDC_COMBO_PLAYSCOPE, CB_GETCURSEL, 0, 0 );

	TCHAR str[10];
	GetDlgItemText( hDlg, IDC_PLAYTIME , str, 10 ); if( !_stscanf( str, _T("%f"), &p_c->sec_playtime  ) ) p_c->sec_playtime  = 0;
	GetDlgItemText( hDlg, IDC_EXTRAFADE, str, 10 ); if( !_stscanf( str, _T("%f"), &p_c->sec_extrafade ) ) p_c->sec_extrafade = 0;
	GetDlgItemText( hDlg, IDC_VOLUME   , str, 10 ); if( !_stscanf( str, _T("%f"), &p_c->volume        ) ) p_c->volume        = 0;

	return true;
}

static bool _AlarmParameter( HWND hDlg, const ptConfig_Build  *p_c )
{
	TCHAR err_msg[ 100 ] = {0};
	if( p_c->sec_extrafade < 0 )
	{
		if( Japanese_Is() ) _tcscpy( err_msg, _T("追加フェードアウトが異常です") );
		else                _tcscpy( err_msg, _T("Illegal Extra Fade Out"      ) );
	}
	if( p_c->scope_mode == BUILDSCOPEMODE_BYTIME && p_c->sec_playtime <= 0 )
	{
		if( Japanese_Is() ) _tcscpy( err_msg, _T("演奏時間が異常です"          ) );
		else                _tcscpy( err_msg, _T("Illegal Play Time"           ) );
	}
	if( p_c->volume < 0.01f || p_c->volume > 1.00 )
	{
		if( Japanese_Is() ) _tcscpy( err_msg, _T("ボリュームの範囲( 0.01 - 1.00 )") );
		else                _tcscpy( err_msg, _T("Volume: 0.01 - 1.00"            ) );
	}
	if( _tcslen( err_msg ) )
	{
		Japanese_MessageBox( hDlg, err_msg, _T("error"), MB_OK|MB_ICONEXCLAMATION );
		return true;
	}
	return false;
}

//コールバック
INT_PTR CALLBACK
dlg_BuildOption_Procedure( HWND hDlg, UINT msg, WPARAM w, LPARAM l )
{
	static ptConfig_Build* _p_cfg = NULL ;
	static bool            _bInit = false;

	switch( msg ){

	//ダイアログ起動
	case WM_INITDIALOG:

		_p_cfg = (ptConfig_Build*)l;

		pxwWindowRect_center(       hDlg );
		Japanese_DialogItem_Change( hDlg );
		_InitDialog(   hDlg );
		_SetParameter( hDlg, _p_cfg );
		_bInit = true;

		break;

	case WM_CLOSE:
		break;

	//ボタンクリック
	case WM_COMMAND:

		switch( LOWORD( w ) ){
		case IDOK:
			if( _GetInputParameter( hDlg, _p_cfg ) && !_AlarmParameter( hDlg, _p_cfg ) )
			{
				EndDialog( hDlg, true );
			}
			break;

		case IDCANCEL:
			_bInit = false;
			EndDialog( hDlg, false );
			break;

		case IDC_DEFAULT:
			_p_cfg->set_default();
			_SetParameter( hDlg, _p_cfg );
			break;

		case IDC_COMBO_PLAYSCOPE: _Enable_Scope( hDlg ); break;

		}
		break;

	default:return false;

	}
	return true;
}
