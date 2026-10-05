
#include <pxwDx09Draw.h>
extern pxwDx09Draw*      g_dxdraw;

#include <pxtnService.h>
extern pxtnService*      g_pxtn;

#include <pxtonewinXA2.h>
extern pxtonewinXA2*     g_strm_xa2;

#include <pxtonewinWoice.h>
extern pxtonewinWoice*   g_strm_woi;

#include <pxwPathDialog.h>
extern pxwPathDialog*    g_path_dlg_ptn;
extern pxwPathDialog*    g_path_dlg_wav;

#include <pxwWindowRect.h>


#include "../Generic/KeyControl.h"
#include "../Generic/OpenVolumeControl.h"
#include "../Generic/Japanese.h"
#include "../ptConfig/ptConfig_Stream.h"

#include "../pxtoneTool/pxtoneTool.h"
#include "../ptConfig/ptConfig.h"

#include "./resource.h"

#include "./interface/Interface.h"

#include "./PcmTable.h"

#include "./ptNoise.h"

extern HINSTANCE g_hInst       ;
extern HWND      g_hWnd_Main   ;
extern TCHAR     g_app_name  [];
extern TCHAR     g_dir_temp  [];
extern TCHAR     g_dir_module[];
extern TCHAR*    g_main_rect_name;
extern TCHAR*    g_default_material_folder;

extern const char* g_strm_config_name;

static bool      _bInterfaceActive = false;


///////////// loop /////////////////////////////
#define _DEFAULT_LOOP false
static bool         _b_loop        = _DEFAULT_LOOP;
static const TCHAR* _filename_loop = _T("loop.bin");
static const pxFile2* _ref_file_profile = NULL;

void MainProc_set_file_profile( const pxFile2* file_profile )
{
	_ref_file_profile = file_profile;
}

static bool _loop_save()
{
	pxDescriptor* desc = NULL;
	if( !_ref_file_profile->open_w( &desc,  _filename_loop, NULL, NULL ) ) return false;
	if( !desc->w_asfile( &_b_loop, sizeof(_b_loop), 1 ) ){ SAFE_DELETE( desc ); return false; }
	SAFE_DELETE( desc );
	return true;
}
static bool _loop_load()
{
	bool          b_ret = false;
	pxDescriptor* desc  = NULL ;

	if( !_ref_file_profile->open_r( &desc,  _filename_loop, NULL, NULL ) ) goto term;
	if( !desc->r( &_b_loop, sizeof(_b_loop), 1 ) ) goto term;
	b_ret = true;
term:
	if( !b_ret ) _b_loop = _DEFAULT_LOOP;
	SAFE_DELETE( desc );
	return b_ret;
}
//////////////////////////////////


extern int32_t g_MinimizeWidth ;
extern int32_t g_MinimizeHeight;


INT_PTR CALLBACK
dlg_About(       HWND hDlg, UINT msg, WPARAM w, LPARAM l );

INT_PTR CALLBACK
dlg_YesNo(       HWND hWnd, UINT msg, WPARAM w, LPARAM l );

INT_PTR CALLBACK
dlg_NoiseDesign_Quality( HWND hDlg, UINT msg, WPARAM w, LPARAM l );

INT_PTR CALLBACK
dlg_Config_Procedure(   HWND hDlg, UINT msg, WPARAM w, LPARAM l );

INT_PTR CALLBACK
dlg_PCM_Volume      (   HWND hDlg, UINT msg, WPARAM w, LPARAM l );


// タイトルバー表記
void MainWindow_SetTitle( const TCHAR *path )
{
	TCHAR str[MAX_PATH];
	TCHAR *p_name;
	TCHAR *empty = _T("-");

	if( !path || path[0] == '\0' ) p_name = empty;
	else                           p_name = PathFindFileName( path );

#ifdef NDEBUG
	_stprintf_s( str, MAX_PATH, _T("%s [%s]"      ), g_app_name, p_name );
#else
	_stprintf_s( str, MAX_PATH, _T("%s Debug [%s]"), g_app_name, p_name );
#endif
	SetWindowText( g_hWnd_Main, str );
}

static void _MinimizedWindow()
{
	_bInterfaceActive = false;
}

static void _RestoredWindow()
{
	_bInterfaceActive = true;
}




static bool _SystemTask()
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

static void _Function_IDM_LOOP( HWND hWnd )
{
	HMENU hMenu = GetMenu( hWnd );
	_b_loop = _b_loop ? false : true;
	CheckMenuItem( hMenu, IDM_LOOP, MF_BYCOMMAND|(_b_loop?MF_CHECKED:MF_UNCHECKED) );
	_loop_save();
}
bool MainProc_IsLoop()
{
	return _b_loop;
}

// 環境設定
static bool _Function_IDM_CONFIG( HWND hwnd )
{
	static ptConfig _cfg( _ref_file_profile, ptnDEFAULT_SPS, ptnDEFAULT_CH_NUM, ptnDEFAULT_BUF_SEC );

	_cfg.load();

	PcmTable_Woice_Stop( true );

	if( !DialogBoxParam( g_hInst, _T("DLG_CONFIG"), hwnd, dlg_Config_Procedure, (LPARAM)&_cfg ) ) return true;

	if( !_cfg.save() ) MessageBox( hwnd, _T("Save Config."), _T("Error"), MB_ICONERROR );

	if( !g_strm_xa2->stream_finalize( 0.1f, 10, 3 ) )
	{
		MessageBox( hwnd, _T("stream stop timeout."), _T("error"), MB_OK|MB_ICONEXCLAMATION );
		return false;
	}

	PcmTable_Woice_Update();

	if( !g_strm_xa2->stream_start( _cfg.strm->ch_num, _cfg.strm->sps, _cfg.strm->buf_sec ) )
	{
		MessageBox( hwnd, _T("stream start."), _T("error"), MB_OK|MB_ICONEXCLAMATION );
		return false;
	}
	return true;
}

static bool _Function_IDM_CONVERTVOLUME( HWND hWnd )
{
	float v = 1;

	PcmTable_Woice_Stop( true );

	if( DialogBoxParam( g_hInst, _T("DLG_PCM_VOLUME"), hWnd, dlg_PCM_Volume, (LPARAM)&v ) )
	{
		PcmTable_Woice_PCM_ConvertVolume( v );
		PcmTable_Woice_Update();
	}
	return true;
}

static bool _Load_PCM( HWND hWnd, const TCHAR* path )
{
	if( !PcmTable_Wave_Load( path ) )
	{
		TCHAR str[ 100 ] = {0};
		if( Japanese_Is() ) _stprintf_s( str, 100, _T("音源の読み込みに失敗しました") );
		else                _stprintf_s( str, 100, _T("load *.wav"                  ) );
		Japanese_MessageBox( hWnd, str, _T("error"), MB_OK|MB_ICONEXCLAMATION );
		MainWindow_SetTitle( NULL );
		return false;
	}

	MainWindow_SetTitle  ( path );
	PcmTable_Woice_Update();

	return true;
}

// load PCM.
static bool _Function_IDM_LOAD_PCM( HWND hWnd )
{
	static HEARSELECTDIALOGSTRUCT hear = {0};

	hear.visible_flags = HEARSELECTVISIBLE_PCM;

	if( g_path_dlg_wav->get_last_path( hear.dir_default, MAX_PATH ) )
	{
		PathRemoveFileSpec( hear.dir_default );
	}
	else
	{
		_stprintf_s( hear.dir_default, MAX_PATH, _T("%s\\%s"), g_dir_module, g_default_material_folder );
	}

	PcmTable_Woice_Stop( true );

	{ int sps; g_strm_xa2->stream_get_quality_safe( NULL, &sps, NULL ); g_strm_woi->set_sps( sps ); }

	if( !pxtoneTool_HearSelect_Dialog( hWnd, &hear ) ) return true;

	g_path_dlg_wav->set_loaded_path( hear.path_selected );

	return _Load_PCM( hWnd, hear.path_selected );
}

// save pcm..
static bool _Function_IDM_SAVE_PCM( HWND hWnd, bool b_as )
{
	TCHAR path_dst[ MAX_PATH ] = {0};
	TCHAR title   [    32    ] = {0};

	if( !g_path_dlg_wav->entrust_save_path( hWnd, b_as, path_dst, _T("no name") ) ) return true;

	if( !PcmTable_Wave_Save( path_dst ) )
	{
		if( Japanese_Is() ) _stprintf_s( title, 32, _T("音源の保存に失敗しました") );
		else                _stprintf_s( title, 32, _T("Save *.wav"              ) );
		Japanese_MessageBox( hWnd,   title, _T("error"), MB_OK|MB_ICONEXCLAMATION );
		return false;
	}
	MainWindow_SetTitle( path_dst );

	return true;
}

// ファイルドロップ(ウインドウ)
static bool _Function_WM_DROPFILES( HWND hWnd, WPARAM wParam )
{
	TCHAR path[ MAX_PATH ] = {0};
	HDROP hDrop = (HDROP)wParam;

	if( DragQueryFile( hDrop, -1, NULL, 0 ) != 0 )
	{
		DragQueryFile( hDrop,  0, path, MAX_PATH );

		if( !_tcsicmp( PathFindExtension( path ), _T(".wav") ) )
		{
			if( _Load_PCM( hWnd, path ) ) g_path_dlg_wav->set_loaded_path( path );
		}
	}

	DragFinish( hDrop );
	return true;
}


LRESULT CALLBACK WindowProc_Main( HWND hWnd, UINT msg, WPARAM w, LPARAM l )
{
	switch( msg )
	{
	case WM_CREATE:

		g_hWnd_Main = hWnd;
		UpdateWindow( hWnd );
		MainWindow_SetTitle( NULL );
		_bInterfaceActive = true;
		DragAcceptFiles( hWnd, true );

		_loop_load();
		{
			HMENU hMenu = GetMenu( hWnd );
			CheckMenuItem( hMenu, IDM_LOOP, MF_BYCOMMAND|(_b_loop?MF_CHECKED:MF_UNCHECKED) );
		}
		break;

	case WM_CLOSE:

		DestroyWindow( hWnd );
		PostQuitMessage( 0 );
		break;

	case WM_PAINT:
	{
		HDC         hdc;
		PAINTSTRUCT ps;

		hdc = BeginPaint(  hWnd, &ps  );
		Interface_Process( hWnd, true );
		EndPaint(          hWnd, &ps  );
		break;
	}
	break;

	case WM_ACTIVATE:
		switch( LOWORD( w ) )
		{
		case WA_ACTIVE:
		case WA_CLICKACTIVE:

			WINDOWPLACEMENT place;
			place.length = sizeof(WINDOWPLACEMENT);
			if( GetWindowPlacement( g_hWnd_Main, &place ) )
			{
				if( place.showCmd != SW_SHOWMINIMIZED )
				{
					Interface_Process( g_hWnd_Main, true );
				}
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
		case IDM_CLOSE        : SendMessage( hWnd, WM_CLOSE, 0, 0       ); break;
		case IDM_CONFIG       : _Function_IDM_CONFIG       ( hWnd        ); break;
		case IDM_CONVERTVOLUME: _Function_IDM_CONVERTVOLUME( hWnd        ); break;
		case IDM_SAVE_AS_PCM  : _Function_IDM_SAVE_PCM     ( hWnd, true  ); break;
		case IDM_SAVE         : _Function_IDM_SAVE_PCM     ( hWnd, false ); break;
		case IDM_LOAD_PCM     : _Function_IDM_LOAD_PCM     ( hWnd        ); break;
		case IDM_VOLUMEDIALOG : OpenVolumeControl(           hWnd        ); break;
		case IDM_ABOUT        : DialogBox( g_hInst, _T("DLG_ABOUT"), hWnd, dlg_About ); break;
		case IDM_LOOP         : _Function_IDM_LOOP(         hWnd        ); break;

		default: break;
		}
		break;


	case WM_MOVE:
		pxwWindowRect_save( hWnd, g_main_rect_name );
		break;

	case WM_SIZE:
		switch( w )
		{
		case SIZE_MINIMIZED: _MinimizedWindow(); break;
		case SIZE_MAXIMIZED:
		case SIZE_RESTORED :
			if( g_dxdraw ) g_dxdraw->WindowMode_mag( hWnd, 0, LOWORD(l), HIWORD(l) );
			pxwWindowRect_save( hWnd, g_main_rect_name );
			break;
		}
		break;

	// minimum size
	case WM_GETMINMAXINFO:
	{
		RECT rc_work;
		SystemParametersInfo( SPI_GETWORKAREA, 0, &rc_work, 0 );

		MINMAXINFO* pmm = (MINMAXINFO*)l;
		pmm->ptMinTrackSize.x = g_MinimizeWidth ; // min w
		pmm->ptMinTrackSize.y = g_MinimizeHeight; // min h
		pmm->ptMaxTrackSize.x = rc_work.right   ; // max w
		pmm->ptMaxTrackSize.y = g_MinimizeHeight; // max h
	}
	break;

	case WM_KEYDOWN    :
	case WM_KEYUP      :
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP  :
	case WM_RBUTTONUP  : KeyControl_WM_MESSAGE(    hWnd, msg, w ); Interface_Process( hWnd, false ); break;
	case WM_MOUSEMOVE  :                                           Interface_Process( hWnd, true  ); break;
	case WM_MOUSEWHEEL : KeyControl_WM_MOUSEWHEEL( hWnd, msg, w ); Interface_Process( hWnd, true  ); break;
	case WM_DROPFILES  : _Function_WM_DROPFILES(   hWnd,      w ); Interface_Process( hWnd, true  ); break;

	default:

		return DefWindowProc( hWnd, msg, w, l );
	}

	return 0;
}
