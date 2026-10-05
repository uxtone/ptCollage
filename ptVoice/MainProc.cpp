

#include <pxwDx09Draw.h>
#include <uxStr.h>
#include <pxPath.h>
extern pxwDx09Draw*    g_dxdraw  ;

#include <pxwAlteration.h>
extern pxwAlteration*  g_alte    ;

#include <pxtnService.h>
extern pxtnService*    g_pxtn    ;

#include <pxtonewinXA2.h>
extern pxtonewinXA2*   g_strm_xa2;

#include <pxtonewinWoice.h>
extern pxtonewinWoice* g_strm_woi;

#include "../Generic/pxMidiIn.h"
extern pxMidiIn*       g_midi_in ;

#include "../CursorKeyCtrl.h"
extern CursorKeyCtrl*  g_curkey  ;

#include <pxwFilePath.h>
#include <pxwWindowRect.h>
#include <pxwPathDialog.h>

#include <pxMouse.h>
extern pxMouse* g_mouse;

#include "./interface/Interface.h"
extern Interface* g_interface;


extern pxwPathDialog* g_path_dlg_ptv;
extern pxwPathDialog* g_path_dlg_wav;
extern pxwPathDialog* g_path_dlg_txt;

#include "../Generic/KeyControl.h"
#include "../Generic/OpenVolumeControl.h"
#include "../Generic/Japanese.h"

#include "../ptConfig/ptConfig.h"

#include "../pxtoneTool/pxtoneTool.h"

#include "./resource.h"


#include "./ptVoice.h"
#include "./MidiInput.h"
#include "./VoiceFile.h"

#include "./WoiceUnit.h"
extern WoiceUnit* g_vunit;

#include "./ptVoice.h"

extern HINSTANCE g_hInst         ;
extern HWND      g_hWnd_Main     ;
extern uxSS<32>     g_app_name  ;
extern uxSS<MAX_PATH>     g_dir_module  ;
extern uxDS     g_main_rect_name;
extern int32_t   g_client_min_w  ;
extern int32_t   g_client_min_h  ;

static bool      _bInterfaceActive        = false;

static uxSS<12>     _default_material_folder = "my_material";


static const uxSS<1>  _title_ptv_save = "";
static const uxSS<1>  _title_ptv_load = "";
static const uxSS<1>  _title_txt_save = "";
static const uxSS<1>  _title_txt_load = "";

static const pxFile2* _ref_file_profile = NULL;

void MainProc_set_file_profile( const pxFile2* file_profile )
{
	_ref_file_profile = file_profile;
}


INT_PTR CALLBACK
dlg_About(     HWND hDlg, UINT msg, WPARAM w, LPARAM l );

INT_PTR CALLBACK
dlg_YesNo(     HWND hwnd, UINT msg, WPARAM w, LPARAM l );

INT_PTR CALLBACK
dlg_Config_Procedure( HWND hDlg, UINT msg, WPARAM w, LPARAM l );

void MainWindow_SetTitle( const uxDS& path )
{
	uxSS<MAX_PATH> str;
	uxDS p_name;
	uxDS empty = "-";

	if( !path || path[0] == '\0' ) p_name = empty;
	else                           p_name = pxPath_name( path );

#ifdef NDEBUG
	ux_sprintf_s( str, MAX_PATH, "%s [%s]", g_app_name, *p_name );
#else
	ux_sprintf_s( str, MAX_PATH, "%s Debug [%s]", g_app_name, *p_name );
#endif
	SetWindowText( g_hWnd_Main, uxT( str ) );
}

// 最小化
static void _MinimizedWindow()
{
	_bInterfaceActive = false;
}

// ウインドウサイズ
static void _RestoredWindow()
{
	_bInterfaceActive = true ;
}

static bool _Load_PTVOICE( HWND hwnd, const uxDS& path )
{
	g_interface->organkey_reset();

	if( !VoiceFile_Load( path )                  ) return false;
	if( !g_path_dlg_ptv->set_loaded_path( path ) ) return false;
	if( !g_vunit->strm_ready()                   ) return false;

	MainWindow_SetTitle( path );

	g_interface->map_redraw_set();
	return true;
}

static bool _csv_attach( HWND hwnd, const uxDS& path_src )
{
	g_interface->organkey_reset();

	if( !g_vunit->csv_attach( path_src ) )
	{
		Japanese_MessageBox( hwnd, "read csv.", "error", MB_OK|MB_ICONEXCLAMATION );
		return false;
	}

	g_vunit  ->strm_ready();

	g_interface->map_redraw_set();

	return true;
}

// ファイルドロップ(ウインドウ)
static bool _Function_WM_DROPFILES( HWND hwnd, WPARAM wParam )
{
	uxSS<MAX_PATH> path = {0};
	const char*  p_ext     = "";
	HDROP hDrop;

	hDrop = (HDROP)wParam;

	if( DragQueryFile( hDrop, -1, NULL, 0 ) != 0 )
	{
		DragQueryFile( hDrop,  0, uxTOut( path ), MAX_PATH );

		p_ext = pxPath_ext( path );

		if     ( !strcmp( p_ext, ".ptvoice" ) )
		{
			if( _Load_PTVOICE( hwnd, path ) ) g_path_dlg_ptv->set_loaded_path( path );
			else                              MessageBox( hwnd, uxT( "ERROR: load ptvoice" ), uxT( g_app_name ), MB_ICONEXCLAMATION );
		}
		else if( !strcmp( p_ext, ".txt" ) )
		{
			if( _csv_attach  ( hwnd, path ) ) g_path_dlg_txt->set_loaded_path( path );
			else                              MessageBox( hwnd, uxT( "ERROR: load txt" ), uxT( g_app_name ), MB_ICONEXCLAMATION );
		}
	}

	DragFinish( hDrop );
	return true;

}


//システムタスク
static bool _SystemTask()
{
	MSG msg;

	//メッセージループを生成
	while( PeekMessage(   &msg, NULL, 0, 0, PM_NOREMOVE) || !_bInterfaceActive )
	{
		if( !GetMessage(  &msg, NULL, 0, 0 ) ) return false;
		TranslateMessage( &msg );
		DispatchMessage(  &msg );
	}

	return true;
}

static void _Function_IDM_INITIALIZE( HWND hwnd )
{
	uxSS<32> str = {0};

	if( Japanese_Is() ) strcpy( str, "初期化します" );
	else                strcpy( str, "Initialize" );

	if( !DialogBoxParam( g_hInst, uxT( "DLG_YESNO" ), hwnd, dlg_YesNo, (LPARAM)str ) ) return;

	MainWindow_SetTitle( NULL );
	g_path_dlg_ptv->last_filename_clear();
	g_path_dlg_txt->last_filename_clear();

	g_vunit->channel_set( 0 );

	if( !g_vunit->IDM_INITIALIZE_() )
	{
		Japanese_MessageBox( hwnd, "initialize voices", "error", MB_OK|MB_ICONEXCLAMATION );
	}
	g_alte   ->off       ();
	g_curkey ->clear     ();

	g_interface->map_redraw_set();
}


static bool _call_main_proc( HWND hwnd, bool b_draw )
{
	RECT  rc_c; GetClientRect( hwnd, &rc_c );
	fRECT rc_d;
	rc_d.l = (float)rc_c.left  ;
	rc_d.t = (float)rc_c.top   ;
	rc_d.r = (float)rc_c.right  / g_dxdraw->get_screen_mag();
	rc_d.b = (float)rc_c.bottom / g_dxdraw->get_screen_mag();

	if( g_mouse ) g_mouse->trigger_update();

	bool b_ret = false;
	if( g_interface ) g_interface->proc( &rc_d, g_mouse, b_draw );

	b_ret = true;

	if( !b_ret ) PostMessage( hwnd, WM_CLOSE, 0, 0 );
	return b_ret;
}

static void _Function_IDN_COPY( HWND hwnd, bool bBA )
{
	uxSS<32> str = {0};

	if( !bBA )
	{
		if( Japanese_Is() ) ux_sprintf_s( str, "A を B にコピーします" );
		else                ux_sprintf_s( str, "Copy A to B" );
	}
	else
	{
		if( Japanese_Is() ) ux_sprintf_s( str, "B を A にコピーします" );
		else                ux_sprintf_s( str, "Copy B to A" );
	}
	if( MessageBox( hwnd, uxT( str ), uxT( g_app_name ), MB_OKCANCEL ) == IDOK )
	{
		if( !bBA ) g_vunit->copy_ptv_voice( 0, 1 );
		else       g_vunit->copy_ptv_voice( 1, 0 );
	}
}

static bool _Function_IDM_CONFIG( HWND hwnd )
{
	static ptConfig _cfg( _ref_file_profile, ptvDEFAULT_SPS, ptvDEFAULT_CH_NUM, ptvDEFAULT_BUF_SEC );

	_cfg.load();

	if( !DialogBoxParam( g_hInst, uxT( "DLG_CONFIG" ), hwnd, dlg_Config_Procedure, (LPARAM)&_cfg ) ) return false;

	if( !_cfg.save() )
	{
		Japanese_MessageBox( hwnd, "Can't Save Config.", "error", MB_OK|MB_ICONEXCLAMATION );
		return false;
	}

	if( !g_strm_xa2->stream_finalize( 0.1f, 10, 3 ) )
	{
		MessageBox( hwnd, uxT( "stream stop timeout." ), uxT( "error" ), MB_OK|MB_ICONEXCLAMATION );
		return false;
	}

	if( !g_vunit->strm_ready() )
	{
		Japanese_MessageBox( NULL, "ready sample", "error", MB_OK|MB_ICONEXCLAMATION );
		return false;
	}

	// if( !g_strm_xa2->stream_start( _cfg.strm->ch_num, _cfg.strm->sps, _cfg.strm->buf_sec ) )
	// {
	// 	MessageBox( hwnd, "stream start.", "error", MB_OK|MB_ICONEXCLAMATION );
	// 	return false;
	// }

	MidiInput_Reset( _cfg.midi->b_velo, _cfg.midi->key_tuning );
	g_midi_in->Open( _cfg.midi->name, hwnd, NULL );
	g_midi_in->Input_Start();

	return true;
}

static bool _Function_IDM_OUTPUT_TEXT( HWND hwnd )
{
	uxDS path_dst;
	uxDS exte;

	if( !g_path_dlg_txt->get_last_path( path_dst ) &&
		g_path_dlg_ptv->get_last_path( path_dst ) )
	{
		pxPath_remove_ext( path_dst );
		if( !g_path_dlg_txt->extension_get( exte ) ) exte = "txta"; // joke.
		path_dst += exte;
	}

	if( !g_path_dlg_txt->dialog_save( hwnd, path_dst, "no name" ) ) return false;

	if( !g_vunit->csv_output( path_dst ) )
	{
		const char* title = Japanese_Is() ? "テキストファイルの出力に失敗しました" : "Error output text-file"; // a static message, not a copy
		Japanese_MessageBox( hwnd,   title, "error", MB_OK|MB_ICONEXCLAMATION );
		return false;
	}
	return true;
}

// 音源ファイルを出力
static bool _Function_IDM_SAVE_PTVOICE( HWND hwnd, bool b_as )
{
	uxDS path_dst;
	if( !g_path_dlg_ptv->entrust_save_path( hwnd, b_as, path_dst, "no name" ) ) return false;

	if( !VoiceFile_Save( path_dst ) )
	{
		const char* title = Japanese_Is() ? "音源の出力に失敗しました" : "Export *.ptvoice"; // a static message
		Japanese_MessageBox( hwnd,   title, "error", MB_OK|MB_ICONEXCLAMATION );
		return false;
	}
	g_path_dlg_txt->last_filename_clear();
	MainWindow_SetTitle(   path_dst );
	return true;
}

static bool _Function_IDM_LOAD_PTVOICE( HWND hwnd )
{
	bool  b_ret     = false;
	uxSS<48> str = { 0 };

	static HEARSELECTDIALOGSTRUCT hear = {0};

	hear.visible_flags = HEARSELECTVISIBLE_PTV;

	if( !g_path_dlg_ptv->get_last_path( hear.path_selected ) )
	{
		ux_sprintf_s( hear.dir_default, "%s\\%s", g_dir_module, _default_material_folder );
	}

	{ int sps; g_strm_xa2->stream_get_quality_safe( NULL, &sps, NULL ); g_strm_woi->set_sps( sps ); }

	if( !pxtoneTool_HearSelect_Dialog( hwnd, &hear ) ) return true;
	MainWindow_SetTitle( NULL );

	if( !_Load_PTVOICE( hwnd, hear.path_selected ) )
	{
		if( Japanese_Is() ) strcpy( str, "音源の読み込みに失敗しました" );
		else                strcpy( str, "ERROR: load ptvoice" );
		Japanese_MessageBox( hwnd, str, "error", MB_OK|MB_ICONEXCLAMATION );
		goto End;
	}

	_call_main_proc( hwnd, true );

	b_ret = true;
End:

	return b_ret;
}



static bool _WM_MOUSE_CLICK( HWND hwnd, UINT msg, LPARAM l )
{
	switch( msg )
	{
	case WM_LBUTTONDOWN: g_mouse->set_click_l( true  ); break;
	case WM_RBUTTONDOWN: g_mouse->set_click_r( true  ); break;
	case WM_LBUTTONUP  : g_mouse->set_click_l( false ); break;
	case WM_RBUTTONUP  : g_mouse->set_click_r( false ); break;
	default: return false;
	}
	_call_main_proc( hwnd, true );
	return false;
}

#include <dbt.h> // DBT_DEVNODES_CHANGED

LRESULT CALLBACK WindowProc_Main( HWND hwnd, UINT msg, WPARAM w, LPARAM l )
{
	switch( msg )
	{
	case WM_CREATE:

		g_hWnd_Main = hwnd;
		UpdateWindow( hwnd );
		MainWindow_SetTitle( NULL );
		_bInterfaceActive = true;
		DragAcceptFiles( hwnd, true );

		break;

	case WM_CLOSE:
		DestroyWindow( hwnd );
		PostQuitMessage( 0 );
		break;

	case WM_PAINT:
	{
		HDC         hdc;
		PAINTSTRUCT ps ;

		hdc = BeginPaint ( hwnd, &ps  );
		_call_main_proc  ( hwnd, true );
		EndPaint         ( hwnd, &ps  );
		break;
	}
	break;

	case WM_ACTIVATE:

		switch( LOWORD( w ) )
		{
		case WA_ACTIVE:
		case WA_CLICKACTIVE:

		{
			WINDOWPLACEMENT place = {0};
			place.length = sizeof(WINDOWPLACEMENT);

			if( GetWindowPlacement( hwnd, &place ) && place.showCmd != SW_SHOWMINIMIZED )
			{
				_call_main_proc( hwnd, true );
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
		case IDM_CLOSE          : SendMessage( hwnd, WM_CLOSE, 0, 0       ); break;
		case IDM_INITIALIZE     : _Function_IDM_INITIALIZE  ( hwnd        ); break;
		case IDM_SAVE_AS_PTVOICE: _Function_IDM_SAVE_PTVOICE( hwnd, true  ); break;
		case IDM_OUTPUT_TEXT    : _Function_IDM_OUTPUT_TEXT ( hwnd        ); break;
		case IDM_SAVE           : _Function_IDM_SAVE_PTVOICE( hwnd, false ); break;
		case IDM_LOAD_PTVOICE   : _Function_IDM_LOAD_PTVOICE( hwnd        ); break;
		case IDM_CONFIG         : _Function_IDM_CONFIG      ( hwnd        ); break;
		case IDM_ABOUT          : DialogBox( g_hInst, uxT( "DLG_ABOUT" ), hwnd, dlg_About   ); break;
		case IDM_VOLUMEDIALOG   : OpenVolumeControl         ( hwnd        ); break;
		case IDM_COPY_AB        : _Function_IDN_COPY        ( hwnd, false ); break;
		case IDM_COPY_BA        : _Function_IDN_COPY        ( hwnd, true  ); break;
		default:
			break;
		}
		break;

	case WM_MOVE:
		pxwWindowRect_save( hwnd, g_main_rect_name );
		break;

	case WM_SIZE:

		switch( w )
		{
		case SIZE_MINIMIZED: _MinimizedWindow(); break;
		case SIZE_MAXIMIZED:
		case SIZE_RESTORED :
			if( g_dxdraw ) g_dxdraw->WindowMode_mag( hwnd, 0, LOWORD(l), HIWORD(l) );
			pxwWindowRect_save( hwnd, g_main_rect_name );
			break;
		}

		break;

	// minimum size
	case WM_GETMINMAXINFO:
	{
		RECT rc_work;
		SystemParametersInfo( SPI_GETWORKAREA, 0, &rc_work, 0 );

		MINMAXINFO* pmm = (MINMAXINFO*)l;
		pmm->ptMinTrackSize.x = g_client_min_w; // min w
		pmm->ptMinTrackSize.y = g_client_min_h; // min h
		pmm->ptMaxTrackSize.x = g_client_min_w; // max w
		pmm->ptMaxTrackSize.y = rc_work.bottom; // max h
	}
	break;

	case WM_KEYDOWN    :
	case WM_KEYUP      : KeyControl_WM_MESSAGE   ( hwnd, msg, w ); _call_main_proc( hwnd, true ); break;

	case WM_MOUSEMOVE  : g_mouse->set_position(); _call_main_proc( hwnd, true ); break;

	case WM_LBUTTONUP  :
	case WM_RBUTTONUP  :
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN: if( !_WM_MOUSE_CLICK( hwnd, msg, l ) ) return DefWindowProc( hwnd, msg, w, l ); break;

	case WM_MOUSEWHEEL : KeyControl_WM_MOUSEWHEEL( hwnd, msg, w ); _call_main_proc( hwnd, true  ); break;

	case WM_DROPFILES  : _Function_WM_DROPFILES  ( hwnd,      w ); _call_main_proc( hwnd, true  ); break;

	case MIM_OPEN     :
	case MIM_CLOSE    :
	case MIM_DATA     :
	case MIM_LONGDATA :
	case MIM_ERROR    :
	case MIM_LONGERROR:
	case MIM_MOREDATA :
		if( pxMidiIn_Callback( NULL, msg, 0, l, w ) ) _call_main_proc( hwnd, false );
		break;

	case WM_DEVICECHANGE:

		switch( w )
		{
		case DBT_DEVNODES_CHANGED:

			if( g_midi_in )
			{
				ptConfig cfg( _ref_file_profile, ptvDEFAULT_SPS, ptvDEFAULT_CH_NUM , ptvDEFAULT_BUF_SEC );
				cfg.load();
				g_midi_in->Close();
				if( g_midi_in->Open ( cfg.midi->name, g_hWnd_Main, NULL ) ) g_midi_in->Input_Start();
			}
			break;

		default: return DefWindowProc( hwnd, msg, w, l );
		}
		break;

	default: return DefWindowProc( hwnd, msg, w, l );
	}

	return 0;
}
