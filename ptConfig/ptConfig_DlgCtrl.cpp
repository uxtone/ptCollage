
void ptConfig_cmb_quality_init( HWND hdlg, int id_ch, int id_sps, bool b_jp )
{
	static TCHAR *mode_channel_e[] = { _T("mono"    ), _T("stereo"  ) };
	static TCHAR *mode_channel_j[] = { _T("モノラル"), _T("ステレオ") };
	static TCHAR *mode_sps      [] = { _T("11025Hz" ), _T("22050Hz" ), _T("44100Hz"), _T("48000Hz") };

	if( b_jp )
	{
		for( int i = 0; i < 2; i++ ) SendDlgItemMessage( hdlg, id_ch , CB_ADDSTRING, 0, (LPARAM)mode_channel_j[i] );
	}
	else
	{
		for( int i = 0; i < 2; i++ ) SendDlgItemMessage( hdlg, id_ch , CB_ADDSTRING, 0, (LPARAM)mode_channel_e[i] );
	}

	for( int i = 0; i < 4; i++ ) SendDlgItemMessage( hdlg, id_sps,    CB_ADDSTRING, 0, (LPARAM)mode_sps[ i] );
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