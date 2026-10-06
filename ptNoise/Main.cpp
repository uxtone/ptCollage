
#pragma comment(lib, "d3d9"   )
#pragma comment(lib, "d3dx9"  )

#include <pxDebugLog.h>



#include <pxFile2.h>
static pxFile2* _app_file_common  = NULL;
static pxFile2* _app_file_data    = NULL;
static pxFile2* _app_file_profile = NULL;
#include <pxPath.h>

#include <pxtnPulse_NoiseBuilder.h>
pxtnPulse_NoiseBuilder* g_noise_bldr   = NULL;

#include <pxtnService.h>
pxtnService*            g_pxtn         = NULL;

#include <pxtonewinXA2.h>
pxtonewinXA2*           g_strm_xa2     = NULL;

#include <pxtonewinWoice.h>
pxtonewinWoice*         g_strm_woi     = NULL;

#include <pxwDx09Draw.h>
pxwDx09Draw*            g_dxdraw       = NULL;

#include <pxwPathDialog.h>
pxwPathDialog*          g_path_dlg_ptn = NULL;
pxwPathDialog*          g_path_dlg_wav = NULL;

static const uxSS<24>  _title_save_ptn_j  = "ptnoise を保存する";
static const uxSS<13>  _title_save_ptn_e  = "Save ptnoise";
static const uxSS<24>  _title_load_ptn_j  = "ptnoise を読み込む";
static const uxSS<14>  _title_load_ptn_e  = "Load ptnoisee";
static const uxSS<25>  _title_save_wav_j  = "WAV形式で保存する";
static const uxSS<9>  _title_save_wav_e  = "Save wav";
static const uxSS<25>  _title_load_wav_j  = "WAV形式を読み込む";
static const uxSS<9>  _title_load_wav_e  = "Load wav";

static const uxSS<22>  _app_name_t_jp = "ピストンノイズ";
static const uxSS<13>  _app_name_t_en = "pxtone Noise";


#include <pxwWindowRect.h>
#include <pxwFilePath.h>
#include <pxwXAudio2Keep.h>

#include "../Generic/MessageBox.h"
#include "../Generic/cls_ExistingWindow.h"
#include "../Generic/KeyControl.h"
#include "../Generic/Japanese.h"
#include "../Generic/if_Generic.h"

#include "../ptConfig/ptConfig.h"

#include "../pxtoneTool/pxtoneTool.h"


#include "./ptNoise.h"
#include "./interface/Interface.h"
#include "./interface/SurfaceNo.h"
#include "./JapaneseTable.h"
#include "./PcmTable.h"
#include "./resource.h"


HINSTANCE    g_hInst;
HWND         g_hWnd_Main    = NULL;
HWND         g_hDlg_Design  = NULL;
HWND         g_hDlg_History = NULL;
HMENU        g_hMenu_Main   = NULL;

uxDS         g_dir_module;
uxSS<32>         g_app_name = {0};

uxDS         g_main_rect_name          = "main.rect";
uxDS         g_default_material_folder = "my_material";
const uxDS   g_strm_config_name        = "strm.cfg";

static uxSS<5> _class_name       = "Main";
static int32_t   _mag            =          1;

int32_t          g_MinimizeWidth;
int32_t          g_MinimizeHeight;


LRESULT CALLBACK WindowProc_Main( HWND hWnd, UINT msg, WPARAM w, LPARAM l );

INT_PTR CALLBACK
dlg_NoiseDesign_Design(  HWND hDlg, UINT msg, WPARAM w, LPARAM l );

void dlg_NoiseDesign_init( const pxFile2* file_profile );

INT_PTR CALLBACK
dlg_History( HWND hDlg, UINT msg, WPARAM w, LPARAM l );


void MainProc_set_file_profile( const pxFile2* file_profile );


//////////////////////////////////////////
// ローカル関数 //////////////////////////
//////////////////////////////////////////

// ウインドウクラスの登録
static bool _RegistWindowClass( HINSTANCE hInst, const uxDS& class_name, WNDPROC lpfnWndProc )
{
	WNDCLASSEX wc;

	memset( &wc, 0, sizeof(WNDCLASSEX) );

	wc.cbSize        = sizeof(WNDCLASSEX);
	uxT t_class_name( class_name );              // OS text, valid for this function (RegisterClassEx copies the name)
	wc.lpszClassName = t_class_name;
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = lpfnWndProc;
	wc.hInstance     = hInst;        //インスタンス
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);//(HBRUSH)(COLOR_APPWORKSPACE + 1);
	wc.hIcon         = LoadIcon( hInst, uxT( "0" ) );
	wc.hCursor       = LoadCursor( NULL, IDC_ARROW       );

	if( !RegisterClassEx( &wc ) ) return false;

	return true;
}

static bool _Function_DropProjectFile( HWND hWnd, const uxDS& lpszArgs )
{
	uxDS path;
	return pxwFilePath_ArgToPath( lpszArgs, path ); // (the path was never used here)
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

#include <pxwEntryPoint.h>
#include <pxwRuntime.h>

pxwENTRY_POINT( hInst, hPrevInst, args, nWinMode )
{
	pxwRuntime runtime;
	{ int rc; if( !runtime.init( _app_name_t_en, &rc ) ) return rc; }

	cls_EXISTINGWINDOW existing_window;
	static uxSS<12> mutex_name   = "pxtonenoise";
	static uxSS<16> mapping_name = "map_pxtonenoise";

	g_hInst = hInst;

	if( !existing_window.Check( mutex_name, mapping_name, NULL ) ) return 0;

	if( FAILED( CoInitializeEx( NULL, COINIT_MULTITHREADED ) ) ) return 0;

	KeyControl_Clear();

	// 汎用のパスを取得-------------------------------------
	pxwFilePath_GetModuleDirectory( g_dir_module );

	_app_file_common  = new pxFile2(); _app_file_common ->init_base( "data_common", false );
	_app_file_data    = new pxFile2(); _app_file_data   ->init_base( "data_ptn", false );
	_app_file_profile = new pxFile2(); _app_file_profile->init_base( "temp_ptn", true  );
	pxPath_setMode( pxPathMode_auto );

	MainProc_set_file_profile( _app_file_profile );
	dlg_NoiseDesign_init     ( _app_file_profile );
	pxwWindowRect_init       ( _app_file_profile );

	if( !pxDebugLog_init( "temp_ptn", "debuglog" ) ) goto term;

	// japanese..
	{
		uxSS<MAX_PATH> path = {0}; ux_sprintf_s( path, MAX_PATH, "%s\\%s", *g_dir_module, "japanese.ico" );
		FILE* fp = ux_fopen( path, "rb" );
		if( fp ){ fclose( fp ); JapaneseTable_init( true  ); }
		else    {               JapaneseTable_init( false ); }
	}

	if( Japanese_Is() ) strcpy( g_app_name, _app_name_t_jp );
	else                strcpy( g_app_name, _app_name_t_en );

	// window class..
	if( !_RegistWindowClass( hInst, _class_name, WindowProc_Main ) ) goto term;

	g_hMenu_Main = LoadMenu( hInst, uxT( "MENU_MAIN" ) );

	Japanese_MenuItem_Change( g_hMenu_Main );

	{
		DWORD style_flags = WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_CLIPCHILDREN;

		RECT rc = { 0, 0, VIEWDEFAULT_W * pxwDx09Draw_system_mag(), VIEWDEFAULT_H * pxwDx09Draw_system_mag() };

		AdjustWindowRectEx( &rc, style_flags, true, 0 );

		g_MinimizeWidth  = abs(rc.left) + rc.right ;
		g_MinimizeHeight = abs(rc.top ) + rc.bottom;

		g_hWnd_Main  = CreateWindow(
			uxT( _class_name ),
			uxT( g_app_name ) ,
			style_flags,
			16, 16,
			g_MinimizeWidth, g_MinimizeHeight,
			HWND_DESKTOP,
			g_hMenu_Main,
			hInst,
			NULL );
	}

	if( !existing_window.Mapping( mutex_name, mapping_name, g_hWnd_Main ) ) goto term;

	pxwWindowRect_load( g_hWnd_Main, g_main_rect_name, true, true );

	g_dxdraw = new pxwDx09Draw( _app_file_data );

	if( !g_dxdraw->init( g_hWnd_Main ) ){ mbox_c_ERR( NULL, "err:dx-init" ); goto term; }

	// LOADING..
	{
		DWORD count;

		count = GetTickCount();

		if( !if_gen_splash( g_hWnd_Main, g_dxdraw, SURF_LOADING, 2 ) ){ mbox_c_ERR( NULL, "error: splash." ); goto term; }

		g_noise_bldr = new pxtnPulse_NoiseBuilder( _io_read, _io_write, _io_seek, _io_pos ); if( !g_noise_bldr->Init() )
		{
			Japanese_MessageBox( NULL, "pxPulse Noise Init", "error", MB_OK|MB_ICONEXCLAMATION );
			goto term;
		}
		g_pxtn       = new pxtnService( _io_read, _io_write, _io_seek, _io_pos );
		if( g_pxtn->init() != pxtnOK )
		{
			Japanese_MessageBox( NULL, "pxtn Init", "error", MB_OK|MB_ICONEXCLAMATION );
			goto term;
		}

		ptConfig cfg( _app_file_profile, ptnDEFAULT_SPS, ptnDEFAULT_CH_NUM, ptnDEFAULT_BUF_SEC );
		cfg.load();

		g_strm_xa2 = new pxtonewinXA2();
		if( !g_strm_xa2->init( g_pxtn, pxtnMAX_STREAMINGVOICE, pxtnMAX_STREAMINGVOICE ) )
		{
			Japanese_MessageBox( NULL, "Streaming Init", "error", MB_OK|MB_ICONEXCLAMATION );
			goto term;
		}
		if( !g_strm_xa2->stream_start( cfg.strm->ch_num, cfg.strm->sps, cfg.strm->buf_sec ) )
		{
			Japanese_MessageBox( NULL, "Streaming Init", "error", MB_OK|MB_ICONEXCLAMATION );
			goto term;
		}

		g_strm_woi = new pxtonewinWoice();
		if( !g_strm_woi->init( g_strm_xa2 ) )
		{
			Japanese_MessageBox( NULL, "pxtone tool lib", "error", MB_OK|MB_ICONEXCLAMATION );
			goto term;
		}


		{
			const char* title_save = NULL;
			const char* title_load = NULL;

			if( Japanese_Is() ){ title_save = _title_save_ptn_j; title_load = _title_load_ptn_j; }
			else               { title_save = _title_save_ptn_e; title_load = _title_load_ptn_e; }

			g_path_dlg_ptn = new pxwPathDialog();
			if( !g_path_dlg_ptn->init( _app_file_profile,
									   "ptnoise {*.ptnoise}\0*.ptnoise*\0All files {*.*}\0*.*\0\0",
									   "ptnoise",
									   "ptn-ptn.path",
									   title_save, title_load, NULL ) ) goto term;

			if( Japanese_Is() ){ title_save = _title_save_wav_j; title_load = _title_load_wav_j; }
			else               { title_save = _title_save_wav_e; title_load = _title_load_wav_e; }

			g_path_dlg_wav = new pxwPathDialog();
			if( !g_path_dlg_wav->init( _app_file_profile,
									   "wav {*.wav}\0*.wav*\0" "All files {*.*}\0*.*\0\0",
									   "wav",
									   "ptn-wav.path",
									   title_save, title_load, NULL ) ) goto term;
		}


		pxtoneTool_HearSelect_Initialize( g_strm_woi, _app_file_profile );

		{
			bool b_err = false;
			if( !g_dxdraw->tex_load( "img", "parts.png", SURF_PARTS ) ) b_err = true;
			if( b_err ){ mbox_c_ERR( NULL, "load img." ); goto term; }
			if_gen_init( g_dxdraw, SURF_PARTS );
		}

		PcmTable_Initialize();
		Interface_init( g_hWnd_Main, _app_file_profile );

		// ドラッグプロジェクト
		if( args[0] ) _Function_DropProjectFile( g_hWnd_Main, args );

		while( GetTickCount() < count + (1000 * 0.5f) ){}
	}

	{
		RECT rc;
		GetClientRect ( g_hWnd_Main, &rc );
		InvalidateRect( g_hWnd_Main, &rc, false );
		UpdateWindow  ( g_hWnd_Main );

		g_hDlg_Design  = CreateDialog( g_hInst, uxT( "DLG_NOISE_DESIGN" ), g_hWnd_Main, dlg_NoiseDesign_Design );
		g_hDlg_History = CreateDialog( g_hInst, uxT( "DLG_HISTORY" ), g_hWnd_Main, dlg_History            );
	}

	MSG msg;
	//メッセージループを生成
	while( GetMessage( &msg, NULL, 0, 0 ) )
	{
		if( !IsDialogMessage( g_hDlg_Design , &msg ) &&
			!IsDialogMessage( g_hDlg_History, &msg ) )
		{
			TranslateMessage( &msg );//キーボード使用可能
			DispatchMessage(  &msg );//制御をWindowsに戻す
		}
	}

term:

	if( g_strm_xa2 ){ if( g_strm_xa2->stream_finalize( 0.1f, 30, 3 ) ) SAFE_DELETE( g_strm_xa2 ); }

	SAFE_DELETE( g_path_dlg_wav );
	SAFE_DELETE( g_path_dlg_ptn );
	SAFE_DELETE( g_strm_woi     );
	SAFE_DELETE( g_pxtn         );
	SAFE_DELETE( g_noise_bldr   );
	SAFE_DELETE( g_dxdraw       );

	PcmTable_Release     ();

	JapaneseTable_Release();

	pxDebugLog_release   ();

	return 1;
}
