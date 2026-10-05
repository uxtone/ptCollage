#include <uxStr.h>

void ptConfig_cmb_quality_init( HWND hdlg, int id_ch, int id_sps, bool b_jp )
{
	static const char* mode_channel_e[] = { "mono", "stereo" };
	static const char* mode_channel_j[] = { "モノラル", "ステレオ" };
	static const char* mode_sps      [] = { "11025Hz", "22050Hz", "44100Hz", "48000Hz" };

	if( b_jp )
	{
		for( int i = 0; i < 2; i++ ) SendDlgItemMessage( hdlg, id_ch , CB_ADDSTRING, 0, uxLP( mode_channel_j[i] ) );
	}
	else
	{
		for( int i = 0; i < 2; i++ ) SendDlgItemMessage( hdlg, id_ch , CB_ADDSTRING, 0, uxLP( mode_channel_e[i] ) );
	}

	for( int i = 0; i < 4; i++ ) SendDlgItemMessage( hdlg, id_sps,    CB_ADDSTRING, 0, uxLP( mode_sps[ i] ) );
}

void ptConfig_cmb_quality_set(
	HWND hdlg,
	int id_ch , int ch ,
	int id_sps, int sps )
{
	switch( ch )
	{
	case     1: SendDlgItemMessage( hdlg, id_ch, CB_SETCURSEL, 0, 0 ); break;
	case     2: SendDlgItemMessage( hdlg, id_ch, CB_SETCURSEL, 1, 0 ); break;
	default:    SendDlgItemMessage( hdlg, id_ch, CB_SETCURSEL, 0, 0 ); break;
	}

	switch( sps )
	{
	case 11025: SendDlgItemMessage( hdlg, id_sps,     CB_SETCURSEL, 0, 0 ); break;
	case 22050: SendDlgItemMessage( hdlg, id_sps,     CB_SETCURSEL, 1, 0 ); break;
	case 48000: SendDlgItemMessage( hdlg, id_sps,     CB_SETCURSEL, 3, 0 ); break;
	default:    SendDlgItemMessage( hdlg, id_sps,     CB_SETCURSEL, 2, 0 ); break;
	}
}

void ptConfig_cmb_quality_get(
	HWND hdlg,
	int id_ch , int *p_ch ,
	int id_sps, int *p_sps )
{

	switch( SendDlgItemMessage( hdlg, id_ch , CB_GETCURSEL, 0, 0 ) )
	{
	case  0: *p_ch  =     1; break;
	default: *p_ch  =     2; break;
	}

	switch( SendDlgItemMessage( hdlg, id_sps, CB_GETCURSEL, 0, 0 ) )
	{
	case  0: *p_sps = 11025; break;
	case  1: *p_sps = 22050; break;
	case  3: *p_sps = 48000; break;
	default: *p_sps = 44100; break;
	}
}
