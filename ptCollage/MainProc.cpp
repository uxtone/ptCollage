
#include <pxwDx09Draw.h>
#include <uxStr.h>
#include <pxPath.h>
extern pxwDx09Draw*            g_dxdraw  ;

#include <pxtnService.h>
extern pxtnService*            g_pxtn    ;

#include <pxtnPulse_NoiseBuilder.h>
extern pxtnPulse_NoiseBuilder* g_ptn_bldr;

#include <pxtonewinXA2.h>
extern pxtonewinXA2*           g_strm_xa2;


#include <pxwWindowRect.h>
#include <pxwFilePath.h>

#include <pxwPathDialog.h>
extern pxwPathDialog* g_path_dlg_proj ;
extern pxwPathDialog* g_path_dlg_build;

#include <pxwAlteration.h>
extern pxwAlteration* g_alte;

#include "../Generic/KeyControl.h"
#include "../Generic/OpenVolumeControl.h"
#include "../Generic/Menu_History.h"
#include "../Generic/Japanese.h"

#include "../Generic/pxMidiIn.h"
extern pxMidiIn *g_midi_in;

#include "../ptConfig/ptConfig_Build.h"
#include "../ptConfig/ptConfig.h"

#include "./ptCollage.h"

#include "./interface/if_Player.h"
#include "./interface/if_Copier.h"
#include "./interface/if_Projector.h"
#include "./interface/if_Effector.h"
#include "./interface/if_UnitTray.h"
#include "./interface/if_WoiceTray.h"

#include "./dialog/dlg_EventVolume.h"
#include "./dialog/dlg_BuildProgress.h"
#include "./dialog/dlg_Delay.h"
#include "./dialog/dlg_OverDrive.h"

#include "./resource.h"
#include "./UndoEvent.h"
#include "./Woice.h"

#include "./MidiInput.h"

extern uxDS        g_main_rect_name;
extern uxSS<32>        g_app_name    ;

extern int32_t      g_MinimizeWidth ;
extern int32_t      g_MinimizeHeight;

extern HINSTANCE    g_hInst         ;
extern HMENU        g_hMenu_Main    ;
extern HWND         g_hWnd_Main     ;

extern uxDS        g_posted_path   ;

static bool         _bInterfaceActive = false;
static const uxSS<12>  _bld_cfg_name     = "bld_cfg.bin";
static const pxFile2* _ref_file_profile = NULL;

void MainProc_set_file_profile( const pxFile2* file_profile )
{
	_ref_file_profile = file_profile;
}

INT_PTR CALLBACK
dlg_YesNo(          HWND hDlg, UINT msg, WPARAM w, LPARAM l );

INT_PTR CALLBACK
dlg_About(          HWND hDlg, UINT msg, WPARAM w, LPARAM l );

INT_PTR CALLBACK
dlg_Property(       HWND hDlg, UINT msg, WPARAM w, LPARAM l );

INT_PTR CALLBACK
dlg_CopyMeas(       HWND hDlg, UINT msg, WPARAM w, LPARAM l );

INT_PTR CALLBACK
dlg_Scope(          HWND hDlg, UINT msg, WPARAM w, LPARAM l );

INT_PTR CALLBACK
dlg_Config_Procedure(      HWND hDlg, UINT msg, WPARAM w, LPARAM l );

INT_PTR CALLBACK
dlg_BuildOption_Procedure( HWND hDlg, UINT msg, WPARAM w, LPARAM l );

// メニュー（プロジェクト）
bool Function_WM_DROPFILES    ( HWND hWnd, WPARAM wParam );
bool Function_DropProjectFile ( HWND hWnd, uxDS& lpszArgs );

bool pxtoneProject_IDM_INITPROJECT ( HWND hWnd );
bool pxtoneProject_IDM_SAVEPROJECT ( HWND hWnd, bool b_as );
void pxtoneProject_IDM_LOADPROJECT ( HWND hWnd );
bool pxtoneProject_IDM_HISTORY     ( HWND hWnd, UINT idm );
bool pxtoneProject_IDM_OUTPUTTUNEAS( HWND hWnd );
bool pxtoneProject_IDM_SAVEPROJECTDIFFERENCE( HWND hWnd, bool *pb_bool );

bool    pxtoneProject_load_and_init_tools( HWND hWnd, const uxDS& path, bool *pb_cancel, bool *pb_save_failed );
bool    GetDroppedPath_Window     ( HWND hWnd, WPARAM wParam, uxDS& path_drop );
int32_t GetCompileVersion         ( int32_t *p1, int32_t *p2, int32_t *p3, int32_t *p4 );


// メイン処理呼び出し
void Interface_Process( HWND hWnd, bool bDraw );



// タイトルバー表記
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

void MainWindow_SetAsterisk( bool b )
{
	uxSS<MAX_PATH>    str = { 0 };
	int32_t len           =   0  ;
	GetWindowText( g_hWnd_Main, uxTOut( str ), MAX_PATH );
	len = strlen( str );
	if( !len ) return;

	if(  b && str[len-1] != '*' ){ strcat( str, "*" )   ; SetWindowText( g_hWnd_Main, uxT( str ) ); }
	if( !b && str[len-1] == '*' ){          str[len-1] = '\0'; SetWindowText( g_hWnd_Main, uxT( str ) ); }

}

// 最小化
static void _MinimizedWindow()
{
	_bInterfaceActive = false;
}

// ウインドウサイズ
static void _RestoredWindow()
{
	_bInterfaceActive = true;
}


//システムタスク
bool SystemTask()
{
	MSG msg;

	//メッセージループを生成
	while( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE) || !_bInterfaceActive )
	{
		if( !GetMessage( &msg, NULL, 0, 0 ) ) return false;
		TranslateMessage( &msg );
		DispatchMessage(  &msg );
	}
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
	MessageBox( g_hWnd_Main, uxT( "stopping timeout." ), uxT( g_app_name ), MB_ICONEXCLAMATION );
	return false;
}

// アンドゥ
bool Function_IDM_UNDOEVENT()
{
	if( !InquireOperation() ) return false;
	UndoEvent_Pop( true );
	return true;
}

// リドゥ
bool Function_IDM_REDOEVENT()
{
	// building now..
	if( !InquireOperation() ) return false;
	UndoEvent_Pop( false );
	return true;
}

// トランスポーズ
bool Function_IDM_TRANSPOSE( HWND hwnd )
{
	if( !InquireOperation() ) return false;
	DialogBoxParam( g_hInst, uxT( "DLG_EVENTVOLUME" ), hwnd, dlg_EventVolume, (LPARAM)enum_EventKind_Key );
	return true;
}

// パン
bool Function_IDM_PAN_TIME( HWND hwnd )
{
	if( !InquireOperation() ) return false;
	DialogBoxParam( g_hInst, uxT( "DLG_EVENTVOLUME" ), hwnd, dlg_EventVolume, (LPARAM)enum_EventKind_TimePan );
	return true;
}

// パン
bool Function_IDM_PAN_VOLUME( HWND hwnd )
{
	if( !InquireOperation() ) return false;
	DialogBoxParam( g_hInst, uxT( "DLG_EVENTVOLUME" ), hwnd, dlg_EventVolume, (LPARAM)enum_EventKind_VolPan );
	return true;
}

// ベロシティ
bool Function_IDM_VELOCITY( HWND hwnd )
{
	if( !InquireOperation() ) return false;
	DialogBoxParam( g_hInst, uxT( "DLG_EVENTVOLUME" ), hwnd, dlg_EventVolume, (LPARAM)enum_EventKind_Velocity );
	return true;
}

// ボリューム
bool Function_IDM_VOLUME( HWND hwnd )
{
	if( !InquireOperation() ) return false;
	DialogBoxParam( g_hInst, uxT( "DLG_EVENTVOLUME" ), hwnd, dlg_EventVolume, (LPARAM)enum_EventKind_Volume );
	return true;
}


// ディレイ
#define DEFAULT_DELAYSCALE       DELAYUNIT_Beat
#define DEFAULT_DELAYFREQUENCY    3.3f
#define DEFAULT_DELAYRATE        33.0f

bool Function_IDM_DELAY( int32_t index )
{
	if( !InquireOperation() ) return false;

	bool                      b_new = false;
	static EFFECTSTRUCT_DELAY delay  ;
	if( index < 0 )
	{
		index       = 0;
		b_new       = true ;
		delay.group = 0;
		delay.unit  = DEFAULT_DELAYSCALE    ;
		delay.freq  = DEFAULT_DELAYFREQUENCY;
		delay.rate  = DEFAULT_DELAYRATE     ;
	}
	else
	{
		b_new = false;
		pxtnDelay *p_delay = g_pxtn->Delay_Get( index );
		delay.freq  = p_delay->get_freq ();
		delay.group = p_delay->get_group();
		delay.unit  = p_delay->get_unit ();
		delay.rate  = p_delay->get_rate ();
	}

	if( !DialogBoxParam( g_hInst, uxT( "DLG_DELAY" ), g_hWnd_Main, dlg_Delay_Procedure, (LPARAM)&delay ) ) return true;

	if( delay.b_delete )
	{
		if( b_new ) return true;
		g_pxtn->Delay_Remove( index );
	}
	else
	{
		// new
		if( b_new )
		{
			if( !g_pxtn->Delay_Add( delay.unit, delay.freq, delay.rate, delay.group ) ) return false;
		}
		// update
		else
		{
			if( !g_pxtn->Delay_Set( index, delay.unit, delay.freq, delay.rate, delay.group ) ) return false;
		}
		if( g_pxtn->Delay_ReadyTone( index ) != pxtnOK )
		{
			Japanese_MessageBox( g_hWnd_Main, "ready delay tone", "error", MB_OK|MB_ICONEXCLAMATION );
		}
	}
	// delete

	g_alte->set();

	return true;
}


bool Function_IDM_OVERDRIVE( int32_t index )
{
	if( !InquireOperation() ) return false;

	bool                          b_new = false;
	static EFFECTSTRUCT_OVERDRIVE over  ;
	if( index < 0    )
	{
		index       = 0;
		b_new       = true ;
		over.group = 0;
		over.cut   = TUNEOVERDRIVE_DEFAULT_CUT  ;
		over.amp   = TUNEOVERDRIVE_DEFAULT_AMP  ;
	}
	else
	{
		const pxtnOverDrive *p_ov = g_pxtn->OverDrive_Get( index );
		over.group = p_ov->get_group();
		over.cut   = p_ov->get_cut  ();
		over.amp   = p_ov->get_amp  ();
	}

	if( !DialogBoxParam( g_hInst, uxT( "DLG_OVERDRIVE" ), g_hWnd_Main, dlg_OverDrive_Procedure, (LPARAM)&over ) ) return true;

	if( over.b_delete )
	{
		if( b_new ) return true;
		g_pxtn->OverDrive_Remove( index );
	}
	else
	{
		if( b_new ){ if( !g_pxtn->OverDrive_Add( over.cut, over.amp, over.group ) ) return false; }
		else       {      g_pxtn->OverDrive_Set( index, over.cut, over.amp, over.group );         }
		g_pxtn->OverDrive_ReadyTone( index );
	}

	g_alte->set();

	return true;
}

// ビルド
static void _Function_IDM_BUILD( HWND hwnd, bool bBuildAs )
{
	static BUILDPROGRESSSTRUCT _build = { 0 };
	static ptConfig_Build      _cfg_bld( ptcDEFAULT_SPS, ptcDEFAULT_CH_NUM );
	pxDescriptor*              desc   = NULL;

	if( !InquireOperation() ) return;

	memset( &_build, 0, sizeof(BUILDPROGRESSSTRUCT) );

	if( _ref_file_profile->open_r( &desc, _bld_cfg_name, NULL, NULL ) )
	{
		_cfg_bld.read( desc );
	}
	SAFE_DELETE( desc );

	if( !DialogBoxParam( g_hInst, uxT( "DLG_BUILDOPTION" ), hwnd, dlg_BuildOption_Procedure, (LPARAM)&_cfg_bld ) ) return;

	if( !_ref_file_profile->open_w( &desc, _bld_cfg_name, NULL, NULL ) || !_cfg_bld.write( desc ) )
	{
		Japanese_MessageBox( g_hWnd_Main, "Save Build Config.", "error", MB_OK|MB_ICONEXCLAMATION );
	}
	SAFE_DELETE( desc );

	_build.bMute         = _cfg_bld.b_mute       ;
	_build.sec_extrafade = _cfg_bld.sec_extrafade;
	_build.sec_playtime  = _cfg_bld.sec_playtime ;
	_build.volume        = _cfg_bld.volume       ;

	switch( _cfg_bld.scope_mode )
	{
	case BUILDSCOPEMODE_TOPLAST: _build.scope = BUILDTUNESCOPE_TOPLAST; break;
	case BUILDSCOPEMODE_BYTIME : _build.scope = BUILDTUNESCOPE_BYTIME ; break;
	}

	_build.ver = GetCompileVersion( 0, 0, 0, 0 );

	// make file-name( .wav )
	{
		uxDS path;
		uxDS exte;
		if( !g_path_dlg_proj ->get_last_path( path ) ) return;
		if( !g_path_dlg_build->extension_get( exte           ) ) return;
		pxPath_remove_ext( path );
		path += ".";
		path += exte;
		if( !g_path_dlg_build->dialog_save( hwnd, _build.output_path, pxPath_name( path ) ) ) return;
	}

	{
		int old_ch_num, old_sps;

		g_pxtn->get_destination_quality( &old_ch_num, &old_sps );
		g_pxtn->set_destination_quality( _cfg_bld.strm->ch_num, _cfg_bld.strm->sps );
		if( g_pxtn->tones_ready() != pxtnOK ){ Japanese_MessageBox( hwnd, "ready tones.", "error", MB_OK|MB_ICONEXCLAMATION ); return; }

		{
			WINDOWPLACEMENT place;
			place.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement( hwnd, &place );
			ShowWindow        ( hwnd, SW_HIDE );
			DialogBoxParam( g_hInst, uxT( "DLG_BUILDPROGRESS" ), hwnd, dlg_BuildProgress, (LPARAM)&_build );
			ShowWindow        ( hwnd, place.showCmd );
		}

		g_pxtn->set_destination_quality( old_ch_num, old_sps );
	}
}


// 環境設定
static bool _Function_IDM_CONFIG( HWND hwnd )
{
	static ptConfig _cfg( _ref_file_profile, ptcDEFAULT_SPS, ptcDEFAULT_CH_NUM, ptcDEFAULT_BUF_SEC );

	if( !InquireOperation() ) return false;

	_cfg.load();

	if( !DialogBoxParam( g_hInst, uxT( "DLG_CONFIG" ), hwnd, dlg_Config_Procedure, (LPARAM)&_cfg ) ) return false;

	if( !_cfg.save() )
	{
		Japanese_MessageBox( hwnd, "Can't Save Config.", "error", MB_OK|MB_ICONEXCLAMATION );
		return false;
	}

	// apply font

	if_Projector_RedrawName   ( _cfg.font->name );
	if_UnitTray_RedrawAllName ( _cfg.font->name );
	if_WoiceTray_RedrawAllName( _cfg.font->name );

	if( !g_strm_xa2->stream_finalize( 0.1f, 10, 3 ) )
	{
		MessageBox( hwnd, uxT( "stream stop timeout." ), uxT( "error" ), MB_OK|MB_ICONEXCLAMATION );
		return false;
	}

	if( !g_strm_xa2->stream_start( _cfg.strm->ch_num, _cfg.strm->sps, _cfg.strm->buf_sec ) )
	{
		MessageBox( hwnd, uxT( "stream start." ), uxT( "error" ), MB_OK|MB_ICONEXCLAMATION );
		return false;
	}

	MidiInput_Reset( _cfg.midi->b_velo, _cfg.midi->key_tuning );
	g_midi_in->Open( _cfg.midi->name, hwnd, NULL );
	g_midi_in->Input_Start();

	return true;
}


// 小節コピー
static bool _Function_IDM_COPYMEAS( HWND hwnd )
{
	if( !InquireOperation() ) return false;

	if( DialogBoxParam( g_hInst, uxT( "DLG_COPYMEAS" ), hwnd, dlg_CopyMeas, NULL ) ){

	}
	return true;
}

// 数値スコープ
bool Function_IDM_SCOPE( HWND hwnd )
{
	if( !InquireOperation() ) return false;

	if( DialogBoxParam( g_hInst, uxT( "DLG_SCOPE" ), hwnd, dlg_Scope, NULL ) ){

	}
	return true;
}

// ファイルドロップ(ウインドウ)
static bool _Function_WM_DROPFILES( HWND hWnd, WPARAM wParam )
{
	HDROP hDrop;
	bool  b_ret = false;
	WORD  num;
	uxSS<MAX_PATH>  path;

	hDrop = (HDROP)wParam;
	num   = DragQueryFile( hDrop, -1, NULL, 0 );

	for( WORD i = 0; i < num && i < pxtnMAX_TUNEWOICESTRUCT; i++ )
	{
		DragQueryFile( hDrop, i, uxTOut( path ), MAX_PATH );
		if( !i && !ux_stricmp( pxPath_ext( path ), ".ptcop"   ) )
		{
			bool b_cancel = false, b_save_failed = false;
			pxtoneProject_load_and_init_tools( hWnd, path, &b_cancel, &b_save_failed );
			break;
		}
		else if(  !ux_stricmp( pxPath_ext( path ), ".wav" ) ||
				  !ux_stricmp( pxPath_ext( path ), ".ptvoice" ) ||
				  !ux_stricmp( pxPath_ext( path ), ".ptnoise" ) ||
				  !ux_stricmp( pxPath_ext( path ), ".ogg" ) )
		{
			if( !Woice_Add( hWnd, path ) ) goto End;
		}
	}

	b_ret = true;
End:

	DragFinish( hDrop );

	return b_ret;
}


static void _WM_CREATE( HWND hWnd )
{
	g_hWnd_Main = hWnd;
	UpdateWindow( hWnd );
	MainWindow_SetTitle( NULL );

	_bInterfaceActive = true;
	DragAcceptFiles( hWnd, true );

#ifndef _DEBUG
	DeleteMenu( GetMenu( hWnd ), IDM_OUTPUT_PTI, MF_BYCOMMAND );
#endif

}

void _WM_CLOSE( HWND hwnd )
{
	bool b_cancel;
	if( !pxtoneProject_IDM_SAVEPROJECTDIFFERENCE( hwnd, &b_cancel ) ) return;

	if_Player_SavePosition   ();
	if_Copier_SavePosition   ();
	if_Projector_SavePosition();
	if_Effector_SavePosition ();
	Menu_History_Save       ();

	DestroyWindow( hwnd );
	PostQuitMessage( 0 );
}

#include <dbt.h> // DBT_DEVNODES_CHANGED

LRESULT CALLBACK WindowProc_Main( HWND hWnd, UINT msg, WPARAM w, LPARAM l )
{
	switch( msg )
	{
	case WM_CREATE: _WM_CREATE( hWnd ); break;
	case WM_CLOSE : _WM_CLOSE ( hWnd ); break;

	/*
		case WM_PAINT:

			if( _bInterfaceActive )
				Interface_Process( hWnd, true );
			break;
			*/
	case WM_PAINT:
	{
		HDC         hdc;
		PAINTSTRUCT ps ;
		hdc = BeginPaint ( hWnd, &ps  );
		Interface_Process( hWnd, true );
		EndPaint         ( hWnd, &ps  );
//			_bInterfaceActive = _bInterfaceActive;
	}
	break;

	case WM_ERASEBKGND:
		break; // Direct2Dが自前で背景の描画を行うので、このメッセージを無効に

	case WM_ACTIVATE:

		switch( LOWORD( w ) )
		{
		case WA_ACTIVE     :
		case WA_CLICKACTIVE:
			WINDOWPLACEMENT place;
			place.length      = sizeof(WINDOWPLACEMENT);
			if( GetWindowPlacement( g_hWnd_Main, &place ) && place.showCmd != SW_SHOWMINIMIZED ) Interface_Process( g_hWnd_Main, true );
			_bInterfaceActive = true;
			KeyControl_Clear();
			break;

		default: _bInterfaceActive = false; break;
		}
		break;

	//コマンドメッセージ ========
	case WM_COMMAND:

		switch( LOWORD( w ) )
		{
		// メインウインドウを閉じさせる
		case IDM_CLOSE        : SendMessage( hWnd, WM_CLOSE, 0, 0 ); break;

		// property
		case IDM_PROPERTY     : DialogBox( g_hInst, uxT( "DLG_PROPERTY" ), hWnd, dlg_Property ); break;
		case IDM_ABOUT        : DialogBox( g_hInst, uxT( "DLG_ABOUT" ), hWnd, dlg_About    ); break;

		case IDM_BUILDAS      : _Function_IDM_BUILD(    hWnd, true  ); break;
		case IDM_CONFIG       : _Function_IDM_CONFIG(   hWnd        ); break;
		case IDM_COPYMEAS     : _Function_IDM_COPYMEAS( hWnd        ); break;
		case IDM_SCOPE        : Function_IDM_SCOPE(     hWnd        ); break;

		// project
		case IDM_INITPROJECT  : pxtoneProject_IDM_INITPROJECT(   hWnd );  Interface_Process( hWnd, true ); break;
		case IDM_LOADPROJECT  : pxtoneProject_IDM_LOADPROJECT(   hWnd );  Interface_Process( hWnd, true ); break;
		case IDM_SAVEPROJECTAS: pxtoneProject_IDM_SAVEPROJECT(   hWnd, true  ); break;
		case IDM_SAVEPROJECT  : pxtoneProject_IDM_SAVEPROJECT(   hWnd, false ); break;
		case IDM_OUTPUTTUNE   : pxtoneProject_IDM_OUTPUTTUNEAS(  hWnd ); break;

		case IDM_UNDOEVENT    : if( Function_IDM_UNDOEVENT( ) ) Interface_Process( hWnd, true ); break;
		case IDM_REDOEVENT    : if( Function_IDM_REDOEVENT( ) ) Interface_Process( hWnd, true ); break;
		case IDM_PAN_VOLUME   : Function_IDM_PAN_VOLUME(    hWnd    ); break;
		case IDM_PAN_TIME     : Function_IDM_PAN_TIME(      hWnd    ); break;
		case IDM_VELOCITY     : Function_IDM_VELOCITY(      hWnd    ); break;
		case IDM_VOLUME       : Function_IDM_VOLUME(        hWnd    ); break;
		case IDM_VOLUMEDIALOG : OpenVolumeControl(          hWnd    ); break;

		case IDM_HISTORY_0    :
		case IDM_HISTORY_1    :
		case IDM_HISTORY_2    :
		case IDM_HISTORY_3    :
		case IDM_HISTORY_4    :
		case IDM_HISTORY_5    :
		case IDM_HISTORY_6    :
		case IDM_HISTORY_7    :
		case IDM_HISTORY_8    :
		case IDM_HISTORY_9    :
		case IDM_HISTORY_10   :
		case IDM_HISTORY_11   :
		case IDM_HISTORY_12   :
		case IDM_HISTORY_13   :
		case IDM_HISTORY_14   :
		case IDM_HISTORY_15   :
		case IDM_HISTORY_16   :
		case IDM_HISTORY_17   :
		case IDM_HISTORY_18   :
		case IDM_HISTORY_19   :


			pxtoneProject_IDM_HISTORY( hWnd, LOWORD( w ) ); break;

		default:
			break;
		}
		break;

	// 起動後ファイルドロップ
	case WM_DROPFILES:

		_Function_WM_DROPFILES( hWnd, w    );
		Interface_Process(      hWnd, true );
		break;

	// 起動後関連ファイル
	case WM_USER_RELATEDFILE:
		if( g_posted_path )
		{
			bool b_cancel = false, b_save_failed = false;
			pxtoneProject_load_and_init_tools( hWnd, g_posted_path, &b_cancel, &b_save_failed );
		}

		Interface_Process( hWnd, true );
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
		pmm->ptMaxTrackSize.y = rc_work.bottom  ; // max h
	}
	break;

	case WM_KEYDOWN    :
	case WM_KEYUP      :
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP  :
	case WM_RBUTTONUP  :
		KeyControl_WM_MESSAGE( hWnd, msg, w );
		Interface_Process(     hWnd, false  );
		break;

	case WM_MOUSEMOVE:
		if( hWnd == GetFocus()	) Interface_Process( hWnd, true  );
		break;

	case WM_MOUSEWHEEL:
		KeyControl_WM_MOUSEWHEEL( hWnd, msg, w );
		Interface_Process( hWnd, true );
		break;

	case MIM_OPEN     :
	case MIM_CLOSE    :
	case MIM_DATA     :
	case MIM_LONGDATA :
	case MIM_ERROR    :
	case MIM_LONGERROR:
	case MIM_MOREDATA :
		if( pxMidiIn_Callback( NULL, msg, 0, l, w ) ) Interface_Process( hWnd, false );
		break;

	case WM_DEVICECHANGE:

		switch( w )
		{
		case DBT_DEVNODES_CHANGED:

			if( g_midi_in )
			{
				ptConfig cfg( _ref_file_profile, ptcDEFAULT_SPS, ptcDEFAULT_CH_NUM, ptcDEFAULT_BUF_SEC );
				cfg.load();
				g_midi_in->Close();
				if( g_midi_in->Open ( cfg.midi->name, g_hWnd_Main, NULL ) ) g_midi_in->Input_Start();
			}
			break;

		default: return DefWindowProc( hWnd, msg, w, l );
		}
		break;

	default: return DefWindowProc( hWnd, msg, w, l );
	}

	return 0;
}
