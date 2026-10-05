
#include <pxtnPulse_Oggv.h>

#include <pxtnService.h>
extern pxtnService *g_pxtn;

#include <pxwWindowRect.h>

#include <pxShiftJIS.h>
#include <pxTText.h>

#include "../../Generic/Japanese.h"
#include "../../Generic/MessageBox.h"

#include "../resource.h"

static void _Initialize( HWND hDlg, int32_t w )
{
	// ステータス
	switch( g_pxtn->Woice_Get( w )->get_type() )
	{
	case pxtnWOICE_PCM :
		SetDlgItemText( hDlg, IDC_TYPE, _T("PCM"         ) );
		break;
	case pxtnWOICE_PTN :
		SetDlgItemText( hDlg, IDC_TYPE, _T("pxtone Noise") );
		break;
	case pxtnWOICE_PTV :
		SetDlgItemText( hDlg, IDC_TYPE, _T("pxtone Voice") );

		EnableWindow( GetDlgItem( hDlg, IDC_BASICKEY       ), false );
		EnableWindow( GetDlgItem( hDlg, IDC_TEXT_BASICKEY  ), false );

		ShowWindow( GetDlgItem( hDlg, IDC_CHECK_WAVELOOP ), SW_HIDE );
		ShowWindow( GetDlgItem( hDlg, IDC_CHECK_SMOOTH   ), SW_HIDE );
		ShowWindow( GetDlgItem( hDlg, IDC_CHECK_BEATFIT  ), SW_HIDE );
		ShowWindow( GetDlgItem( hDlg, IDC_BASICKEY_DOWN  ), SW_HIDE );
		ShowWindow( GetDlgItem( hDlg, IDC_BASICKEY_UP    ), SW_HIDE );
		ShowWindow( GetDlgItem( hDlg, IDC_TUNING         ), SW_HIDE );
		ShowWindow( GetDlgItem( hDlg, IDC_TEXT_TUNING    ), SW_HIDE );
		break;

	case pxtnWOICE_OGGV:
		SetDlgItemText( hDlg, IDC_TYPE, _T("Ogg Vorbis") );
		break;
	}
}

static void _SetParameter( HWND hDlg, int32_t w )
{
	TCHAR status[ 100 ];
	const pxtnWoice     *p_w   = g_pxtn->Woice_Get( w ); if( !p_w  ) return;
	const pxtnVOICEUNIT *p_vc  = p_w->get_voice( 0 );    if( !p_vc ) return;
	const pxtnPulse_PCM   *p_pcm =  p_vc->p_pcm;
	int32_t ch       = p_pcm->get_ch      ();
	int32_t sps      = p_pcm->get_sps     ();
	int32_t bps      = p_pcm->get_bps     ();
	int32_t smp_body = p_pcm->get_smp_body();

	// ステータス
	switch( p_w->get_type() )
	{
	case pxtnWOICE_PCM :
		if( ch == 2 ) _stprintf_s( status, 100, _T("Stereo %dbit %dHz %dsample"), bps, sps, smp_body );
		else          _stprintf_s( status, 100, _T("Mono %dbit %dHz %dsample"  ), bps, sps, smp_body );
		break;

	case pxtnWOICE_OGGV:
#ifdef pxINCLUDE_OGGVORBIS
		int ch, sps, smp_num;
		p_vc->p_oggv->GetInfo( &ch, &sps, &smp_num );
		if( ch == 2 ) _stprintf_s( status, 100, _T("Stereo %dHz %dsample"      ), sps, smp_num );
		else          _stprintf_s( status, 100, _T("Mono %dHz %dsample"        ), sps, smp_num );
#else
		_stprintf_s( status, 100, _T("not support ogg.") );
#endif
		break;

	case pxtnWOICE_PTN : _stprintf_s( status, 100, _T("%0.2f sec"   ), p_vc->p_ptn->get_sec() ); break;
	case pxtnWOICE_PTV : _stprintf_s( status, 100, _T("ptvoice [%d]"), p_w->get_voice_num  () ); break;
	}

	// フラグ
	if( p_w->get_type() != pxtnWOICE_PTV )
	{
		if( p_vc->voice_flags & PTV_VOICEFLAG_WAVELOOP ) CheckDlgButton( hDlg, IDC_CHECK_WAVELOOP, true  );
		else                                             CheckDlgButton( hDlg, IDC_CHECK_WAVELOOP, false );
		if( p_vc->voice_flags & PTV_VOICEFLAG_SMOOTH   ) CheckDlgButton( hDlg, IDC_CHECK_SMOOTH,   true  );
		else                                             CheckDlgButton( hDlg, IDC_CHECK_SMOOTH,   false );
		if( p_vc->voice_flags & PTV_VOICEFLAG_BEATFIT  ) CheckDlgButton( hDlg, IDC_CHECK_BEATFIT,  true  );
		else                                             CheckDlgButton( hDlg, IDC_CHECK_BEATFIT,  false );
	}

	pxTText tt; if( tt.set_sjis_to_t( p_w->get_name_buf( NULL ) ) )
	{
		SetDlgItemText( hDlg, IDC_NAME, tt.tchr() );
	}
	SetDlgItemText( hDlg, IDC_STATUS, status    );

	TCHAR str[10] = {0};
	_stprintf_s( str, 10, _T("%0.3f"), p_vc->tuning );
	SetDlgItemText( hDlg, IDC_TUNING  , str );
	SetDlgItemInt ( hDlg, IDC_BASICKEY, p_vc->basic_key / 0x100, false );
}

static bool _GetParameter( HWND hDlg, int32_t woice_index )
{
	pxtnWoice     *p_w  = g_pxtn->Woice_Get_variable( woice_index ); if( !p_w  ) return false;
	pxtnVOICEUNIT *p_vc = p_w->get_voice_variable   (           0 ); if( !p_vc ) return false;

	TCHAR name[ pxtnMAX_TUNEWOICENAME + 1 ];
	pxTText tt;

	GetDlgItemText( hDlg, IDC_NAME, name, pxtnMAX_TUNEWOICENAME+1 );

	if( !tt.set_TCHAR_to_sjis( name ) ){ mbox_t_ERR( hDlg, _T("name to sjis: %s"), name ); return false; }

	int32_t size = 0;
	pxShiftJIS_check_size( tt.sjis(), &size, true );
	p_w->set_name_buf( tt.sjis(), size );

	// フラグ
	if( p_w->get_type() != pxtnWOICE_PTV )
	{
		if( IsDlgButtonChecked( hDlg, IDC_CHECK_WAVELOOP ) ) p_vc->voice_flags  |=  PTV_VOICEFLAG_WAVELOOP;
		else                                                 p_vc->voice_flags  &= ~PTV_VOICEFLAG_WAVELOOP;
		if( IsDlgButtonChecked( hDlg, IDC_CHECK_SMOOTH   ) ) p_vc->voice_flags  |=  PTV_VOICEFLAG_SMOOTH  ;
		else                                                 p_vc->voice_flags  &= ~PTV_VOICEFLAG_SMOOTH  ;
		if( IsDlgButtonChecked( hDlg, IDC_CHECK_BEATFIT  ) ) p_vc->voice_flags  |=  PTV_VOICEFLAG_BEATFIT ;
		else                                                 p_vc->voice_flags  &= ~PTV_VOICEFLAG_BEATFIT ;
	}

	TCHAR str[ 10 ] = {0};
	GetDlgItemText( hDlg, IDC_TUNING,  str, 10 ); p_vc->tuning = (float)_ttof( str );
	if( p_vc->tuning <   0 ) p_vc->tuning *=  -1;
	if( p_vc->tuning > 256 ) p_vc->tuning  = 256;

	if( p_w->get_type() != pxtnWOICE_PTV )
	{
		int32_t basic_key;
		basic_key = GetDlgItemInt( hDlg, IDC_BASICKEY, NULL, false );
		if( basic_key <  21 ) basic_key =  21;
		if( basic_key > 108 ) basic_key = 108;
		p_vc->basic_key = basic_key * 0x100;
	}

	return true;
}

static void _ShiftBasicKey( HWND hDlg, int32_t shift )
{
	int32_t basic_key;

	basic_key = GetDlgItemInt( hDlg, IDC_BASICKEY, NULL, false );
	basic_key += shift;
	if( basic_key <  21 ) return;
	if( basic_key > 108 ) return;;

	SetDlgItemInt( hDlg, IDC_BASICKEY, basic_key, false );

}

//コールバック
INT_PTR CALLBACK
dlg_Woice( HWND hDlg, UINT msg, WPARAM w, LPARAM l )
{
	static int32_t _index;

	switch( msg )
	{

	//ダイアログ起動
	case WM_INITDIALOG:

		pxwWindowRect_center(       hDlg );
		Japanese_DialogItem_Change( hDlg );

		SendDlgItemMessage( hDlg, IDC_NAME, EM_SETLIMITTEXT, pxtnMAX_TUNEWOICENAME, 0 );

		_index = (int32_t)l;
		if( !g_pxtn->Woice_Get( _index ) ){ EndDialog( hDlg, false ); break; }
		_Initialize( hDlg, _index );
		_SetParameter( hDlg, _index );

		break;

	//ボタンクリック
	case WM_COMMAND:

		switch( LOWORD( w ) ){
		case IDOK:
			if( !_GetParameter( hDlg, _index ) ) break;
			EndDialog( hDlg, true );
			break;

		case IDCANCEL:
			EndDialog( hDlg, false );
			break;

		case IDC_BASICKEY_DOWN: _ShiftBasicKey( hDlg, -12 ); break;
		case IDC_BASICKEY_UP:   _ShiftBasicKey( hDlg,  12 ); break;

		}
		break;

	default: return false;

	}
	return true;
}
