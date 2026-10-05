
#include <pxtnService.h>
extern pxtnService*  g_pxtn    ;

#include <pxtonewinXA2.h>
extern pxtonewinXA2* g_strm_xa2;

#include "../../Generic/KeyControl.h"
#include "../../Generic/if_gen_Scroll.h"

#include "../WoiceFocus.h"
#include "../Woice.h"

#include "Interface.h"
#include "if_Cursor.h"
#include "if_WoiceTray.h"
#include "if_Menu_Woice.h"
#include "if_ToolPanel.h"

extern if_gen_Scroll g_ScrlWoiceH;
extern if_gen_Scroll g_ScrlWoiceV;
extern HWND       g_hWnd_Main;

static bool _ActionFree( float cur_x, float cur_y )
{
	int32_t  num          =    0;
	int32_t  w            =    0;
	int32_t  offset       =    0;
	fRECT*   p_field_rect = NULL;

	// カーソルキー
	if( KeyControl_IsKeyTrigger( VK_UP ) || KeyControl_IsKeyTrigger( VK_DOWN ) )
	{
		WoiceFocus_Shift( KeyControl_IsKeyTrigger( VK_DOWN ) );
		if_WoiceTray_JustScroll();
		w               = WoiceFocus_Get(); if( w < 0 ) return false;
		g_cursor.action = 4;
		g_cursor.focus  = ifCurFocus_WoiceTray;
		g_cursor.tgt_id = w;
		if( !g_strm_xa2->tune_is_sampling() ) if_Cursor_SetActiveTone_Woice( w );
		return true;
	}

	p_field_rect = if_WoiceTray_GetRect();
	if( g_cursor.action == 0 )
	{
		if( cur_x <  p_field_rect->l + 16 ||
			cur_x >= p_field_rect->r      ||
			cur_y <  p_field_rect->t      ||
			cur_y >= p_field_rect->b  ) return false;
	}

	offset = g_ScrlWoiceV.GetOffset();
	num    = g_pxtn->Woice_Num();

	// どのユニットか
	for( w = 0; w < num; w++ )
	{
		if( cur_y >= p_field_rect->t + w * WOICE_HEIGHT                - offset &&
			cur_y <  p_field_rect->t + w * WOICE_HEIGHT + WOICE_HEIGHT - offset ) break;
	}
	if( w == num ) return false;

	g_cursor.drag_idy = (int32_t)( ( cur_y - p_field_rect->t + offset ) / WOICE_HEIGHT );
	g_cursor.rect.l   = p_field_rect->l + 16;
	g_cursor.rect.t   = p_field_rect->t + g_cursor.drag_idy * WOICE_HEIGHT - offset;

	if( KeyControl_IsClickLeftTrigger() )
	{
		if( if_ToolPanel_GetMode() == enum_ToolMode_W )
		{
			g_cursor.start_x    = cur_x;
			g_cursor.start_y    = cur_y;
			g_cursor.focus      = ifCurFocus_WoiceTray;

			WoiceFocus_Set( w );
			if( !g_strm_xa2->tune_is_sampling() )
			{
				if_Cursor_SetActiveTone_Woice( w );
				g_cursor.tgt_id       = w;
				g_cursor.p_field_rect = p_field_rect;
				g_cursor.action       = 1;
			}
		}

		// ユニットメニュー
	}
	else if( KeyControl_IsClickRightTrigger() )
	{
		// 点灯
		g_cursor.p_field_rect = p_field_rect;
		g_cursor.focus        = ifCurFocus_WoiceTray;
		g_cursor.action       = 5;
		g_cursor.tgt_id       = w;
		WoiceFocus_Set( w );
	}
	else
	{
		return false;
	}

	return true;
}


static bool _ActionDrag_Replace( float cur_x, float cur_y )
{
	if( KeyControl_IsClickRight() )
	{
		if_Cursor_StopActiveTone();
		g_cursor.action   = 0;
		return true;
	}

	if( KeyControl_IsClickLeft() )
	{
		fRECT*  p_field_rect = g_cursor.p_field_rect;
		int32_t max_ypos     = g_pxtn->Woice_Num() - 1;
		int32_t offset       = g_ScrlWoiceV.GetOffset();

		g_cursor.drag_idy = (int32_t)( ( cur_y - p_field_rect->t + offset ) / WOICE_HEIGHT );
		if( g_cursor.drag_idy <  0       ) g_cursor.drag_idy = 0;
		if( g_cursor.drag_idy > max_ypos ) g_cursor.drag_idy = max_ypos;

		g_cursor.rect.l = p_field_rect->l + 16;
		g_cursor.rect.t = p_field_rect->t + g_cursor.drag_idy * WOICE_HEIGHT - offset;

		if_Cursor_DragScroll( p_field_rect->t, p_field_rect->b, cur_y, &g_ScrlWoiceV );
	}
	// ドラッグ終了
	else
	{
		g_cursor.action   = 0;
		if_Cursor_StopActiveTone();
		Woice_Replace( g_cursor.tgt_id, g_cursor.drag_idy );
	}

	return true;
}


static bool _ActionDrag_Shift()
{
	if( !KeyControl_IsKey( VK_UP ) && !KeyControl_IsKey( VK_DOWN ) )
	{
		if_Cursor_StopActiveTone();
		g_cursor.action = 0;
	}
	return true;
}

static bool _ActionDrag_CallMenu( float cur_x, float cur_y )
{
	float x, y;

	if( KeyControl_IsClickLeft() && KeyControl_IsClickRight() )
	{
		g_cursor.action = 0;
		return true;
	}
	if( !KeyControl_IsClickRight() )
	{
		if( if_WoiceTray_GetNamePosition( g_cursor.tgt_id, &x, &y ) )
		{
			if_Menu_Woice_SetActive( true );
			if_Menu_Woice_SetPosition( x, y, g_cursor.tgt_id );
		}
		g_cursor.action = 0;
	}

	return true;
}



// ユニット編集カーソル
bool if_Cursor_Action_WoiceTray( float cur_x, float cur_y )
{
	bool b_ret = false;

	switch( g_cursor.action )
	{
	case 0: b_ret = _ActionFree(          cur_x, cur_y ); break;
	case 1: b_ret = _ActionDrag_Replace(  cur_x, cur_y ); break;
	case 4: b_ret = _ActionDrag_Shift(                 ); break;
	case 5: b_ret = _ActionDrag_CallMenu( cur_x, cur_y ); break;
	}

	return b_ret;

}
