

#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../../Generic/KeyControl.h"
#include "../../Generic/if_gen_Scroll.h"
#include "../../Generic/if_Generic.h"

#include "../UndoEvent.h"
#include "../UnitFocus.h"
#include "../MidiInput.h"

#include "if_VolumePanel.h"
#include "if_VolumeField.h"
#include "if_Keyboard.h"
#include "if_KeyField.h"
#include "if_BaseField.h"
#include "if_Panel_Comment.h"
#include "if_Panel_Logo.h"
#include "if_Panel_Rack.h"
#include "if_Panel_Scale.h"
#include "if_PlayField.h"
#include "if_Cursor.h"
#include "if_UnitField.h"
#include "if_UnitTray.h"
#include "if_WoiceDummy.h"
#include "if_WoiceField.h"
#include "if_WoiceTray.h"
#include "if_ToolPanel.h"
#include "if_Tenkey_Tuning.h"
#include "if_Free.h"
#include "if_Copier.h"
#include "if_Player.h"
#include "if_Projector.h"
#include "if_Effector.h"
#include "if_PlayField.h"
#include "if_ScopeField.h"
#include "if_ShortcutKey.h"
#include "if_Menu_Scope.h"
#include "if_Menu_Play.h"
#include "if_Menu_Unit.h"
#include "if_Menu_Woice.h"
#include "if_Menu_Division.h"
#include "if_Menu_Zoom.h"
#include "if_SelectNo_Woice.h"
#include "if_SelectNo_Group.h"
#include "if_SubWindows.h"
#include "Interface.h"
#include "SurfaceNo.h"

#define KEYSEPARATOR_Y (HEADER_H + TOOLPANEL_HEIGHT + UNIT_HEIGHT)
#define _FLIPWAIT 20

// 変数群 =======================================-

static bool              _b_init     = false;

static CRITICAL_SECTION  _critical          ;
static bool              _b_critical = false;

if_gen_Scroll  g_ScrlEventH;
if_gen_Scroll  g_ScrlWoiceH;
if_gen_Scroll  g_ScrlWoiceV;
if_gen_Scroll  g_ScrlUnitV ;
if_gen_Scroll  g_ScrlKeyV  ;



// 関数群 =======================================-

int32_t CountFramePerSecond();


// マウスカーソルの座標を取得
static void _GetMousePointer( HWND hWnd, float* p_cur_x, float* p_cur_y )
{
	POINT pt;

	GetCursorPos( &pt );
	ScreenToClient( hWnd, &pt );

	*p_cur_x = (float)pt.x / g_dxdraw->get_screen_mag();
	*p_cur_y = (float)pt.y / g_dxdraw->get_screen_mag();
}

static void _SetRects( const fRECT *rc_view )
{
	if_Free_SetRect         ( rc_view );
	if_Panel_Comment_SetRect( rc_view );
	if_Panel_Logo_SetRect   ( rc_view );
	if_Panel_Rack_SetRect(    rc_view );
	if_Panel_Scale_SetRect(   rc_view );
	if_ToolPanel_SetRect(     rc_view );
	if_WoiceDummy_SetRect(    rc_view );
	if_WoiceTray_SetRect    ( rc_view );
	if_WoiceField_SetRect(    rc_view );
	if_UnitTray_SetRect(      rc_view );
	if_UnitField_SetRect(     rc_view );
	if_Keyboard_SetRect(      rc_view );
	if_KeyField_SetRect(      rc_view );
	if_VolumePanel_SetRect(   rc_view );
	if_VolumeField_SetRect(   rc_view );
	if_VolumeSwitch_SetRect(  rc_view );
	if_BaseField_SetRect(     rc_view );
	if_PlayField_SetRect(     rc_view );
	if_ScopeField_SetRect(    rc_view );
	if_Player_SetRect(        rc_view );
	if_Copier_SetRect(        rc_view );
	if_Projector_SetRect(     rc_view );
	if_Effector_SetRect(      rc_view );
	if_Menu_Play_SetRect(     rc_view );
	if_Menu_Scope_SetRect(    rc_view );
	if_Menu_Unit_SetRect(     rc_view );
	if_Menu_Woice_SetRect(    rc_view );
	if_Tenkey_Tuning_SetRect( rc_view );
}



bool Interface_init( HWND hWnd, const pxPalette* palette, const pxFile2* file_profile )
{
	// スクロールバーの初期化
	g_ScrlEventH.Initialize( g_dxdraw, SURF_TABLES, false );
	g_ScrlWoiceH.Initialize( g_dxdraw, SURF_TABLES, false );
	g_ScrlWoiceV.Initialize( g_dxdraw, SURF_TABLES, true  );
	g_ScrlUnitV.Initialize ( g_dxdraw, SURF_TABLES, true  );
	g_ScrlKeyV.Initialize  ( g_dxdraw, SURF_TABLES, true  );

	g_ScrlWoiceV.SetSize   ( UNIT_HEIGHT * 1              );
	g_ScrlUnitV .SetSize   ( UNIT_HEIGHT * 1              );
	g_ScrlKeyV  .SetSize   ( KEY_HEIGHT  * KEYNUM         );

	if_SubWindows_Initialize();

	if_Panel_init( file_profile );


	if_Projector_RedrawName( NULL );

	if( !if_BaseField_init( DEFAULT_BEATWIDTH, DEFAULT_DEVINUM, palette ) ) return false;

	if_WoiceField_Initialize(    );

	if( !if_UnitField_init  ( palette ) ) return false;
	if( !if_KeyField_init   ( palette ) ) return false;
	if( !if_Cursor_init     ( palette ) ) return false;
	if( !if_VolumeField_init( palette ) ) return false;
	if( !if_ScopeField_init ( palette ) ) return false;
	if( !if_WoiceTray_init  ( palette ) ) return false;

	if_InitVolumePanel(          );
	if( !if_PlayField_Initialize( palette ) ) return false;
	if_Keyboard_Initialize(      );
	if_Copier_init   ( hWnd, file_profile );
	if_Player_init   ( hWnd, file_profile );
	if_Projector_init( hWnd, file_profile );
	if_Effector_init ( hWnd, file_profile );
	if_Menu_Scope_Initialize(    );
	if_Menu_Play_Initialize(     );
	if_Menu_Unit_Initialize(     );
	if_Tenkey_Tuning_Initialize( );
	if_Menu_Woice_Initialize(    );
	if_Menu_Division_Initialize( );
	if_Menu_Zoom_Initialize(     );
	if_SelectNo_Woice_Initialize();
	if_SelectNo_Group_Initialize();

	if_Panel_Scale_Load(         );

	InitializeCriticalSection( &_critical ); _b_critical = true;
	_b_init = true;

	return true;
}

void Interface_release()
{
	if_Panel_Scale_Save  ();
	if_BaseField_Release ();
	if_SubWindows_Release();

	if( _b_critical )
	{
		EnterCriticalSection ( &_critical );
		_b_critical = false;
		DeleteCriticalSection( &_critical );
	}

}


void Interface_Process( HWND hWnd, bool bDraw )
{
	fRECT rcDummyLast     = {200,400,216,416}; // 一番右下
	fRECT rcDummyVolume   = {200,272,216,400}; // ボリュームの右
	fRECT rcDummy4        = {144, 88,160,136}; // スケールの右
	fRECT rcVolumeName    = {  0, 80,128,208}; // ボリューム

	fRECT rcKeySeparator1 = {368,160,512,168};
	fRECT rcKeySeparator2 = {368,288,384,296};

	float cur_x;
	float cur_y;
	fRECT rc_view;

	static bool _bKeyboardStart = false;
	static bool _bKeyboardEnd   = false;

	if( !_b_init ) return;

	if( !TryEnterCriticalSection( &_critical ) ) return;

	if( !_b_critical ) return;

	if_Effector_SetCursor( 0, 0 );

	KeyControl_UpdateTrigger();
	_GetMousePointer( hWnd, &cur_x, &cur_y );

	// fRECT ========================================================
	{
		RECT rc; GetClientRect( hWnd, &rc );
		rc_view.l = (float)rc.left  ;
		rc_view.t = (float)rc.top   ;
		rc_view.r = (float)rc.right  / g_dxdraw->get_screen_mag();
		rc_view.b = (float)rc.bottom / g_dxdraw->get_screen_mag();
	}
	g_dxdraw->SetViewport( rc_view.l, rc_view.t, rc_view.r, rc_view.b, 0, 0 );

	// 勝手にボリュームを閉じる
	if( MINVIEW_HEIGHT + VOLUME_HEIGHT > rc_view.w() ) if_SetUseVolume( false );

	// Make Rect ====================================================
	_SetRects( &rc_view );

	// Action =======================================================
	MidiInput_Procedure();

	if( if_ShortcutKey_Action( hWnd ) || if_Cursor_Action(    cur_x, cur_y ) ) bDraw = true;
	if(                                  if_BaseField_Action( cur_x, cur_y ) ) bDraw = true;

	// Put ==========================================================
	if( bDraw )
	{
		bool b_tex_reloaded = false;
		g_dxdraw->Begin( &b_tex_reloaded );
		if( b_tex_reloaded )
		{
			if_Projector_RedrawName   ( NULL );
			if_UnitTray_RedrawAllName ( NULL );
			if_WoiceTray_RedrawAllName( NULL );
		}

		// RECT作り直し
		_SetRects( &rc_view );

		// put
		if( if_ToolPanel_GetMode() == enum_ToolMode_W )
		{
			if_WoiceDummy_Put(        );
			if_WoiceField_Put(        );
			g_ScrlWoiceH. Put();
		}
		else
		{
			if_BaseField_Put_Field(   );
			if_PlayField_Put(         );
			if_BaseField_Put_Grid(    );
			if_ScopeField_Put(        );
			if_VolumeField_Put(       );
			g_ScrlEventH.Put();
		}

		if( if_ToolPanel_GetMode() == enum_ToolMode_K )
		{
			if_KeyField_PutField();
			if_KeyField_PutEvent();
			if_Keyboard_Put();
			g_dxdraw->tex_Put_View( rc_view.l,                    KEYSEPARATOR_Y, &rcKeySeparator1, SURF_FIELDS );
			g_dxdraw->tex_Put_View( rc_view.r - SCROLLBUTTONSIZE, KEYSEPARATOR_Y, &rcKeySeparator2, SURF_FIELDS );
		}

		if( if_ToolPanel_GetMode() == enum_ToolMode_W ) if_WoiceTray_Put();
		else                                            if_UnitTray_Put( );

		switch( if_ToolPanel_GetMode() ){
		case enum_ToolMode_W: if_WoiceField_Put(      ); break;
		case enum_ToolMode_U: if_UnitField_Put( true  ); break;
		case enum_ToolMode_K:
			if_UnitField_Put( false );
			break;
		}

		if_PutUnitEvent(     );
		if_Cursor_Put(       );
		if_PutVolumePanel(   );
		if_PutVolumeSwitch(  );

		if_Free_Put(         );
		if_Panel_Comment_Put();
		if_Panel_Logo_Put(   );
		if_Panel_Rack_Put(   );
		if_Panel_Scale_Put(  );

		if_ToolPanel_Put(    );

		if( if_VolumePanel_IsUse() && if_ToolPanel_GetMode() != enum_ToolMode_W )
		{
			g_dxdraw->tex_Put_View( rc_view.r - SCROLLBUTTONSIZE, rc_view.b - SCROLL_HEIGHT - VOLUME_HEIGHT, &rcDummyVolume, SURF_TABLES );
		}

		g_dxdraw->tex_Put_View( rc_view.r - SCROLLBUTTONSIZE, rc_view.t + HEADER_H        , &rcDummy4,    SURF_TABLES );
		g_dxdraw->tex_Put_View( rc_view.r - SCROLLBUTTONSIZE, rc_view.b - SCROLLBUTTONSIZE, &rcDummyLast, SURF_TABLES );

		if_SubWindows_Put();

		if_Menu_Play_Put(     );
		if_Menu_Scope_Put(    );
		if_Menu_Unit_Put(     );
		if_Menu_Woice_Put(    );
		if_Menu_Zoom_Put(     );
		if_Menu_Division_Put( );
		if_SelectNo_Woice_Put();
		if_SelectNo_Group_Put();
		if_Tenkey_Tuning_Put();

		// フレーム数
		fRECT    rcFps = {112, 96,136,104};
		uint32_t fps   = CountFramePerSecond();
		g_dxdraw->tex_Put_View( rc_view.r - 40 - 4,  2, &rcFps, SURF_DIALOGS );
		if_gen_num6( rc_view.r - 16 - 4,  2, fps, 2 );

#ifdef _DEBUG
		int32_t undo_index = UndoEvent_GetUndoIndex();
		int32_t redo_index = UndoEvent_GetRedoIndex();
		if_gen_num6( 0,16, undo_index, 6 );
		if_gen_num6( 0,24, redo_index, 6 );
#endif

		g_dxdraw->End( true );

	}

	LeaveCriticalSection( &_critical );

}

