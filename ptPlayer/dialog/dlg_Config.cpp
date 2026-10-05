
#include <pxwWindowRect.h>
#include <uxStr.h>
#include <pxwFilePath.h>

#include "../../Generic/Japanese.h"

#include "../../ptConfig/ptConfig_DlgCtrl.h"
#include "../../ptConfig/ptConfig.h"

#include "../resource.h"

static const uxSS<2>  _dummy_font_name = "-";
static HFONT        _hFont           = NULL   ;

static int32_t CALLBACK _EnumFont( ENUMLOGFONT *p_elf, NEWTEXTMETRIC *p_ntm, int32_t type, LPARAM l )
{
	int32_t len  =       0;
	HWND    hWnd = (HWND)l;

	if( type & TRUETYPE_FONTTYPE && IsWindow( hWnd ) )
	{
		len = 0; while( p_elf->elfLogFont.lfFaceName[ len ] ) len++; // OS text: wide
		for( int32_t i = 0; i < len; i++ )
		{
			if( p_elf->elfLogFont.lfFaceName[ i ] == '@' ) break;
			SendDlgItemMessage( hWnd, IDC_COMBO_FONT, CB_ADDSTRING, 0, (LPARAM)p_elf->elfLogFont.lfFaceName );
			break;
		}
	}
	return 1;
}

static bool _PreviewFont( HWND hDlg )
{
	uxSS<BUFSIZE_FONTNAME>         name = {0};
	const char* p = NULL;

	GetDlgItemText( hDlg, IDC_COMBO_FONT, uxTOut( name ), BUFSIZE_FONTNAME );

	if(ux_stricmp( name, _dummy_font_name ) ) p = name;

	if( _hFont ){ DeleteObject( _hFont ); _hFont = NULL; }

	_hFont = CreateFont( 12, 0,
						 0,              // 角度
						 0,
						 FW_NORMAL,
						 false,          // Italic
						 false,          // Underline
						 false,          // StrikeOut
						 DEFAULT_CHARSET,
						 OUT_TT_PRECIS,
						 CLIP_DEFAULT_PRECIS,
						 DEFAULT_QUALITY,
						 FIXED_PITCH,
						 uxT( p ) );

	if( _hFont ) SendMessage( GetDlgItem( hDlg, IDC_FONTSAMPLE ), WM_SETFONT, (WPARAM)_hFont, true );


	return true;
}

static void _InitDialog( HWND hDlg )
{
	ptConfig_cmb_quality_init( hDlg, IDC_COMBO_CHANNEL, IDC_COMBO_SPS, Japanese_Is() );
	{
		SendDlgItemMessage( hDlg, IDC_COMBO_FONT, CB_ADDSTRING, 0, uxLP( _dummy_font_name ) );
		HDC hDC = GetDC( hDlg );
		EnumFontFamilies(      hDC, NULL, (FONTENUMPROC)_EnumFont, (LPARAM)hDlg );
		ReleaseDC(       hDlg, hDC );
	}
}

static void _SetParameter( HWND hDlg, const ptConfig *p_c )
{
	{
		uxSS<10> str;
		ux_sprintf_s( str, 10, "%0.2f", p_c->strm->buf_sec );
		SetDlgItemText( hDlg, IDC_BUFFER, uxT( str ) );
	}

	ptConfig_cmb_quality_set( hDlg,
							  IDC_COMBO_CHANNEL, p_c->strm->ch_num,
							  IDC_COMBO_SPS    , p_c->strm->sps );
	{
		int32_t i = SendDlgItemMessage( hDlg, IDC_COMBO_FONT, CB_FINDSTRING, 0, uxLP( p_c->font->name ) );
		if( i == CB_ERR ) i = 0;
		SendDlgItemMessage( hDlg, IDC_COMBO_FONT, CB_SETCURSEL , i, 0 );
		_PreviewFont( hDlg );
	}
}

static bool _GetInputParameter( HWND hDlg, ptConfig *p_c )
{

	uxSS<10> str = {0}; GetDlgItemText( hDlg, IDC_BUFFER, uxTOut( str ), 10 ); p_c->strm->buf_sec = (float)_ttof( uxT( str ) );

	ptConfig_cmb_quality_get( hDlg,
							  IDC_COMBO_CHANNEL, &p_c->strm->ch_num,
							  IDC_COMBO_SPS    , &p_c->strm->sps );

	{
		GetDlgItemText( hDlg, IDC_COMBO_FONT, uxTOut( p_c->font->name ), BUFSIZE_FONTNAME );
		if( !ux_stricmp( p_c->font->name, _dummy_font_name ) ) memset( p_c->font->name, 0, sizeof(p_c->font->name) );
	}

	return true;
}

static bool _CheckParameter( HWND hDlg, ptConfig *p_c )
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
	static ptConfig* _p_cfg;
	static bool       _bInit = false;

	switch( msg )
	{
	case WM_INITDIALOG:

		_p_cfg = (ptConfig*)l;

		pxwWindowRect_center(       hDlg );
		Japanese_DialogItem_Change( hDlg );
		_InitDialog(   hDlg );
		_SetParameter( hDlg, _p_cfg );
		_bInit = true;

		break;

	case WM_CLOSE:
		if( _hFont ){ DeleteObject( _hFont ); _hFont = NULL; }
		break;

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
			_bInit = false;
			EndDialog( hDlg, false );
			break;

		case IDC_DEFAULT:
			_p_cfg->set_default();
			_SetParameter( hDlg, _p_cfg );
			break;

		case IDC_COMBO_FONT:
			if( HIWORD(w) == CBN_SELCHANGE ) _PreviewFont( hDlg );
			break;
		}
		break;

	default: return false;

	}
	return true;
}
