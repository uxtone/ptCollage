
#include <pxTText.h>

#include <pxtnService.h>
extern pxtnService *g_pxtn;

#include <pxwWindowRect.h>

#include <pxShiftJIS.h>

#include "../../Generic/Japanese.h"

#include "../Project.h"

#include "../resource.h"

#include "../../ptConfig/ptConfig.h"

bool IsShiftJIS( unsigned char c );


static void _InitDialog( HWND hDlg )
{
	TCHAR *mode_beat[]      = {  _T("2"),_T("3"),_T("4"),_T("5"),_T("6"),_T("7"),_T("8"),_T("9"),_T("15") };
	for( int i = 0; i < 9; i++ ) SendDlgItemMessage( hDlg, IDC_COMBO_BEAT,CB_ADDSTRING, 0, (LPARAM)mode_beat[i] );
}

static bool _SetParameter( HWND hDlg )
{
	TCHAR   str[10];
	int32_t i;
	int32_t beat_num, beat_clock, meas_num;
	float   beat_tempo;

	const char* name_c = NULL;
	if( name_c = g_pxtn->text->get_name_buf( NULL ) )
	{
		pxTText tt; if( !tt.set_sjis_to_t( name_c ) ) return false;
		SetDlgItemText( hDlg, IDC_NAME, tt.tchr() );
	}

	g_pxtn->master->Get( &beat_num, &beat_tempo, &beat_clock, &meas_num );

	SendDlgItemMessage( hDlg, IDC_NAME, EM_SETLIMITTEXT, MAX_PROJECTNAME, 0 );

	SetDlgItemInt(  hDlg, IDC_MEASNUM,   meas_num,       true );
	SetDlgItemInt(  hDlg, IDC_BEATCLOCK, beat_clock,     true );

	_stprintf_s( str, 10, _T("%0.0f"), beat_tempo );
	SetDlgItemText( hDlg, IDC_TEMPO, str );

	switch( beat_num )
	{
	case  2: i = 0; break;
	case  3: i = 1; break;
	case  4: i = 2; break;
	case  5: i = 3; break;
	case  6: i = 4; break;
	case  7: i = 5; break;
	case  8: i = 6; break;
	case  9: i = 7; break;
	case 15: i = 8; break;
	default: i = 2; break;
	}

	SendDlgItemMessage( hDlg, IDC_COMBO_BEAT, CB_SETCURSEL, i, 0 );
	return true;
}

static void _GetParameter( HWND hDlg,
		TCHAR*   p_name      ,
		int32_t* p_beat_num  ,
		float*   p_beat_tempo,
		int32_t* p_beat_clock,
		int32_t* p_meas_num )
{
	TCHAR str[10];

	if( p_name ) GetDlgItemText( hDlg, IDC_NAME, p_name, MAX_PROJECTNAME + 1 );

	GetDlgItemText( hDlg, IDC_TEMPO,  str, 10 );

	*p_beat_tempo = (float)_ttof( str );
	*p_meas_num   = GetDlgItemInt( hDlg, IDC_MEASNUM  , NULL, true );
	*p_beat_clock = GetDlgItemInt( hDlg, IDC_BEATCLOCK, NULL, true );

	switch( SendDlgItemMessage( hDlg, IDC_COMBO_BEAT, CB_GETCURSEL, 0, 0 ) ){
	case  0: *p_beat_num =  2; break;
	case  1: *p_beat_num =  3; break;
	case  2: *p_beat_num =  4; break;
	case  3: *p_beat_num =  5; break;
	case  4: *p_beat_num =  6; break;
	case  5: *p_beat_num =  7; break;
	case  6: *p_beat_num =  8; break;
	case  7: *p_beat_num =  9; break;
	case  8: *p_beat_num = 15; break;
	default: *p_beat_num =  4; break;
	}

}

bool _CheckParameter( HWND hDlg, float beat_tempo, int32_t meas_num )
{
	if( !meas_num )
	{
		Japanese_MessageBox( hDlg, _T("meas"), _T("error"), MB_OK|MB_ICONEXCLAMATION );
		return false;
	}

	if( beat_tempo < 20 || beat_tempo > 600 )
	{
		Japanese_MessageBox( hDlg, _T("tempo (20 - 600)"), _T("error"), MB_OK|MB_ICONEXCLAMATION );
		return false;
	}
	return true;
}

static void _TotalSampleSize( HWND hDlg )
{
	int32_t  beat_num, beat_clock, meas_num;
	float beat_tempo;
	TCHAR str[ 100 ];

	double size;

	int32_t bps =    16;
	int32_t sps = 44100;
	int32_t ch  =     2;

	_GetParameter( hDlg, NULL, &beat_num, &beat_tempo, &beat_clock, &meas_num );

	if( beat_tempo )
	{
		size = (double)meas_num * (double)beat_num * bps * sps * ch / 8 * 60 / (double)beat_tempo;
	}
	else
	{
		size = 0;
	}

	_stprintf_s( str, 100, _T("%0.2f"), (float)(size/1000) ); SetDlgItemText( hDlg, IDC_SAMPLESIZE, str );

	if( beat_tempo )
	{
		size = (double)meas_num * (double)beat_num * 60 / (double)beat_tempo;
	}
	else
	{
		size = 0;
	}
	_stprintf_s( str, 100, _T("%0.2f"), (float)size ); SetDlgItemText( hDlg, IDC_SAMPLESEC , str );

}

//コールバック
INT_PTR CALLBACK
dlg_ProjectOption( HWND hDlg, UINT msg, WPARAM w, LPARAM l )
{
	// IME off
	switch( msg )
	{
	case WM_COMMAND:
		switch( LOWORD( w ) )
		{
		case IDC_TEMPO:
		case IDC_MEASNUM:

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

		pxwWindowRect_center      ( hDlg );
		Japanese_DialogItem_Change( hDlg );
		_InitDialog               ( hDlg );
		_SetParameter             ( hDlg );
		_TotalSampleSize          ( hDlg );
		break;

	//ボタンクリック
	case WM_COMMAND:

		switch( LOWORD( w ) )
		{
		case IDOK:
			{
				TCHAR   name_t[ MAX_PROJECTNAME + 1 ] = {0};
				int32_t b_num, q_clock, m_num;
				float   b_tempo;
				_GetParameter(   hDlg, name_t, &b_num, &b_tempo, &q_clock, &m_num );

				if( _CheckParameter( hDlg, b_tempo, m_num ) )
				{
					pxTText tt; if( tt.set_TCHAR_to_sjis( name_t ) )
					{
						int32_t size = 0;
						pxShiftJIS_check_size( tt.sjis(), &size, true );
						g_pxtn->text->set_name_buf( tt.sjis(), size );
					}
					g_pxtn->master->Set( b_num, b_tempo, q_clock );
					g_pxtn->master->set_meas_num( m_num );
					EndDialog( hDlg, true );
				}
			}
			break;

		case IDCANCEL:
			EndDialog( hDlg, false );
			break;

		case IDC_TEMPO:
		case IDC_MEASNUM:
		case IDC_BEATNUM:
			if( HIWORD( w ) == EN_CHANGE ) _TotalSampleSize( hDlg );
			break;

		}
		break;

	default:return false;

	}
	return true;
}
