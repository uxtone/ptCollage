
#include <pxtnService.h>
#include <uxStr.h>
extern pxtnService* g_pxtn;

#include <pxwWindowRect.h>
#include <pxwFilePath.h>

#include "../../Generic/Japanese.h"

#include "../../ptConfig/ptConfig_Build.h"
#include "../../ptConfig/ptConfig_DlgCtrl.h"

#include "../resource.h"

#include "../../ptConfig/ptConfig.h"

#include "./dlg_BuildProgress.h"

static void _InitDialog( HWND hDlg )
{
	int32_t  i;

	const char*  mode_scope_e[] = { "top - last", "appoint by time" };
	const char*  mode_scope_j[] = { "最初から最後まで", "演奏時間で指定する" };

	if( Japanese_Is() )
	{
		for( i = 0; i < 2; i++ ) SendDlgItemMessage( hDlg, IDC_COMBO_PLAYSCOPE, CB_ADDSTRING, 0, uxLP( mode_scope_j[  i] ) );
	}
	else
	{
		for( i = 0; i < 2; i++ ) SendDlgItemMessage( hDlg, IDC_COMBO_PLAYSCOPE, CB_ADDSTRING, 0, uxLP( mode_scope_e[  i] ) );
	}

	ptConfig_cmb_quality_init( hDlg, IDC_COMBO_CHANNEL, IDC_COMBO_SPS, Japanese_Is() );

	{
		uxSS<10>   str;
		int32_t beat_num, beat_clock;
		float   beat_tempo;
		double  total_sec = 0;
		double  loop_sec  = 0;
		double  head_sec  = 0;
		int32_t meas_num;

		g_pxtn->master->Get( &beat_num, &beat_tempo, &beat_clock, &meas_num );

		if( beat_tempo )
		{
			total_sec = (double)meas_num * (double)beat_num * 60 / (double)beat_tempo;
			meas_num  = g_pxtn->master->get_play_meas() - g_pxtn->master->get_repeat_meas();
			loop_sec  = (double)meas_num * (double)beat_num * 60 / (double)beat_tempo;
			head_sec  = total_sec - loop_sec;
		}

		ux_sprintf_s( str, 10, "%0.2f", (float)total_sec ); SetDlgItemText( hDlg, IDC_TOTALTIME, uxT( str ) );
		ux_sprintf_s( str, 10, "%0.2f", (float)loop_sec  ); SetDlgItemText( hDlg, IDC_LOOPTIME , uxT( str ) );
		ux_sprintf_s( str, 10, "%0.2f", (float)head_sec  ); SetDlgItemText( hDlg, IDC_HEADTIME , uxT( str ) );
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

static void _SetParameter( HWND hDlg, const ptConfig_Build *p_bld )
{
	ptConfig_cmb_quality_set( hDlg,
							  IDC_COMBO_CHANNEL, p_bld->strm->ch_num,
							  IDC_COMBO_SPS    , p_bld->strm->sps );

	CheckDlgButton( hDlg, IDC_CHECK_UNITMUTE, p_bld->b_mute ? 1 : 0 );

	SendDlgItemMessage( hDlg, IDC_COMBO_PLAYSCOPE, CB_SETCURSEL, (WPARAM)p_bld->scope_mode, 0 );

	uxSS<10> str;
	ux_sprintf_s( str, 10, "%0.2f", p_bld->sec_playtime  ); SetDlgItemText( hDlg, IDC_PLAYTIME , uxT( str ) );
	ux_sprintf_s( str, 10, "%0.2f", p_bld->sec_extrafade ); SetDlgItemText( hDlg, IDC_EXTRAFADE, uxT( str ) );
	ux_sprintf_s( str, 10, "%0.2f", p_bld->volume        ); SetDlgItemText( hDlg, IDC_VOLUME   , uxT( str ) );

	_Enable_Scope( hDlg );
}

static bool _GetInputParameter( HWND hDlg, ptConfig_Build *p_c )
{
	ptConfig_cmb_quality_get( hDlg,
							  IDC_COMBO_CHANNEL, &p_c->strm->ch_num,
							  IDC_COMBO_SPS    , &p_c->strm->sps   );

	p_c->b_mute     = IsDlgButtonChecked( hDlg, IDC_CHECK_UNITMUTE ) ? true : false;
	p_c->scope_mode = (BUILDSCOPEMODE) SendDlgItemMessage( hDlg, IDC_COMBO_PLAYSCOPE, CB_GETCURSEL, 0, 0 );

	uxSS<10> str;
	GetDlgItemText( hDlg, IDC_PLAYTIME , uxTOut( str ), 10 ); p_c->sec_playtime  = (float)_ttof( uxT( str ) );
	GetDlgItemText( hDlg, IDC_EXTRAFADE, uxTOut( str ), 10 ); p_c->sec_extrafade = (float)_ttof( uxT( str ) );
	GetDlgItemText( hDlg, IDC_VOLUME   , uxTOut( str ), 10 ); p_c->volume        = (float)_ttof( uxT( str ) );

	return true;
}

static bool _AlarmParameter( HWND hDlg, const ptConfig_Build *p_bld )
{
	uxSS<100> err_msg = {0};
	if( p_bld->sec_extrafade < 0 )
	{
		if( Japanese_Is() ) strcpy( err_msg, "追加フェードアウトが異常です" );
		else                strcpy( err_msg, "Illegal Extra Fade Out" );
	}
	if( p_bld->scope_mode == BUILDSCOPEMODE_BYTIME && p_bld->sec_playtime <= 0 )
	{
		if( Japanese_Is() ) strcpy( err_msg, "演奏時間が異常です" );
		else                strcpy( err_msg, "Illegal Play Time" );
	}
	if( p_bld->volume > 1 || p_bld->volume < 0.01 )
	{
		if( Japanese_Is() ) strcpy( err_msg, "ボリュームの範囲(0.01 - 1.00)" );
		else                strcpy( err_msg, "Volume: 0.01 - 1.00" );
	}

	if( strlen( err_msg ) )
	{
		Japanese_MessageBox( hDlg, err_msg, "error", MB_OK|MB_ICONEXCLAMATION );
		return true;
	}
	return false;
}

//コールバック
INT_PTR CALLBACK
dlg_BuildOption_Procedure( HWND hDlg, UINT msg, WPARAM w, LPARAM l )
{
	static ptConfig_Build* _p_cfg;
	static bool            _bInit = false;

	switch( msg )
	{
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

	case WM_COMMAND:

		switch( LOWORD( w ) )
		{
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
			break;

		case IDC_COMBO_PLAYSCOPE: _Enable_Scope( hDlg ); break;

		}
		break;

	default: return false;

	}
	return true;
}
