
#include <pxtnService.h>


#include <pxPath.h>
extern pxtnService*            g_pxtn    ;

#include <pxtnPulse_NoiseBuilder.h>
extern pxtnPulse_NoiseBuilder* g_ptn_bldr;

#include <pxtonewinXA2.h>
extern pxtonewinXA2*           g_strm_xa2;

#include <pxDebugLog.h>
// #include <pxTText.h>



#include <pxwPathDialog.h>
extern pxwPathDialog* g_path_dlg_proj;
extern pxwPathDialog* g_path_dlg_tune;

#include <pxwAlteration.h>
extern pxwAlteration* g_alte;

#include "resource.h"

#include "../Generic/Menu_History.h"
#include "../Generic/Japanese.h"

#include "Project.h"
#include "UnitFocus.h"
#include "UndoEvent.h"
#include "Unit.h"

#include "interface/if_Projector.h"
#include "interface/if_Panel_Scale.h"
#include "interface/if_Player.h"
#include "interface/if_PlayField.h"
#include "interface/if_ScopeField.h"
#include "interface/if_UnitTray.h"
#include "interface/if_WoiceTray.h"
#include "interface/if_ToolPanel.h"
#include "interface/if_KeyField.h"

extern HINSTANCE  g_hInst       ;
extern HWND       g_hWnd_Main   ;
extern uxSS<MAX_PATH>       g_dir_module;
extern uxSS<32>       g_app_name;

#include "../Generic/if_gen_Scroll.h"
extern if_gen_Scroll g_ScrlEventH;
extern if_gen_Scroll g_ScrlWoiceH;
extern if_gen_Scroll g_ScrlWoiceV;
extern if_gen_Scroll g_ScrlUnitV ;
extern if_gen_Scroll g_ScrlKeyV  ;


bool InquireOperation();
bool SaveTune_Version( const uxDS& path, bool bTune );

INT_PTR CALLBACK
dlg_ProjectOption( HWND hWnd, UINT msg, WPARAM w, LPARAM l );

INT_PTR CALLBACK
dlg_YesNo(         HWND hDlg, UINT msg, WPARAM w, LPARAM l );

void    MainWindow_SetTitle               ( const uxDS& path        );
bool    pxtoneProject_IDM_SAVEPROJECTDIFFERENCE( HWND hWnd, bool *pb_bool );
int32_t GetCompileVersion( int32_t *p1, int32_t *p2, int32_t *p3, int32_t *p4 );

static void _tools_init()
{
	g_ScrlEventH.SetOffset    ( 0 );
	g_ScrlWoiceH.SetOffset    ( 0 );
	g_ScrlWoiceV.SetOffset    ( 0 );
	g_ScrlUnitV .SetOffset    ( 0 );
	if_KeyField_ResetScroll   ();
	UnitFocus_Set             ( 0, true );
	UndoEvent_ReleaseUndoRedo ();

	g_alte->off();

	if_PlayField_Reset        ();
	if_Player_OnMutePlay      ();
	if_ScopeField_Zero        ();

	if_Projector_RedrawName   ( NULL );
	if_WoiceTray_RedrawAllName( NULL );
	if_UnitTray_RedrawAllName ( NULL );
}


bool _SaveWithVersion( const uxDS& path, bool bTune, pxtnERR* p_pxtn_err )
{
	bool  b_ret = false;
	FILE* fp    = ux_fopen( path, "wb" ); if( !fp ) return false;

	*p_pxtn_err = g_pxtn->write( fp, bTune, (unsigned short)GetCompileVersion( 0, 0, 0, 0 ) );
	if( *p_pxtn_err != pxtnOK  ) goto term;
	if( !bTune ) Menu_History_Add( path );

	b_ret = true;
term:
	if( fp ) fclose( fp );
	return b_ret;
}

bool pxtoneProject_load_and_init_tools( HWND hWnd, const uxDS& path, bool *pb_cancel, bool *pb_save_failed )
{
	bool    b_ret    = false       ;
	pxtnERR pxtn_err = pxtnERR_VOID;
	uxDS    path_get;
	uxDS              err_msg;

	if( pb_cancel      ) *pb_cancel      = false;
	if( pb_save_failed ) *pb_save_failed = false;

	if( !pxtoneProject_IDM_SAVEPROJECTDIFFERENCE( hWnd, pb_cancel ) )
	{
		if( pb_save_failed ) *pb_save_failed = true;
		return false;
	}

	SetCursor( LoadCursor( NULL, IDC_WAIT ) );

	if( !InquireOperation() ){ dlog_c( "err inq(init tool)" ); return false; }

	path_get = path;

	if( path_get.size() || g_path_dlg_proj->get_last_path( path_get ) )
	{
		dlog_t( "LoadProject InitializeTool: ", path_get );
		FILE* fp = ux_fopen( path_get, "rb" ); if( !fp ) goto End;
		pxtn_err = g_pxtn->read( fp );
		if( pxtn_err != pxtnOK ){ fclose( fp ); goto End; }
		fclose( fp );
		dlog_c( "LoadProject InitializeTool( read project ok )" );
		g_path_dlg_proj->set_loaded_path( path_get );
		Menu_History_Add               ( path_get );
		MainWindow_SetTitle             ( path_get );
	}

	if( g_pxtn->tones_ready() != pxtnOK ) Japanese_MessageBox( hWnd, "ready works", "error", MB_OK|MB_ICONEXCLAMATION );

	dlog_c( "LoadProject InitializeTool( ready work ok )" );

	b_ret = true;
End:

	if( !b_ret )
	{
		// pxTText tt; tt.set_sjis_to_t( pxtnError_get_string( pxtn_err ) );
		uxDS tt; ( tt = uxDS_from_sjis( pxtnError_get_string( pxtn_err ) ) );

		g_path_dlg_proj->last_filename_clear();
		if( Japanese_Is() ) err_msg.format( "%s は開けません。", *path_get );
		else                err_msg.format( "Can't open %s.", *path_get );
		err_msg += "\r\n";
		err_msg += tt;

		Japanese_MessageBox( hWnd, *err_msg, "error", MB_OK|MB_ICONEXCLAMATION );
		MainWindow_SetTitle( NULL );
	}

	_tools_init();

	SetCursor( LoadCursor( NULL, IDC_ARROW ) );

	return b_ret;
}

// プロジェクトロード
void pxtoneProject_IDM_LOADPROJECT( HWND hWnd )
{
	uxDS  path_get;
	bool  b_cancel           = false;
	bool  b_save_failed      = false;

	if( !g_path_dlg_proj->dialog_load( hWnd, path_get ) ) return;

	if( !pxtoneProject_load_and_init_tools( hWnd, path_get, &b_cancel, &b_save_failed ) ) return;
}

bool pxtoneProject_IDM_HISTORY( HWND hWnd, UINT idm )
{
	uxDS path;
	bool b_cancel     = false;
	bool b_save_failed = false;

	if( !Menu_History_GetPath( idm, path ) ) return false;
	if( !pxtoneProject_load_and_init_tools( hWnd, path, &b_cancel, &b_save_failed ) && !b_cancel && !b_save_failed )
	{
		Menu_History_Delete( idm );
		return false;
	}
	return true;
}

bool pxtoneProject_IDM_SAVEPROJECT( HWND hwnd, bool b_as )
{
	uxDS    path_get;
	pxtnERR pxtn_err             = pxtnERR_VOID;

	InquireOperation();

	if( !g_path_dlg_proj->entrust_save_path( hwnd, b_as, path_get, "no name" ) ) return false;

	if( !_SaveWithVersion( path_get, false, &pxtn_err ) )
	{
		Japanese_MessageBox( hwnd, "open file", "error", MB_OK|MB_ICONEXCLAMATION );
		return false;
	}

	MainWindow_SetTitle( path_get );

	g_alte->off();

	return true;
}

// プロジェクト編集
void pxtoneProject_IDM_EDITPROJECT()
{
	InquireOperation();
	if( DialogBox( g_hInst, uxT( "DLG_PROJECT" ), g_hWnd_Main, dlg_ProjectOption ) )
	{
		g_pxtn->AdjustMeasNum();
		if_Projector_RedrawName( NULL );
		if( g_pxtn->tones_ready() != pxtnOK )
		{
			Japanese_MessageBox( g_hWnd_Main, "ready delay work", "error", MB_OK|MB_ICONEXCLAMATION );
		}
		g_alte->set();
	}
}

// プロジェクトを初期化
bool pxtoneProject_IDM_INITPROJECT( HWND hWnd )
{
	InquireOperation();

	const char* str = Japanese_Is() ? "プロジェクトを初期化します" : "Initialize project"; // a static message, not a copy
	if( !DialogBoxParam( g_hInst, uxT( "DLG_YESNO" ), hWnd, dlg_YesNo, (LPARAM)str ) ) return false;
	g_pxtn->clear();
	_tools_init();
	g_path_dlg_proj->last_filename_clear();
	g_path_dlg_tune->last_filename_clear();
	MainWindow_SetTitle( NULL );
	if_Projector_RedrawName( NULL );
	return true;
}

bool pxtoneProject_IDM_SAVEPROJECTDIFFERENCE( HWND hWnd, bool *pb_cancel )
{
	if( pb_cancel ) *pb_cancel = false;

	if( !g_alte->is() ) return true;

	const char* str = Japanese_Is() ? "変更を上書きしますか？" : "overwrite?"; // a static message (33 bytes in UTF-8: it must not be copied into a small buffer)
	switch( MessageBox( hWnd, uxT( str ), uxT( g_app_name ), MB_YESNOCANCEL ) )
	{
	case IDCANCEL: if( pb_cancel ) *pb_cancel = true; return false;
	case IDNO:     return true;
	}

	return pxtoneProject_IDM_SAVEPROJECT( hWnd, false );
}

bool pxtoneProject_IDM_OUTPUTTUNEAS( HWND hWnd )
{
	uxDS    path_dst;
	uxSS<MAX_PATH>    path_name = {0};
	uxDS    exte;
	pxtnERR pxtn_err = pxtnERR_VOID;

	InquireOperation();

	if( !g_path_dlg_proj->get_last_path( path_dst ) ) return false;
	if( !g_path_dlg_tune->extension_get( exte ) ) return false;
	pxPath_remove_ext( path_name );
	path_dst += ".";
	path_dst += exte;

	if( !g_path_dlg_tune->dialog_save( hWnd, path_dst, "no name" ) ) return false;

	if( !_SaveWithVersion( path_dst, true, &pxtn_err ) )
	{
		// pxTText tt; tt.set_sjis_to_t( pxtnError_get_string( pxtn_err ) );
		uxDS tt; ( tt = uxDS_from_sjis( pxtnError_get_string( pxtn_err ) ) );
		// Japanese_MessageBox( hWnd, tt.tchr(), "error", MB_OK|MB_ICONEXCLAMATION );
		Japanese_MessageBox( hWnd, *tt, "error", MB_OK|MB_ICONEXCLAMATION );
	}
	return true;
}
