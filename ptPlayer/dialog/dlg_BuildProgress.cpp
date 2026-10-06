
#include <pxStdDef.h>


#include <pxPath.h>

#include <pxtnService.h>
extern pxtnService* g_pxtn;

#include <pxwWindowRect.h>

#include "../../Generic/Japanese.h"

#include "../resource.h"

#include "dlg_BuildProgress.h"


extern uxSS<64> g_app_name_t;
extern HWND  g_hWnd_Main   ;

bool if_Player_Callback_Sampled( void* user, const pxtnService* pxtn );

static HWND   _hDlg    = NULL;
static HANDLE _hThread = NULL;


typedef struct
{
	unsigned short formatID;      // PCM:0x0001
	unsigned short ch;            // モノラル:1 / ステレオ:2
	uint32_t            sps;           // サンプリングレート
	uint32_t            byte_per_sec;  // 秒間バイト数
	unsigned short block_size;    // ブロックサイズ（）
	unsigned short bps;           // 1サンプルのビット数
	unsigned short ext;           // 拡張部分のサイズ(リニアPCMには無いとされている。ゼロを入れる)
}
_WAVEFORMATCHUNK;

static BOOL CALLBACK _Progress( DWORD val )
{
	static int32_t _val_old = 0;
	if( _val_old != val )
	{
		_val_old  = val;
		SendDlgItemMessage( _hDlg, IDC_PROGRESS, PBM_SETPOS, (WPARAM)val, 0 );
	}
	return true;
}

static BOOL CALLBACK _LastProc( LPVOID p_param )
{
	BUILDPROGRESSSTRUCT *p_build = (BUILDPROGRESSSTRUCT *)p_param;

	switch( p_build->result_exit )
	{
	// success.
	case enum_BuildTuneExit_Success:
	{
		uxDS p_name;
		uxSS<MAX_PATH> str;
		p_name = pxPath_name( p_build->output_path );
		if( Japanese_Is() ) ux_sprintf_s( str, MAX_PATH, "ビルド完了 -> %s (%0.3f秒)", *p_name, (float)p_build->result_count / 1000 );
		else                ux_sprintf_s( str, MAX_PATH, "Build Completed -> %s (%0.3fsec)", *p_name, (float)p_build->result_count / 1000 );
		Japanese_MessageBox( _hDlg, str, g_app_name_t, MB_OK|MB_ICONINFORMATION );
		PostMessage( _hDlg, WM_COMMAND, IDOK, 0 );
	}
	break;

	// error..
	case enum_BuildTuneExit_Error:
		Japanese_MessageBox( _hDlg, "build", "error", MB_OK|MB_ICONEXCLAMATION );
		DeleteFile( uxT( p_build->output_path ) );
		PostMessage( _hDlg, WM_COMMAND, IDOK, 0 );
		break;

	// cancel.
	case enum_BuildTuneExit_Stop:
		DeleteFile( uxT( p_build->output_path ) );
		PostMessage( _hDlg, WM_COMMAND, IDOK, 0 );
		break;
	}

	return true;
}



enum _enum_Mode
{
	_enum_Mode_Idle = 0,
	_enum_Mode_Start,
	_enum_Mode_Building,
	_enum_Mode_Stop,
	_enum_Mode_Error,
};

static _enum_Mode _mode = _enum_Mode_Idle;

#include <pxRiffWav.h>

static DWORD WINAPI _BuildThread( LPVOID lpParam )
{
	BUILDPROGRESSSTRUCT* p_build = (BUILDPROGRESSSTRUCT *)lpParam;

	char* p  = NULL;
	uint32_t   sample_total;
	uint32_t   sample_extrafade;
	int32_t    block;
	DWORD      count;

	int32_t    beat_num  ;
	float      beat_tempo;
	int32_t    meas_num  ;
	int32_t    ch_num, sps;

	FILE*            fp = NULL;
	pxDescriptor     desc;
	pxRiffWav_output wo  ;

	p_build->result_exit = enum_BuildTuneExit_Error;

	g_pxtn->master->Get( &beat_num, &beat_tempo, NULL, &meas_num );
	g_pxtn->get_destination_quality( &ch_num, &sps );

	_Progress( 0 );

	g_pxtn->moo_preparation     ( NULL );
	g_pxtn->set_sampled_callback( NULL, NULL );

	block = ch_num * pxtnBITPERSAMPLE / 8;

	p = (char*)malloc( block ); if( !p ) goto term;

	sample_extrafade = (int32_t)( p_build->sec_extrafade * sps );

	switch( p_build->scope )
	{
	case BUILDTUNESCOPE_TOPLAST:
		sample_total = (uint32_t)( (double)sps * 60 * (double)(meas_num * beat_num) / (double)beat_tempo ) +
			sample_extrafade;
		break;
	case BUILDTUNESCOPE_BYTIME:
		sample_total = (int32_t)( p_build->sec_playtime * sps ) + sample_extrafade;
		break;
	}

	// open output pcm..

	if( !( fp = ux_fopen( p_build->output_path, "wb" ) ) ) goto term;
	if( !  desc.set_file_w( fp )                            ) goto term;

	if( !wo.write_header( &desc, ch_num, sps, pxtnBITPERSAMPLE, sample_total ) ) goto term;

	count = GetTickCount();

	for( uint32_t s = 0; s < sample_total; s++ )
	{
		if( _mode == _enum_Mode_Stop ){ p_build->result_exit = enum_BuildTuneExit_Stop; goto term; }
		if( s == sample_total - sample_extrafade ) g_pxtn->moo_set_fade( -1, p_build->sec_extrafade );
		g_pxtn->Moo( p, block );
		if( !wo.write_sample( &desc, p, 1 ) ) goto term;

		_Progress( 100 * s / sample_total );
	}

	if( !wo.terminate( &desc ) ) goto term;

	p_build->result_count = GetTickCount() - count;
	p_build->result_exit  = enum_BuildTuneExit_Success;
term:

	_Progress( 0 );

	if( fp ) fclose( fp ); fp = NULL;

	if( p  ) free(   p  );

	_LastProc( p_build );

	_mode = _enum_Mode_Idle;
	g_pxtn->set_sampled_callback( if_Player_Callback_Sampled, g_hWnd_Main );

	return 1;
}


static bool _build_Start( const BUILDPROGRESSSTRUCT *p_build )
{
	static DWORD _thread_id;

	if( _mode != _enum_Mode_Idle ) return false;

	_hThread = CreateThread( NULL, 0, _BuildThread, (LPVOID)p_build, CREATE_SUSPENDED, &_thread_id );
	if( !_hThread ) return false;
	switch( 0 )
	{
	case 0 : SetThreadPriority( _hThread, THREAD_PRIORITY_NORMAL  ); break;
	case 1 : SetThreadPriority( _hThread, THREAD_PRIORITY_HIGHEST ); break;
	case 2 : SetThreadPriority( _hThread, THREAD_PRIORITY_LOWEST  ); break;
	case 3 : SetThreadPriority( _hThread, THREAD_PRIORITY_IDLE    ); break;
	}
	ResumeThread( _hThread );

	_mode = _enum_Mode_Building;

	return true;
}

static bool _build_Stop()
{
	if( _mode != _enum_Mode_Building ) return false;
	_mode = _enum_Mode_Stop;
	return true;
}

INT_PTR CALLBACK
dlg_BuildProgress( HWND hDlg, UINT msg, WPARAM w, LPARAM l )
{
	static BUILDPROGRESSSTRUCT _build;


	switch( msg )
	{

	case WM_INITDIALOG:
		_hDlg  = hDlg;
		_build = *(BUILDPROGRESSSTRUCT *)l;

		if( Japanese_Is() ) SetWindowText( hDlg, uxT( "ビルド中…" ) );
		else                SetWindowText( hDlg, uxT( "Building.." ) );

		if( !_build_Start( &_build ) )
		{
			Japanese_MessageBox( hDlg, "build", "error", MB_OK|MB_ICONEXCLAMATION );
			EndDialog( hDlg, false );
			break;
		}

		{
			uxSS<100> str;
			int ch_num, sps;

			g_pxtn->get_destination_quality( &ch_num, &sps );

			if( Japanese_Is() )
			{
				if( ch_num == 1 ) ux_sprintf_s( str, 100, "モノラル / %dbit / %dHz", pxtnBITPERSAMPLE, sps );
				else              ux_sprintf_s( str, 100, "ステレオ / %dbit / %dHz", pxtnBITPERSAMPLE, sps );
				SetDlgItemText( hDlg, IDCANCEL, uxT( "中止" ) );
			}else{
				if( ch_num == 1 ) ux_sprintf_s( str, 100, "mono / %dbit / %dHz", pxtnBITPERSAMPLE, sps );
				else              ux_sprintf_s( str, 100, "stereo / %dbit / %dHz", pxtnBITPERSAMPLE, sps );
			}
			SetDlgItemText( hDlg, IDC_STATUS, uxT( str ) );
		}

		pxwWindowRect_center( hDlg );
		break;

	case WM_COMMAND:

		switch( LOWORD(w) )
		{
		case IDOK:
			EndDialog( hDlg, true );
			break;

		case IDCANCEL:
			_build_Stop();
			EnableWindow( GetDlgItem( hDlg, IDCANCEL ), false );
			break;
		}
	default: return false;

	}
	return true;
}
