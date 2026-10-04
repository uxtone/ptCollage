
#include <pxwWindowRect.h>
#include <pxwFilePath.h>

#include "../../Generic/Japanese.h"

#include "../../ptConfig/ptConfig_DlgCtrl.h"
#include "../../ptConfig/ptConfig.h"

#include "../resource.h"

static bool _b_midi_devices = false;

static void _InitDialog( HWND hDlg )
{

	ptConfig_cmb_quality_init( hDlg, IDC_COMBO_AT_CHANNEL, IDC_COMBO_AT_SPS, Japanese_Is() );

	// midi..
	{
		int num = midiInGetNumDevs();
		if( num )
		{
			_b_midi_devices  = true;
			MIDIINCAPS caps;
			for( int i = 0; i < num; i++ )
			{
				if( midiInGetDevCaps( i, &caps, sizeof(caps)) == MMSYSERR_NOERROR )
				{
					SendDlgItemMessage( hDlg, IDC_COMBO_MIDIDEVICE, CB_ADDSTRING, 0, (LPARAM)caps.szPname );
				}
			}
		}
		else
		{
			_b_midi_devices = false;
			EnableWindow( GetDlgItem( hDlg, IDC_COMBO_MIDIDEVICE ), FALSE );
		}
	}

}

static void _SetParameter( HWND hDlg, const ptConfig *p_c )
{
	{
		TCHAR str[ 10 ];
		_stprintf_s( str, 10, _T("%0.2f"), p_c->strm->buf_sec    ); SetDlgItemText( hDlg, IDC_AT_BUFFER, str );
		_stprintf_s( str, 10, _T("%0.4f"), p_c->midi->key_tuning ); SetDlgItemText( hDlg, IDC_KEYTUNING, str );
	}

	ptConfig_cmb_quality_set( hDlg,
		IDC_COMBO_AT_CHANNEL, p_c->strm->ch_num,
		IDC_COMBO_AT_SPS    , p_c->strm->sps   );

	if( _b_midi_devices )
	{
		int32_t i = SendDlgItemMessage( hDlg, IDC_COMBO_MIDIDEVICE, CB_FINDSTRING, 0, (LPARAM)p_c->midi->name );
		if( i == CB_ERR ) i = 0;
		SendDlgItemMessage( hDlg, IDC_COMBO_MIDIDEVICE, CB_SETCURSEL , i, 0 );
	}
	else
	{
		SetDlgItemText( hDlg, IDC_COMBO_MIDIDEVICE, p_c->midi->name );
	}

	CheckDlgButton( hDlg, IDC_CHK_VELOCITY, p_c->midi->b_velo ? 1 : 0 );
}

static bool _GetInputParameter( HWND hDlg, ptConfig *p_c )
{
	p_c->set_default();

	{
		TCHAR str[ 10 ];
		GetDlgItemText( hDlg, IDC_AT_BUFFER, str, 10 ); p_c->strm->buf_sec    = (float)_ttof( str );
		GetDlgItemText( hDlg, IDC_KEYTUNING, str, 10 ); p_c->midi->key_tuning = (float)_ttof( str );
	}

	ptConfig_cmb_quality_get( hDlg,
		IDC_COMBO_AT_CHANNEL, &p_c->strm->ch_num,
		IDC_COMBO_AT_SPS    , &p_c->strm->sps   );

	// midi..
	GetDlgItemText( hDlg, IDC_COMBO_MIDIDEVICE, p_c->midi->name, BUFSIZE_MIDIDEVICENAME );
	p_c->midi->b_velo = IsDlgButtonChecked( hDlg, IDC_CHK_VELOCITY ) ? true : false;

	return true;
}

static bool _CheckParameter( HWND hDlg, ptConfig *p_c )
{
	if( p_c->strm->buf_sec < 0.01f || p_c->strm->buf_sec > 1.00f )
	{
		MessageBox( hDlg, _T("Sound Buffer :\r\nmin 0.01sec / max 1.00sec"), _T("error"), MB_OK );
		return false;
	}
	return true;
}

//コールバック
INT_PTR CALLBACK
dlg_Config_Procedure( HWND hDlg, UINT msg, WPARAM w, LPARAM l )
{
	static ptConfig *_p_cfg;

	switch( msg )
	{

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

		switch( LOWORD( w ) )
		{
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

	default:return false;

	}
	return true;
}
