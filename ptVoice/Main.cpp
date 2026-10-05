
#pragma comment(lib, "d3d9"   )
#pragma comment(lib, "d3dx9"  )

#include <pxMem.h>
#include <pxDebugLog.h>

#include <pxStrT.h>

#include <pxFile2.h>
static pxFile2* _app_file_common  = NULL;
static pxFile2* _app_file_data    = NULL;
static pxFile2* _app_file_profile = NULL;
#include <pxPath.h>

#include <pxtnPulse_Frequency.h>
pxtnPulse_Frequency* g_freq     = NULL;

#include <pxtnService.h>
pxtnService*         g_pxtn     = NULL;

#include <pxtonewinXA2.h>
pxtonewinXA2*        g_strm_xa2 = NULL;

#include <pxtonewinWoice.h>
pxtonewinWoice*      g_strm_woi = NULL;

#include <pxwDx09Draw.h>
pxwDx09Draw*         g_dxdraw = NULL;

#include <pxwAlteration.h>
pxwAlteration* g_alte         = NULL;

#include <pxMouse.h>
pxMouse* g_mouse = NULL;

#include "./interface/Interface.h"
Interface* g_interface = NULL;

#include <pxwFilePath.h>
#include <pxwWindowRect.h>
#include <pxwXAudio2Keep.h>

#include "../Generic/MessageBox.h"
#include "../Generic/if_Generic.h"
#include "../Generic/cls_ExistingWindow.h"
#include "../Generic/KeyControl.h"
#include "../Generic/Japanese.h"

#include "../Generic/pxMidiIn.h"
pxMidiIn *g_midi_in = NULL;


#include <pxwPathDialog.h>

pxwPathDialog* g_path_dlg_ptv = NULL;
pxwPathDialog* g_path_dlg_wav = NULL;
pxwPathDialog* g_path_dlg_txt = NULL;


static const TCHAR* _title_save_ptv_j  = _T("ptvoice を保存する"    );
static const TCHAR* _title_save_ptv_e  = _T("Save ptvoice"          );
static const TCHAR* _title_load_ptv_j  = _T("ptvoice を読み込む"    );
static const TCHAR* _title_load_ptv_e  = _T("Load ptvoice"          );
static const TCHAR* _title_oput_txt_j  = _T("テキスト形式で出力する");
static const TCHAR* _title_oput_txt_e  = _T("Output txt"            );

static const TCHAR* _app_name_t_jp = _T("ピストンボイス");
static const TCHAR* _app_name_t_en = _T("pxtone Voice"  );


void MainProf_set_file_profile( const pxFile2* file_profile );


#include "../ptConfig/ptConfig_Stream.h"

#include "../pxtoneTool/pxtoneTool.h"
#include "../ptConfig/ptConfig.h"

#include "./interface/Interface.h"

#include "./interface/ptuiLayout.h"


#include "./JapaneseTable.h"
#include "./ptvVelocity.h"
#include "./MidiInput.h"

#include "./resource.h"

#include "./ptVoice.h"

#include "./CursorKeyCtrl.h"
CursorKeyCtrl* g_curkey = NULL;

#include "./WoiceUnit.h"
WoiceUnit*     g_vunit  = NULL;


HINSTANCE      g_hInst                  = NULL;
HWND           g_hWnd_Main              = NULL;
HMENU          g_hMenu_Main             = NULL;
fRECT          g_view_rect              = { 0, 0, PTVOICE_W, PTVOICE_H };

TCHAR          g_dir_module[ MAX_PATH ] = { 0 };
TCHAR          g_app_name  [       32 ] = { 0 };
TCHAR*         g_dir_res                = NULL; // dummy for pxFile.
TCHAR*         g_main_rect_name         = _T("main.rect");

int32_t        g_client_min_w           = 0;
int32_t        g_client_min_h           = 0;

static TCHAR*  _class_name              = _T("Main"     );
static int32_t _mag                     = 1;

LRESULT CALLBACK WindowProc_Main( HWND hWnd, UINT msg, WPARAM w, LPARAM l );
void MainProc_set_file_profile( const pxFile2* file_profile );

TCHAR* g_posted_path = NULL;

//////////////////////////////////////////
// ローカル関数 //////////////////////////
//////////////////////////////////////////

// ウインドウクラスの登録
static bool _RegistWindowClass( HINSTANCE hInst, TCHAR *class_name, WNDPROC lpfnWndProc )
{
	WNDCLASSEX wc;

	memset( &wc, 0, sizeof(WNDCLASSEX) );

	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.lpszClassName = class_name;
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = lpfnWndProc;
	wc.hInstance     = hInst;        //インスタンス
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);//(HBRUSH)(COLOR_APPWORKSPACE + 1);
	wc.hIcon         = LoadIcon( hInst, _T("0") );
	wc.hCursor       = LoadCursor( NULL, IDC_ARROW       );

	if( !RegisterClassEx( &wc ) ) return false;

	return true;
}

// ファイルドロップ(アイコン)
static bool _Function_DropProjectFile( TCHAR** pp_path, const TCHAR *lpszArgs )
{
	TCHAR   path[MAX_PATH];
	int32_t a;

	if( !_tcslen( lpszArgs ) ) return false;

	if( lpszArgs[0] == '"' )
	{
		for( a = 0; a < MAX_PATH-1; a++ )
		{
			if( lpszArgs[a+1] == '"' || lpszArgs[a+1] == '\0') break;
			path[ a ] = lpszArgs[a+1];
		}
	}
	else
	{
		for( a = 0; a < MAX_PATH-1; a++ )
		{
			if(                         lpszArgs[a+0] == '\0') break;
			path[ a ] = lpszArgs[a+0];
		}
	}
	path[ a ] = '\0';

	return pxStrT_copy_allocate( pp_path, path );
}




// I/O..
static bool _io_read( void* user, void* p_dst, int32_t size, int32_t num )
{
	if( fread( p_dst, size, num, (FILE*)user ) != num ) return false; return true;
}
static bool _io_write( void* user, const void* p_dst, int32_t size, int32_t num )
{
	if( fwrite( p_dst, size, num, (FILE*)user ) != num ) return false; return true;
}
static  bool _io_seek( void* user,       int   mode , int32_t size              )
{
	if( fseek( (FILE*)user, size, mode ) ) return false; return true;
}
static bool _io_pos( void* user, int32_t* p_pos )
{
	fpos_t sz{};
	if( fgetpos( (FILE*)user, &sz ) ) return false;
	*p_pos  = pxFPOS_OFFSET(sz);
	return true;
}


//////////////////////////////////////////
// WinMain ///////////////////////////////
//////////////////////////////////////////

LRESULT CALLBACK WindowProc_Main( HWND hWnd, UINT msg, WPARAM w, LPARAM l );
void GetCompileDate( int32_t *year, int32_t *month, int32_t *day );

#define _WINDOW_STYLE (WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_CLIPCHILDREN)
//WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX

#include <pxwEntryPoint.h>
#include <pxwRuntime.h>

pxwENTRY_POINT( hInst, hPrevInst, args, nWinMode )
{
	pxwRuntime runtime;
	{ int rc; if( !runtime.init( _app_name_t_en, &rc ) ) return rc; }

	cls_EXISTINGWINDOW existing_window;
	static TCHAR *mutex_name   = _T("ptvoice"    );
	static TCHAR *mapping_name = _T("map_ptvoice");

	g_hInst = hInst;

	// 既に起動しているアプリを調べる
	if( !existing_window.Check( mutex_name, mapping_name, NULL ) ) return 0;

	// 汎用のパスを取得-------------------------------------
	pxwFilePath_GetModuleDirectory ( g_dir_module );

	_app_file_common  = new pxFile2(); _app_file_common ->init_base( _T("data_common"), false );
	_app_file_data    = new pxFile2(); _app_file_data   ->init_base( _T("data_ptv"   ), false );
	_app_file_profile = new pxFile2(); _app_file_profile->init_base( _T("temp_ptv"   ), true  );
	pxPath_setMode( pxPathMode_auto );

	MainProc_set_file_profile( _app_file_profile );
	ptvVelocity_init         ( _app_file_profile );
	pxwWindowRect_init       ( _app_file_profile );

	pxDebugLog_init( _T("temp_ptv"), _T("debuglog") );
	{
		int32_t y, m, d;
		GetCompileDate( &y, &m, &d );

		dlog_c( "pxtone Voice Start ------[v%04d/%02d/%02d-2]", y,m,d  );
	}

	// japanese..
	{
		TCHAR path[ MAX_PATH ] = {0}; _stprintf_s( path, MAX_PATH, _T("%s\\%s"), g_dir_module, _T("japanese.ico") );
		FILE* fp = _tfopen( path, _T("rb") );
		if( fp ){ fclose( fp ); JapaneseTable_init( true  ); }
		else    {               JapaneseTable_init( false ); }
	}
	if( Japanese_Is() ) _tcscpy( g_app_name, _app_name_t_jp );
	else                _tcscpy( g_app_name, _app_name_t_en );

	// ウインドウクラスを定義
	if( !_RegistWindowClass( hInst, _class_name, WindowProc_Main ) ) goto term;

	g_hMenu_Main = LoadMenu( hInst, _T("MENU_MAIN") );

	Japanese_MenuItem_Change( g_hMenu_Main );

	{
		RECT rc = { 0, 0, PTVOICE_W * pxwDx09Draw_system_mag(), PTVOICE_H * pxwDx09Draw_system_mag() };
		AdjustWindowRectEx( &rc, _WINDOW_STYLE, TRUE, 0 );
		g_client_min_w = abs(rc.left) + rc.right ;
		g_client_min_h = abs(rc.top ) + rc.bottom;
	}

	g_hWnd_Main  = CreateWindow(
		_class_name, g_app_name,
		_WINDOW_STYLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		g_client_min_w, g_client_min_h,
		HWND_DESKTOP ,
		g_hMenu_Main ,
		hInst,
		NULL );

	if( !existing_window.Mapping( mutex_name, mapping_name, g_hWnd_Main ) ) goto term;

	// ウインドウ位置の読み込み
	pxwWindowRect_load( g_hWnd_Main, g_main_rect_name, true, true );

	g_dxdraw = new pxwDx09Draw( _app_file_data );
	if( !g_dxdraw->init( g_hWnd_Main ) ){ mbox_c_ERR( NULL, "dxdraw." ); goto term; }

	g_curkey = new CursorKeyCtrl();
	g_vunit  = new WoiceUnit    ();

	// LOADING..表示
	{
		DWORD count = GetTickCount();

		if( !if_gen_splash( g_hWnd_Main, g_dxdraw, SURF_LOADING, 2 ) ){ mbox_c_ERR( NULL, "error: splash." ); goto term; }

		g_midi_in = new pxMidiIn   ();

		g_pxtn    = new pxtnService        ( _io_read, _io_write, _io_seek, _io_pos );
		if( g_pxtn->init() != pxtnOK ){ mbox_c_ERR( NULL, "pxtone." ); goto term; }

		g_freq    = new pxtnPulse_Frequency( _io_read, _io_write, _io_seek, _io_pos );
		if( !g_freq->Init()         ){ MessageBox( NULL, _T("init frequency." ), _T("error"), MB_OK ); goto term; }

		// config
		ptConfig cfg( _app_file_profile, ptvDEFAULT_SPS, ptvDEFAULT_CH_NUM , ptvDEFAULT_BUF_SEC );
		cfg.load();

		g_strm_xa2 = new pxtonewinXA2();
		if( !g_strm_xa2->init( g_pxtn, pxtnMAX_STREAMINGVOICE, pxtnMAX_STREAMINGVOICE ) )
		{
			Japanese_MessageBox( NULL, _T("Streaming Init")  , _T("error"), MB_OK|MB_ICONEXCLAMATION );
			goto term;
		}

		if( !g_vunit->init( g_strm_xa2 ) ){ MessageBox( NULL, _T("initialize voice"), _T("error"), MB_OK ); goto term; }
		ptvVelocity_load();

		if( !g_vunit->strm_ready() )
		{
			Japanese_MessageBox( NULL, _T("ready voice streaming"), _T("error"), MB_OK|MB_ICONEXCLAMATION );
			goto term;
		}

		if( !g_strm_xa2->stream_start( cfg.strm->ch_num, cfg.strm->sps, cfg.strm->buf_sec ) )
		{
			MessageBox( NULL, _T("stream start"), _T("error"), MB_OK|MB_ICONEXCLAMATION );
			goto term;
		}

		g_strm_woi = new pxtonewinWoice();
		if( !g_strm_woi->init( g_strm_xa2 ) )
		{
			Japanese_MessageBox( NULL, _T("pxtone tool lib"), _T("error"), MB_OK|MB_ICONEXCLAMATION );
			goto term;
		}

		{
			const TCHAR* title_save = NULL;
			const TCHAR* title_load = NULL;

			if( Japanese_Is() ){ title_save = _title_save_ptv_j; title_load = _title_load_ptv_j; }
			else               { title_save = _title_save_ptv_e; title_load = _title_load_ptv_e; }

			g_path_dlg_ptv = new pxwPathDialog();
			if( !g_path_dlg_ptv->init( _app_file_profile,
									   _T("ptvoice {*.ptvoice}\0*.ptvoice*\0All files {*.*}\0*.*\0\0"),
									   _T("ptvoice"     ),
									   _T("ptv-ptv.path"),
									   title_save, title_load, NULL ) ) goto term;

			g_path_dlg_wav = new pxwPathDialog();
			if( !g_path_dlg_wav->init( _app_file_profile,
									   _T("wav {*.wav}\0*.wav\0All files {*.*}\0*.*\0\0"),
									   _T("wav"     ),
									   _T("ptv-wav.path"),
									   _T("Save wav file"), _T("Load wav file"), NULL ) ) goto term;

			if( Japanese_Is() ){ title_save = _title_oput_txt_j; }
			else               { title_save = _title_oput_txt_e; }

			g_path_dlg_txt = new pxwPathDialog();
			if( !g_path_dlg_txt->init( _app_file_profile,
									   _T("text[*.txt}\0*.txt\0All files {*.*}\0*.*\0\0"),
									   _T("txt"     ),
									   _T("ptv-txt.path"),
									   title_save, title_load, NULL ) ) goto term;
		}

		g_alte = new pxwAlteration(); g_alte->set_window( g_hWnd_Main );
		g_mouse     = new pxMouse( g_hWnd_Main );
		g_interface = new Interface(); if( !g_interface->init() ) MessageBox( g_hWnd_Main, _T("interface error"), g_app_name, MB_OK );

		pxtoneTool_HearSelect_Initialize( g_strm_woi, _app_file_profile );

		{
			bool b_err = false;
			if( !g_dxdraw->tex_load( _T("img"), _T("parts.png"), SURF_PARTS ) ) b_err = true;
			if( !g_dxdraw->tex_create( SCREEN_WAVEMINI_W, SCREEN_WAVEMINI_H, 0x00000000, SURF_WAVE_mini_0 ) ) b_err = true;
			if( !g_dxdraw->tex_create( SCREEN_WAVEMINI_W, SCREEN_WAVEMINI_H, 0x00000000, SURF_WAVE_mini_1 ) ) b_err = true;
			if( !g_dxdraw->tex_create( SCREEN_WAVE_W    , SCREEN_WAVE_H    , 0x00000000, SURF_WAVE_SCREEN ) ) b_err = true;

			if( !g_dxdraw->tex_create( SCREEN_ENVEMINI_W, SCREEN_ENVEMINI_H, 0x00000000, SURF_ENVE_mini_0 ) ) b_err = true;
			if( !g_dxdraw->tex_create( SCREEN_ENVEMINI_W, SCREEN_ENVEMINI_H, 0x00000000, SURF_ENVE_mini_1 ) ) b_err = true;
			if( !g_dxdraw->tex_create( SCREEN_ENVE_W    , SCREEN_ENVE_H    , 0x00000000, SURF_ENVE_SCREEN ) ) b_err = true;
			if( b_err ){ mbox_c_ERR( NULL, "load img." ); goto term; }
			if_gen_init( g_dxdraw, SURF_PARTS );
		}

		// MIDI..
		MidiInput_Init();
		MidiInput_Reset( cfg.midi->b_velo, cfg.midi->key_tuning );
		g_midi_in->Open( cfg.midi->name, g_hWnd_Main, NULL );
		g_midi_in->Input_Start();

		// ドラッグプロジェクト
		if( args[0] ) _Function_DropProjectFile( &g_posted_path, args );

		while( GetTickCount() < count + (1000 * 0.5f) ){}
	}

	{
		RECT rc;
		GetClientRect(  g_hWnd_Main, &rc );
		InvalidateRect( g_hWnd_Main, &rc, false );
		UpdateWindow(   g_hWnd_Main );
	}

	MSG msg;
	//メッセージループを生成
	while( GetMessage( &msg, NULL, 0, 0 ) )
	{
		TranslateMessage( &msg );
		DispatchMessage ( &msg );
	}

term:

	ptvVelocity_save();

	if( g_strm_xa2 ){ if( g_strm_xa2->stream_finalize( 0.1f, 30, 3 ) ) SAFE_DELETE( g_strm_xa2 ); }

	SAFE_DELETE( g_interface    );
	SAFE_DELETE( g_mouse        );
	SAFE_DELETE( g_alte         );
	SAFE_DELETE( g_path_dlg_txt );
	SAFE_DELETE( g_path_dlg_wav );
	SAFE_DELETE( g_path_dlg_ptv );
	SAFE_DELETE( g_strm_woi     );
	SAFE_DELETE( g_freq         );
	SAFE_DELETE( g_pxtn         );
	SAFE_DELETE( g_midi_in      );
	SAFE_DELETE( g_vunit        );
	SAFE_DELETE( g_curkey       );
	SAFE_DELETE( g_dxdraw       );

	dlog_c( "pxtone Voice End ------" );

	JapaneseTable_Release();
	pxDebugLog_release();

	return 1;
}
