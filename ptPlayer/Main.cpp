
#include <pxDebugLog.h>
#include <pxStrT.h>

#include <pxwDx09Draw.h>
pxwDx09Draw *g_dxdraw = NULL;

#include <pxwFilePath.h>
#include <pxwWindowRect.h>
#include <pxwPathDialog.h>
//#include <pxwXAudio2Keep.h>

#include <pxFile2.h>
static pxFile2* _app_file_common  = NULL;
static pxFile2* _app_file_data    = NULL;
static pxFile2* _app_file_profile = NULL;
#include <pxPath.h>

static TCHAR* _posted_path = NULL;

#pragma comment(lib,"d3d9")
#pragma comment(lib,"d3dx9")

#include "../Generic/if_Generic.h"
#include "../Generic/MessageBox.h"

#include "../Generic/cls_ExistingWindow.h"
#include "../Generic/KeyControl.h"
#include "../Generic/OpenVolumeControl.h"
#include "../Generic/Menu_History.h"
#include "../Generic/Japanese.h"

#include <pxtonewinXA2.h>
pxtonewinXA2*  g_strm_xa2      = NULL;

#include <pxtnService.h>
pxtnService*   g_pxtn          = NULL;

#include <pxwPathDialog.h>
pxwPathDialog* g_path_dlg_tune = NULL;

#include "../ptConfig/ptConfig.h"
#include "../ptConfig/ptConfig_Build.h"

#include "interface/Interface.h"
#include "interface/if_Player.h"
#include "interface/SurfaceNo.h"

#include "./Tune.h"
#include "JapaneseTable.h"

#include "resource.h"
#include "Call_ptCollage.h"

#include "dialog/dlg_BuildProgress.h"

#define WM_USER_RELATEDFILE (WM_USER + 0)

#define _STREAM_DEFAULT_SPS    44100
#define _STREAM_DEFAULT_CH_NUM     2
#define _STREAM_DEFAULT_BUF_SEC 0.1f


#define _VIEW_WIDTH  160
#define _VIEW_HEIGHT  88

#define _FLIPWAIT     20

HINSTANCE   g_hInst;
HWND        g_hWnd_Main  = NULL;
HMENU       g_hMenu_Main = NULL;

TCHAR       g_dir_module[ MAX_PATH ] = {0};
TCHAR       g_app_name_t[       64 ] = {0};
char        g_app_name_c[       64 ] = {0};

static const TCHAR*   _rect_name         = _T("main.rect" );
static TCHAR*         _class_name        = _T("Main"      );
static int32_t        _mag               = 1;
static const TCHAR*   _build_config_name = _T("build.conf");
static pxwPathDialog* _path_dlg_build    = NULL;

static const TCHAR*   _title_save_wav_j = _T("WAVファイルに出力");
static const TCHAR*   _title_save_wav_e = _T("Output *.wav"     );


static const TCHAR* _app_name_t_jp = _T("ピストンプレイヤー");
static const TCHAR* _app_name_t_en = _T("pxtone Player"     );

static bool _bInterfaceActive = false;

bool GameMain( HWND hWnd );

INT_PTR CALLBACK
dlg_Config_Procedure( HWND hDlg, UINT msg, WPARAM w, LPARAM l );

INT_PTR CALLBACK
dlg_About           ( HWND hDlg, UINT msg, WPARAM w, LPARAM l );

INT_PTR CALLBACK
dlg_BuildOption_Procedure( HWND hDlg, UINT msg, WPARAM w, LPARAM l );

int32_t GetCompileVersion( int32_t *p1, int32_t *p2, int32_t *p3, int32_t *p4 );


//////////////////////////////////////////
// ローカル関数 //////////////////////////
//////////////////////////////////////////

static void _MinimizedWindow()
{
	_bInterfaceActive = false;
}

static void _RestoredWindow()
{
	_bInterfaceActive = true;
}

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
    wc.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE + 1);
    wc.hIcon         = LoadIcon( hInst, _T("0") );
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW    );

    if( !RegisterClassEx( &wc ) ) return false;

	return true;
}

// タイトルバー表記
void MainWindow_SetTitle( const TCHAR *path )
{
	TCHAR str[MAX_PATH];
	TCHAR *p_name;
	TCHAR *empty = _T("-");

	if( !path || path[0] == '\0' ) p_name = empty;
	else                           p_name = PathFindFileName( path );

#ifdef NDEBUG
	_stprintf_s( str, MAX_PATH, _T("[%s] %s "     ), p_name, g_app_name_t );
#else
	_stprintf_s( str, MAX_PATH, _T("[%s] %s Debug"), p_name, g_app_name_t );
#endif
	SetWindowText( g_hWnd_Main, str );
}


#define WM_USER_RELATEDFILE (WM_USER + 0)


// ファイルドロップ(ウインドウ)
static bool _GetDroppedPath_Window( HWND hWnd, WPARAM wParam, TCHAR *path_drop )
{
	HDROP hDrop = (HDROP)wParam;
	bool  b_ret = false;

	memset( path_drop, 0, MAX_PATH * sizeof(TCHAR) );

	if( DragQueryFile( hDrop, -1, NULL, 0 ) != 0 )
	{
		DragQueryFile( hDrop,  0, path_drop, MAX_PATH );
		b_ret = true;
	}

	DragFinish( hDrop );
	return b_ret;
}

// ファイルドロップ(アイコン)
static bool _GetDroppedPath_Start( TCHAR *lpszArgs, TCHAR *path_drop )
{
	int32_t a;

	memset( path_drop, 0, MAX_PATH * sizeof(TCHAR) );
	if( !_tcslen( lpszArgs ) ) return false;

	if( lpszArgs[0] == '"' )
	{
		for( a = 0; a < MAX_PATH-1; a++ )
		{
			if( lpszArgs[a+1] == '"' || lpszArgs[a+1] == '\0') break;
			path_drop[a] = lpszArgs[a+1];
		}
	}
	else
	{
		for( a = 0; a < MAX_PATH-1; a++ ){
			if(                         lpszArgs[a+0] == '\0') break;
			path_drop[a] = lpszArgs[a+0];
		}
	}
	path_drop[a] = '\0';

	return true;
}

bool InquireOperation()
{
	if( !g_strm_xa2->tune_is_sampling() ) return true;
	if_Player_StopPlay();

	for( int i = 0; i < 30; i++ )
	{
		Sleep( 100 );
		if( !g_strm_xa2->tune_is_sampling() ) return true;
	}
	MessageBox( g_hWnd_Main, _T("stopping timeout."), g_app_name_t, MB_ICONEXCLAMATION );
	return false;
}

static bool _Function_IDM_LOAD( HWND hWnd )
{
	Tune_SelectAndPlay( g_hWnd_Main );
	KeyControl_Clear();
	return true;
}

static bool _Function_IDM_HISTORY( HWND hwnd, UINT idm )
{
	TCHAR path[ MAX_PATH ];
	if( !Menu_History_GetPath( idm , path ) ) return false;
	if( !Tune_LoadAndPlay     ( hwnd, path ) ){ Menu_History_Delete( idm ); return false; }
	return true;
}

static bool _Function_IDM_CONFIG( HWND hwnd )
{
	static ptConfig _cfg( _app_file_profile, _STREAM_DEFAULT_SPS, _STREAM_DEFAULT_CH_NUM, _STREAM_DEFAULT_BUF_SEC );

	InquireOperation();

	_cfg.load();

	if( !DialogBoxParam( g_hInst, _T("DLG_CONFIG"), hwnd, dlg_Config_Procedure, (LPARAM)&_cfg ) ) return true;

	if( !_cfg.save() )
	{
		Japanese_MessageBox( hwnd, _T("Save Config"), _T("Error"), MB_OK|MB_ICONEXCLAMATION );
		return false;
	}

	if( !g_strm_xa2->stream_finalize( 0.1f, 10, 3 ) )
	{
		MessageBox( hwnd, _T("stream stop timeout."), _T("error"), MB_OK|MB_ICONEXCLAMATION );
		return false;
	}

	if_Player_ZeroSampleOffset();

	if( !g_strm_xa2->stream_start( _cfg.strm->ch_num, _cfg.strm->sps, _cfg.strm->buf_sec ) )
	{
		MessageBox( hwnd, _T("stream start."), _T("error"), MB_OK|MB_ICONEXCLAMATION );
		return false;
	}

	g_pxtn->set_sampled_callback( if_Player_Callback_Sampled, g_hWnd_Main );

	// font
	if( !g_dxdraw->d3d_font_init( _cfg.font->name, 12 ) )
	{
		MessageBox( hwnd, _T("err: font init."), g_app_name_t, MB_OK|MB_ICONEXCLAMATION );
		return false;
	}
	if_Player_RedrawName( _cfg.font->name );
	return true;
}

// build..
static void _Function_IDM_BUILD( HWND hwnd )
{
	static BUILDPROGRESSSTRUCT _build;
	static ptConfig_Build _bld( _STREAM_DEFAULT_SPS, _STREAM_DEFAULT_CH_NUM );
	bool                  b_ret = false;
	pxDescriptor* desc = NULL;

	if( !Tune_is_valid_data() ) return;

	InquireOperation();

	if( _app_file_profile->open_r( &desc, _build_config_name, NULL, NULL ) ){ _bld.read( desc ); SAFE_DELETE( desc ); }

	if( !DialogBoxParam( g_hInst, _T("DLG_BUILDOPTION"), hwnd, dlg_BuildOption_Procedure, (LPARAM)&_bld ) ) return;

	if( !_app_file_profile->open_w( &desc, _build_config_name, NULL, NULL ) ||
		!_bld.write( desc ) )
	{
		SAFE_DELETE( desc );
		MessageBox( hwnd, _T("save build-config"), _T("Error"), MB_ICONERROR );
		return;
	}
	SAFE_DELETE( desc );

	_build.sec_extrafade = _bld.sec_extrafade;
	_build.sec_playtime  = _bld.sec_playtime ;
	_build.volume        = _bld.volume       ;

	switch( _bld.scope_mode )
	{
	case BUILDSCOPEMODE_TOPLAST: _build.scope = BUILDTUNESCOPE_TOPLAST; break;
	case BUILDSCOPEMODE_BYTIME : _build.scope = BUILDTUNESCOPE_BYTIME ; break;
	}

	_build.ver = GetCompileVersion( 0, 0, 0, 0 );

	// make file-name( .wav )
	{
		TCHAR name[ MAX_PATH ] = {0};
		TCHAR exte[ 32       ] = {0};
		if( !g_path_dlg_tune ->last_filename_get( name ) ) return;
		if( ! _path_dlg_build->extension_get    ( exte ) ) return;
		PathRemoveExtension( name );
		_tcscat( name, _T(".") );
		_tcscat( name, exte    );
		_path_dlg_build->last_filename_set( name );
	}

	{
		TCHAR path_tune[ MAX_PATH ] = {0};
		if( !g_path_dlg_tune->get_last_path( path_tune, MAX_PATH ) ) return;
		PathRemoveExtension( path_tune );
		if( !_path_dlg_build->dialog_save( hwnd, _build.output_path, PathFindFileName( path_tune ) ) ) return;
	}

	g_pxtn->set_destination_quality(  _bld.strm->ch_num, _bld.strm->sps );
	if( g_pxtn->tones_ready() != pxtnOK )
	{
		MessageBox( hwnd, _T("ready tones."), _T("error"), MB_OK|MB_ICONEXCLAMATION );
		return;
	}

	{
		WINDOWPLACEMENT place;
		place.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement( hwnd, &place );
		ShowWindow( hwnd, SW_HIDE );
		DialogBoxParam( g_hInst, _T("DLG_BUILDPROGRESS"), hwnd, dlg_BuildProgress, (LPARAM)&_build );
		ShowWindow( hwnd, place.showCmd );
	}

	ptConfig cfg_ptp( _app_file_profile, _STREAM_DEFAULT_SPS, _STREAM_DEFAULT_CH_NUM, _STREAM_DEFAULT_BUF_SEC );
	cfg_ptp.load();
	g_pxtn->set_destination_quality(  cfg_ptp.strm->ch_num, cfg_ptp.strm->sps );

	return;
}


static LRESULT CALLBACK _WindowProc( HWND hwnd, UINT msg, WPARAM w, LPARAM l )
{
	switch( msg )
	{
	case WM_CREATE:

		g_hWnd_Main       = hwnd;
		_bInterfaceActive = true;

#ifdef NDEBUG
		{
			HMENU hMenu;
			hMenu = GetMenu( hwnd );
			DeleteMenu( hMenu, IDM_TEST_FADEOUT    , MF_BYCOMMAND );
			DeleteMenu( hMenu, IDM_TEST_TUNERELEASE, MF_BYCOMMAND );
			DrawMenuBar( hwnd );
		}
#endif

		if( !Find_ptCollage() )
		{
			DeleteMenu ( GetMenu( hwnd ), IDM_PTCOLLAGE, MF_BYCOMMAND );
			DrawMenuBar( hwnd );
		}

		UpdateWindow( hwnd );
		MainWindow_SetTitle( NULL );
		DragAcceptFiles( hwnd, true );

		break;


	case WM_CLOSE:

		Menu_History_Save();
		PostQuitMessage( 0 );
		break;

	case WM_DESTROY:
		break;

	case WM_PAINT:
		{
			HDC         hdc;
			PAINTSTRUCT ps ;
			hdc = BeginPaint ( hwnd, &ps  );
			Interface_Process( hwnd, true );
			EndPaint         ( hwnd, &ps  );
		}
		break;

	case WM_ACTIVATE:
		switch( LOWORD( w ) )
		{
		case WA_ACTIVE     :
		case WA_CLICKACTIVE:

			{
				WINDOWPLACEMENT place = {0};
				place.length = sizeof(WINDOWPLACEMENT);

				if( !GetWindowPlacement( hwnd, &place ) ) break;
				if( place.showCmd != SW_SHOWMINIMIZED ) Interface_Process( g_hWnd_Main, true );
			}

			_bInterfaceActive = true;
			KeyControl_Clear();
			break;

		default:
			_bInterfaceActive = false;
			break;
		}
		break;

	//コマンドメッセージ ========
	case WM_COMMAND:

		switch( LOWORD( w ) )
		{
		// メインウインドウを閉じさせる
		case IDM_LOAD            : _Function_IDM_LOAD  ( hwnd );          break;
		case IDM_CLOSE           : SendMessage         ( hwnd, WM_CLOSE, 0, 0 ); break;
		case IDM_CONFIG          : _Function_IDM_CONFIG( hwnd );        break;
		case IDM_ABOUT           : DialogBox( g_hInst, _T("DLG_ABOUT"), hwnd, dlg_About ); break;
		case IDM_VOLUMEDIALOG    : OpenVolumeControl   ( hwnd       );  break;
		case IDM_EXPORTWAV       : _Function_IDM_BUILD(  hwnd );  break;
		case IDM_PTCOLLAGE       :
			{
				static TCHAR path[ MAX_PATH ] = {0};
				memset( path, 0, sizeof(path) );
				if( !g_path_dlg_tune->get_last_path( path, MAX_PATH ) ) break;
				if_Player_StopPlay();
				Call_ptCollage( hwnd, path );
			}
			break;
		case IDM_TEST_FADEOUT    :
			g_strm_xa2->tune_order_stop( 1.0f );  break;

		case IDM_HISTORY_0:
		case IDM_HISTORY_1:
		case IDM_HISTORY_2:
		case IDM_HISTORY_3:
		case IDM_HISTORY_4:
		case IDM_HISTORY_5:
		case IDM_HISTORY_6:
		case IDM_HISTORY_7:
		case IDM_HISTORY_8:
		case IDM_HISTORY_9:
			_Function_IDM_HISTORY( hwnd, LOWORD( w ) );
			break;
		default:
			break;
		}
		break;

	// 起動後ファイルドロップ
	case WM_DROPFILES:
		{
			TCHAR path[ MAX_PATH ] = {0};
			if( _GetDroppedPath_Window( hwnd, w, path ) ) Tune_LoadAndPlay( hwnd, path );
			Interface_Process( hwnd, true );
		}
		break;

	// 起動後関連ファイル
	case WM_USER_RELATEDFILE:
		{
			TCHAR path[ MAX_PATH ] = {0};
			if( _posted_path ) Tune_LoadAndPlay( hwnd, _posted_path );
			Interface_Process( hwnd, true );
		}
		break;

	case WM_MOVE:
		pxwWindowRect_save( hwnd, _rect_name );
		break;

	//サイズ変更
	case WM_SIZE:

		switch( w )
		{
		case SIZE_MINIMIZED:
			_MinimizedWindow();
			break;
		case SIZE_MAXIMIZED:
		case SIZE_RESTORED:
			{
				WINDOWPLACEMENT place;
				place.length = sizeof(WINDOWPLACEMENT);
				if( !GetWindowPlacement( hwnd, &place ) ) return false;

				if( g_dxdraw ) g_dxdraw->WindowMode_mag( hwnd, 0, LOWORD(l), HIWORD(l) );

				RECT rc;
				GetClientRect(  hwnd, &rc );
				InvalidateRect( hwnd, &rc, false );
				UpdateWindow(   hwnd );
				_RestoredWindow();
			}

			break;
		}

		break;

	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		KeyControl_WM_MESSAGE( hwnd, msg, w );
		Interface_Process(     hwnd, false  );
		break;

	case WM_MOUSEMOVE:
		//if( !pxtone_Tune_IsStreaming() )
		Interface_Process( hwnd, false );
		break;

	case WM_MOUSEWHEEL:
		KeyControl_WM_MOUSEWHEEL( hwnd, msg, w );
		Interface_Process(        hwnd, true   );
		break;

	// minimum size
	case WM_GETMINMAXINFO:
		{
			RECT rc_work;
			SystemParametersInfo( SPI_GETWORKAREA, 0, &rc_work, 0 );

			MINMAXINFO* pmm = (MINMAXINFO*)l;
			pmm->ptMinTrackSize.x = _VIEW_WIDTH  * pxwDx09Draw_system_mag(); // min w
			pmm->ptMinTrackSize.y = _VIEW_HEIGHT * pxwDx09Draw_system_mag(); // min h
			pmm->ptMaxTrackSize.x = rc_work.right ; // max w
			pmm->ptMaxTrackSize.y = rc_work.bottom; // max h
		}
		break;

	default://上記スイッチ記述のないものはWindowsに処理を委ねる
		return DefWindowProc( hwnd, msg, w, l );
	}

    return 0;
}

bool _SystemTask()
{
	MSG msg;

	while( PeekMessage(   &msg, NULL, 0, 0, PM_NOREMOVE) || !_bInterfaceActive )
	{
		if( !GetMessage(  &msg, NULL, 0, 0 ) ) return false;
		TranslateMessage( &msg );
		DispatchMessage(  &msg );
	}
	return true;
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

















/////////////////////////
// WinMain //////////////
/////////////////////////


#include <pxwEntryPoint.h>
#include <pxwRuntime.h>

pxwENTRY_POINT( hInst, hPrevInst, lpszArgs, nWinMode )
{
    pxwRuntime runtime;
	{ int rc; if( !runtime.init( _app_name_t_en, &rc ) ) return rc; }

	cls_EXISTINGWINDOW existing_window;
	static TCHAR* mutex_name   = _T("ptplayer"    );
	static TCHAR* mapping_name = _T("map_ptplayer");
	TCHAR path_drop[ MAX_PATH ];

	g_hInst = hInst;

	// メインスレッドの優先度を上げる
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );

	// 汎用のパスを取得-------------------------------------
	pxwFilePath_GetModuleDirectory(  g_dir_module );


	_app_file_common  = new pxFile2(); _app_file_common ->init_base( _T("data_common"), false );
	_app_file_data    = new pxFile2(); _app_file_data   ->init_base( _T("data_ptp"   ), false );
	_app_file_profile = new pxFile2(); _app_file_profile->init_base( _T("temp_ptp"   ), true  );
	pxPath_setMode( pxPathMode_auto );

	pxwWindowRect_init( _app_file_profile );

	dlog_c( "version %d.", GetCompileVersion( 0, 0, 0, 0 ) );

	// japanese..
	{
		TCHAR path[ MAX_PATH ] = {0}; _stprintf_s( path, MAX_PATH, _T("%s\\%s"), g_dir_module, _T("japanese.ico") );
		FILE* fp = _tfopen( path, _T("rb") );
		if( fp ){ fclose( fp ); JapaneseTable_init( true  ); }
		else    {               JapaneseTable_init( false ); }
	}

	if( _GetDroppedPath_Start( lpszArgs, path_drop ) ) pxStrT_copy_allocate( &_posted_path, path_drop );

	// 既に起動しているアプリを調べる
	if( !existing_window.Check( mutex_name, mapping_name, WM_USER_RELATEDFILE ) ) return 0;

	KeyControl_Clear();

	// 日本語モード
	// japanese..
	{
		TCHAR path[ MAX_PATH ] = {0}; _stprintf_s( path, MAX_PATH, _T("%s\\%s"), g_dir_module, _T("japanese.ico") );
		FILE* fp = _tfopen( path, _T("rb") );
		if( fp ){ fclose( fp ); JapaneseTable_init( true  ); }
		else    {               JapaneseTable_init( false ); }
	}
	if( Japanese_Is() ){ _tcscpy( g_app_name_t, _app_name_t_jp ); strcpy( g_app_name_c, "ピストンプレイヤー" ); }
	else               { _tcscpy( g_app_name_t, _app_name_t_en ); strcpy( g_app_name_c, "pxtone Player"      ); }

	_path_dlg_build = new pxwPathDialog();

	if( !_path_dlg_build->init( _app_file_profile,
		_T("wav {*.wav}\0*.wav*\0") _T("All Files {*.*}\0*.*\0\0"),
		_T("wav"), _T("ptp-build.path"), Japanese_Is() ? _title_save_wav_j : _title_save_wav_e,
		NULL, NULL ) )
	{
		MessageBox( NULL, _T("RESOURCE ERROR"), _app_name_t_en, MB_OK|MB_ICONERROR );
		goto term;
	}

	// ウインドウクラスを定義
	if( !_RegistWindowClass( hInst, _class_name, _WindowProc ) ) return false;

    g_hMenu_Main   = LoadMenu( hInst, _T("MENU_MAIN") );
	Japanese_MenuItem_Change( g_hMenu_Main );

	{
		RECT rc = {0, 0, _VIEW_WIDTH * pxwDx09Draw_system_mag(), _VIEW_HEIGHT * pxwDx09Draw_system_mag()};
		int32_t  w, h;

		DWORD cs = WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;

		AdjustWindowRectEx( &rc, cs, true, 0 );

		w = abs(rc.left) + rc.right;
		h = abs(rc.top ) + rc.bottom;

		dlog_c( "adh size( %d, %d )", w, h );

		g_hWnd_Main  = CreateWindow(
			_class_name,
			g_app_name_t,
			WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			w, h,
			HWND_DESKTOP,
			g_hMenu_Main,
			hInst,
			NULL );
	}

	if( !existing_window.Mapping( mutex_name, mapping_name, g_hWnd_Main ) ) return 0;

	{
		UINT table[ 10 ] =
		{
			IDM_HISTORY_0, IDM_HISTORY_1, IDM_HISTORY_2,
			IDM_HISTORY_3, IDM_HISTORY_4, IDM_HISTORY_5,
			IDM_HISTORY_6, IDM_HISTORY_7, IDM_HISTORY_8,
			IDM_HISTORY_9,
		};

		Menu_History_init( GetSubMenu( GetSubMenu( g_hMenu_Main, 0 ), 1 ), 10, table, IDM_HISTORY_D, _app_file_profile );
		Menu_History_Load();
	}

	pxwWindowRect_load( g_hWnd_Main, _rect_name, false, true );

	g_dxdraw   = new pxwDx09Draw( _app_file_data );
	if( !g_dxdraw->init( g_hWnd_Main ) ){ mbox_c_ERR( NULL, "dxdraw."      ); return false; }

	g_pxtn     = new pxtnService( _io_read, _io_write, _io_seek, _io_pos );
	if(  g_pxtn->init() != pxtnOK      ){ mbox_c_ERR( NULL, "pxtone init." ); return false; }

	g_strm_xa2 = new pxtonewinXA2();
	if( !g_strm_xa2->init( g_pxtn, pxtnMAX_STREAMINGVOICE, pxtnMAX_STREAMINGVOICE ) )
	{
		mbox_c_ERR( NULL, "pxtone streaming init." ); return false;
	}

	g_path_dlg_tune = new pxwPathDialog();
	if( !g_path_dlg_tune->init( _app_file_profile,
		_T("pttune {*.pttune;*.ptcop}\0*.pttune;*.ptcop*\0") _T("All files {*.*}\0*.*\0\0"),
		_T("*"), _T("ptp-tune.path"), _T("Save File"), _T("Load File"), NULL ) ) goto term;

	{// LOADING..表示

		DWORD    count  ;
		ptConfig cfg_ptp( _app_file_profile, _STREAM_DEFAULT_SPS, _STREAM_DEFAULT_CH_NUM, _STREAM_DEFAULT_BUF_SEC );

		cfg_ptp.load();

		count = GetTickCount();

		if( !if_gen_splash( g_hWnd_Main, g_dxdraw, SURF_LOADING, 1 ) ){ mbox_c_ERR( NULL, "error: splash." ); return false; }

		{
			if( g_dxdraw->tex_load( _T("img"), _T("tenkey.png"), SURF_TENKEY ) != SURF_TENKEY ){ mbox_c_ERR( NULL, "img:tenkey" ); return false; }
			if( g_dxdraw->tex_load( _T("img"), _T("player.png"), SURF_PLAYER ) != SURF_PLAYER ){ mbox_c_ERR( NULL, "img:player" ); return false; }

			pxGLYPH_PARAM1 prm; memset( &prm, 0, sizeof(prm) );
			_tcscpy( prm.font_name, _T("MS Gothic") );
			prm.font_h      = 12;
			prm.font_argb   = 0xff80f000;
			prm.type        = pxGLYPH_mono;
			prm.b_font_bold = false;
			prm.grid_x      = 32;
			prm.grid_y      = 32;

			if(  g_dxdraw->tex_create( TUNENAME_WIDTH, TUNENAME_HEIGHT, 0xff00f80, SURF_TUNENAME ) != SURF_TUNENAME ){ mbox_c_ERR( NULL, "tex:name" ); return false; }
			if( !g_dxdraw->tex_glyph_init( SURF_TUNENAME, &prm, 128, false ) ){ mbox_c_ERR( NULL, "glyph:ms gothic" ); return false; }

			if_gen_init( g_dxdraw, SURF_TENKEY );
		}

		Interface_init( g_hWnd_Main, NULL, _app_file_profile );

		// pxtone
		g_pxtn->set_sampled_callback( if_Player_Callback_Sampled, g_hWnd_Main );

        if( !g_strm_xa2->stream_start( cfg_ptp.strm->ch_num, cfg_ptp.strm->sps, cfg_ptp.strm->buf_sec ) )
        {
            Japanese_MessageBox( g_hWnd_Main, _T("ready pxtone"), _app_name_t_en, MB_OK|MB_ICONEXCLAMATION );
            return false;
        }

		if_Player_RedrawName( cfg_ptp.font->name );

		// ドラッグプロジェクト
		if( _tcslen( path_drop ) ) Tune_LoadAndPlay( g_hWnd_Main, path_drop );

		while( GetTickCount() < count + (1000 * 0.5f) ){}
	}

	{
		RECT rc;
		GetClientRect ( g_hWnd_Main, &rc );
		InvalidateRect( g_hWnd_Main, &rc, false );
		UpdateWindow  ( g_hWnd_Main );
	}

//	Tune_Initialize();

//	MessageBox( g_hWnd_Main, "これは動作確認バージョンです。\r\n"
//		"過去のデータが正常に再生できない場合は連絡ください\r\n\r\n"
//		"This is test version.\r\nPlease tell me that if this can't play your data normally.",
//		"pxPlayer", MB_OK|MB_ICONINFORMATION );


	MSG msg;
	//メッセージループを生成
	while( GetMessage( &msg, NULL, 0, 0 ) )
	{
		TranslateMessage( &msg );//キーボード使用可能
		DispatchMessage(  &msg );//制御をWindowsに戻す
	}

term:

	if( g_strm_xa2 )
	{
		g_strm_xa2->stream_finalize( 0.1f, 30, 3.0f );
		SAFE_DELETE( g_strm_xa2 );
	}
	SAFE_DELETE( g_pxtn );

	Interface_Release();

	SAFE_DELETE( g_dxdraw );
	DestroyWindow( g_hWnd_Main );

	SAFE_DELETE( _path_dlg_build );
	SAFE_DELETE( g_path_dlg_tune );

	dlog_c( "ptPlayer Exit" );
	Menu_History_Release();

	pxDebugLog_release();

	return 1;
}
