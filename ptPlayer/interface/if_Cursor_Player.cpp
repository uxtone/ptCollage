
#include <pxStdDef.h>

#include "../../Generic/KeyControl.h"

#include "../Tune.h"

#include "Interface.h"
#include "if_Cursor.h"
#include "if_Player.h"

extern HWND g_hWnd_Main;

enum enum_DragMode
{
	enum_DragMode_PlayButton = 0,
	enum_DragMode_SpaceKey,
	enum_DragMode_LoopButton,
	enum_DragMode_FileButton,
	enum_DragMode_CommButton,
	enum_DragMode_Volume    ,
};

static enum_DragMode _drag_mode;


static bool _Action_Free( float cur_x, float cur_y )
{
	fRECT* p_field_rect;

	if( KeyControl_IsKeyTrigger( VK_SPACE ) ){
		g_cursor.action = 1;
		g_cursor.focus  = ifCurFocus_Player;
		switch( if_Player_GetPlayButtonAnime() ){
		case 0:  if_Player_SetPlayButtonAnime( 1 ); break;
		default: if_Player_SetPlayButtonAnime( 3 ); break;
		}
		_drag_mode = enum_DragMode_SpaceKey;
		return true;
	}

	p_field_rect = if_Player_GetRect();

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
		if( if_Player_IsPlayButton( cur_x, cur_y ) )
		{
			g_cursor.action = 1;
			g_cursor.focus  = ifCurFocus_Player;
			switch(  if_Player_GetPlayButtonAnime() ){
			case 0:  if_Player_SetPlayButtonAnime( 1 ); break;
			default: if_Player_SetPlayButtonAnime( 3 ); break;
			}
			_drag_mode = enum_DragMode_PlayButton;
		}
		else if( if_Player_IsFileButton( cur_x, cur_y ) )
		{
			g_cursor.action = 1;
			g_cursor.focus  = ifCurFocus_Player;
			if_Player_SetFileButtonAnime( 1 );
			_drag_mode = enum_DragMode_FileButton;
		}
		else if( Tune_IsComment() && if_Player_IsCommButton( cur_x, cur_y ) )
		{
			g_cursor.action = 1;
			g_cursor.focus  = ifCurFocus_Player;
			if_Player_SetCommButtonAnime( 1 );
			_drag_mode = enum_DragMode_CommButton;
		}
		else if( if_Player_IsStatusButton( cur_x, cur_y ) & IF_PLAYER_STATUS_LOOP )
		{
			g_cursor.action = 1;
			g_cursor.focus  = ifCurFocus_Player;
			if_Player_PushOneStatusFlag( IF_PLAYER_STATUS_LOOP );
			_drag_mode = enum_DragMode_LoopButton;
		}
		else if( if_Player_IsVolume( cur_x, cur_y ) )
		{
			g_cursor.action = 1;
			g_cursor.focus  = ifCurFocus_Player;
			if_Player_SetVolume_cur_pos( cur_x, cur_y );
			_drag_mode = enum_DragMode_Volume;
		}

	}else{
		return false;
	}

	return true;
}

static bool _Action_Drag( float cur_x, float cur_y, PTP_SIGN *p_ptp_sign )
{
	g_cursor.focus      = ifCurFocus_Player;


	//ドラッグキャンセル
	if( KeyControl_IsClickLeft() && KeyControl_IsClickRight() )
	{
		g_cursor.action = 0;

		switch( _drag_mode )
		{
		case enum_DragMode_FileButton: if_Player_SetFileButtonAnime( 0 ); break;
		case enum_DragMode_CommButton: if_Player_SetCommButtonAnime( 0 ); break;
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
	case enum_DragMode_Volume: if_Player_SetVolume_cur_pos( cur_x, cur_y ); break;
	}

	switch( _drag_mode )
	{
	case enum_DragMode_PlayButton:
	case enum_DragMode_FileButton:
	case enum_DragMode_LoopButton:
	case enum_DragMode_CommButton:
	case enum_DragMode_Volume    :

		// ドラッグ終了
		if( !KeyControl_IsClickLeft() )
		{
			g_cursor.action = 0;

			switch( _drag_mode )
			{
			case enum_DragMode_PlayButton:
				if( if_Player_GetPlayButtonAnime() == 1 ) if_Player_StartPlay();
				else                                      if_Player_StopPlay( );
				break;
			case enum_DragMode_FileButton:
//				Tune_SelectAndPlay( g_hWnd_Main );
				*p_ptp_sign = PTP_SIGN_SelectFile;
				if_Player_SetFileButtonAnime( 0 );
				KeyControl_Clear();
				break;
			case enum_DragMode_CommButton:
				Tune_PutComment( g_hWnd_Main );
				if_Player_SetCommButtonAnime( 0 );
				KeyControl_Clear();
				break;
			case enum_DragMode_LoopButton:
				break;

			case enum_DragMode_Volume:
				if_Player_SetVolume_cur_pos( cur_x, cur_y );
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
bool if_Cursor_Action_Player( float cur_x, float cur_y, PTP_SIGN *p_ptp_sign )
{
	bool b_ret = false;

	switch( g_cursor.action )
	{
	case 0: b_ret = _Action_Free( cur_x, cur_y ); break;
	case 1: b_ret = _Action_Drag( cur_x, cur_y, p_ptp_sign ); break;
	}

	return b_ret;
}
