
#include "../../Generic/KeyControl.h"

#include "Interface.h"
#include "if_Cursor.h"
#include "if_Effector.h"


extern HWND g_hWnd_Main;

enum DRAGMODE
{
	_DRAGMODE_FREE  = 0,
	_DRAGMODE_FRAME ,
	_DRAGMODE_SHUT  ,
	_DRAGMODE_PLAYED,
};

static bool _Action_Free( float cur_x, float cur_y )
{
	fRECT* p_field_rect;

	if( !if_Effector_IsOpen() ) return false;

	p_field_rect = if_Effector_GetRect();

	if( cur_x <  p_field_rect->l ||
		cur_x >= p_field_rect->r ||
		cur_y <  p_field_rect->t ||
		cur_y >= p_field_rect->b )
	{
		return false;
	}

	g_cursor.p_field_rect = p_field_rect;
	g_cursor.drag_xpos[0] = cur_x;
	g_cursor.drag_ypos    = cur_y;

	if_Effector_SetCursor( cur_x, cur_y );

	// 左クリック
	if(       KeyControl_IsClickLeftTrigger()  )
	{
		if_Effector_SetFocus();

		if( if_Effector_HitButton( cur_x, cur_y ) ) return true;

		g_cursor.focus  = ifCurFocus_Effector;

		if( if_Effector_ShutButton( cur_x, cur_y ) )
		{
			g_cursor.action = _DRAGMODE_SHUT;
		}
		if( if_Effector_HitPlayed( cur_x, cur_y, &g_cursor.bON ) )
		{
			g_cursor.action = _DRAGMODE_PLAYED;
		}
		else
		{
			if_Effector_GetPosition( &g_cursor.start_x, &g_cursor.start_y );
			g_cursor.action = _DRAGMODE_FRAME;
		}
	}

	return true;
}

static bool _Action_Drag_Shut( float cur_x, float cur_y )
{

	fRECT* p_field_rect;
	bool   bDrag = false;

	p_field_rect          = if_Effector_GetRect();
	g_cursor.focus        = ifCurFocus_Effector;
	g_cursor.p_field_rect = p_field_rect;

	//ドラッグキャンセル
	if( KeyControl_IsClickLeft() && KeyControl_IsClickRight() )
	{
		g_cursor.action = _DRAGMODE_FREE;
		return true;
	}

	// ドラッグ終了
	if( !KeyControl_IsClickLeft() ) g_cursor.action = _DRAGMODE_FREE;

	return true;
}

static bool _Action_Drag_Frame( float cur_x, float cur_y )
{

	fRECT* p_field_rect;
	bool   bDrag = false;

	p_field_rect          = if_Effector_GetRect();
	g_cursor.focus        = ifCurFocus_Effector;
	g_cursor.p_field_rect = p_field_rect;

	//ドラッグキャンセル
	if( KeyControl_IsClickLeft() && KeyControl_IsClickRight() )
	{
		g_cursor.action = _DRAGMODE_FREE;
		if_Effector_SetPosition( g_cursor.start_x, g_cursor.start_y );
		return true;
	}

	if_Effector_SetPosition(
		g_cursor.start_x + cur_x - g_cursor.drag_xpos[ 0 ],
		g_cursor.start_y + cur_y - g_cursor.drag_ypos );

	// ドラッグ終了
	if( !KeyControl_IsClickLeft() ) g_cursor.action = _DRAGMODE_FREE;

	return true;
}

static bool _Action_Drag_Played( float cur_x, float cur_y )
{
	// ドラッグ終了
	if( !KeyControl_IsClickLeft() || KeyControl_IsClickRight() )
	{
		g_cursor.action = _DRAGMODE_FREE;
		return true;
	}

	if_Effector_SetPlayed( cur_x, cur_y, g_cursor.bON ? true : false );

	return true;
}

// 音符編集カーソル
bool if_Cursor_Action_Effector( float cur_x, float cur_y )
{
	bool b_ret = false;

	switch( g_cursor.action )
	{
	case _DRAGMODE_FREE  : b_ret = _Action_Free(        cur_x, cur_y ); break;
	case _DRAGMODE_FRAME : b_ret = _Action_Drag_Frame(  cur_x, cur_y ); break;
	case _DRAGMODE_SHUT  : b_ret = _Action_Drag_Shut(   cur_x, cur_y ); break;
	case _DRAGMODE_PLAYED: b_ret = _Action_Drag_Played( cur_x, cur_y ); break;
	}

	return b_ret;
}

