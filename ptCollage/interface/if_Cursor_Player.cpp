
#include "../../Generic/KeyControl.h"

#include "Interface.h"
#include "if_Cursor.h"
#include "if_Player.h"


extern HWND g_hWnd_Main;

enum enum_DragMode
{
	enum_DragMode_Frame =  0,
	enum_DragMode_PlayButton,
	enum_DragMode_SpaceKey  ,
	enum_DragMode_Button    ,
	enum_DragMode_Volume    ,
};

static enum_DragMode _drag_mode;


static bool _Action_Free( float cur_x, float cur_y )
{
	if( !if_Player_IsOpen() ) return false;

	fRECT* p_field_rect = if_Player_GetRect();

	if( cur_x <  p_field_rect->l ||
		cur_x >= p_field_rect->r ||
		cur_y <  p_field_rect->t ||
		cur_y >= p_field_rect->b ) return false;

	g_cursor.p_field_rect = p_field_rect;
	g_cursor.drag_xpos[0] = cur_x;
	g_cursor.drag_ypos    = cur_y;


	// 左クリック
	if(       KeyControl_IsClickLeftTrigger()  )
	{
		if_Player_SetFocus();

		g_cursor.action = 1;
		g_cursor.focus  = ifCurFocus_Player;

		if( if_Player_IsPlayButton( cur_x, cur_y ) )
		{
			switch(  if_Player_GetPlayButtonAnime() )
			{
			case 0:  if_Player_SetPlayButtonAnime( 1 ); break;
			default: if_Player_SetPlayButtonAnime( 3 ); break;
			}
			_drag_mode = enum_DragMode_PlayButton;
		}
		else if( if_Player_IsVolume( cur_x, cur_y ) )
		{
			_drag_mode = enum_DragMode_Volume;
			if_Player_SetVolume_cur_pos( cur_x, cur_y );
		}
		else if( if_Player_CheckButton( cur_x, cur_y ) )
		{
			_drag_mode = enum_DragMode_Button;
		}
		else
		{
			if_Player_GetPosition( &g_cursor.start_x, &g_cursor.start_y );
			_drag_mode = enum_DragMode_Frame;
		}
	}

	return true;
}

static bool _Action_Drag( float cur_x, float cur_y )
{

	fRECT* p_field_rect;
	bool   bDrag = false;

	p_field_rect          = if_Player_GetRect();
	g_cursor.focus        = ifCurFocus_Player;
	g_cursor.p_field_rect = p_field_rect;

	//ドラッグキャンセル
	if( KeyControl_IsClickLeft() && KeyControl_IsClickRight() )
	{
		g_cursor.action = 0;

		switch( _drag_mode )
		{
		case enum_DragMode_Frame:
			if_Player_SetPosition( g_cursor.start_x, g_cursor.start_y );
			break;
		case enum_DragMode_PlayButton:
		case enum_DragMode_SpaceKey:
			if( if_Player_GetPlayButtonAnime() == 1 ) if_Player_SetPlayButtonAnime( 0 );
			else                                      if_Player_SetPlayButtonAnime( 2 );
			break;
		}
		return true;
	}

	switch( _drag_mode )
	{
	case enum_DragMode_Frame     :
	case enum_DragMode_Volume    :
	case enum_DragMode_PlayButton:
	case enum_DragMode_Button    :
		// ドラッグ中
		if( KeyControl_IsClickLeft() )
		{
			switch( _drag_mode )
			{
			case enum_DragMode_Frame:
				if_Player_SetPosition(
					g_cursor.start_x + cur_x - g_cursor.drag_xpos[0],
					g_cursor.start_y + cur_y - g_cursor.drag_ypos );
				break;
			case enum_DragMode_Volume:
				if_Player_SetVolume_cur_pos( cur_x, cur_y );
				break;
			}
		}
		else
		{
			g_cursor.action = 0;

			switch( _drag_mode )
			{
			case enum_DragMode_Frame:
				if_Player_SetPosition(
					g_cursor.start_x + cur_x - g_cursor.drag_xpos[0],
					g_cursor.start_y + cur_y - g_cursor.drag_ypos );
				break;

			case enum_DragMode_PlayButton:
				if( if_Player_GetPlayButtonAnime() == 1 ) if_Player_StartPlay();
				else                                      if_Player_StopPlay ();
				break;
			}
		}
		break;
	case enum_DragMode_SpaceKey:
		if( !KeyControl_IsKey( VK_SPACE ) )
		{
			g_cursor.action = 0;
			if( if_Player_GetPlayButtonAnime() == 1 ) if_Player_StartPlay();
			else                                      if_Player_StopPlay( );
		}
		break;
	}


	return true;
}

// 音符編集カーソル
bool if_Cursor_Action_Player( float cur_x, float cur_y )
{
	bool b_ret = false;

	switch( g_cursor.action )
	{
	case 0: b_ret = _Action_Free( cur_x, cur_y ); break;
	case 1: b_ret = _Action_Drag( cur_x, cur_y ); break;
	}

	return b_ret;
}

bool if_Cursor_Action_Player_Priority()
{
	if( KeyControl_IsKeyTrigger( VK_SPACE ) )
	{
		g_cursor.action = 1;
		g_cursor.focus  = ifCurFocus_Player;
		switch( if_Player_GetPlayButtonAnime() )
		{
		case 0:  if_Player_SetPlayButtonAnime( 1 ); break;
		default: if_Player_SetPlayButtonAnime( 3 ); break;
		}
		_drag_mode = enum_DragMode_SpaceKey;
		return true;
	}
	return false;
}