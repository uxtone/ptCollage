
#include <pxtnPulse_Frequency.h>
extern pxtnPulse_Frequency *g_freq;

#include <pxtnService.h>
extern pxtnService *g_pxtn;

#include "../../Generic/KeyControl.h"
#include "../../Generic/if_gen_Scroll.h"

#include "../UndoEvent.h"
#include "../UnitFocus.h"
#include "../ActiveTone.h"

#include "Interface.h"
#include "if_Cursor.h"
#include "if_BaseField.h"
#include "if_Keyboard.h"
#include "if_KeyField.h"
#include "if_ToolPanel.h"

extern if_gen_Scroll    g_ScrlEventH;
extern if_gen_Scroll    g_ScrlKeyV;

static bool _Action_Free( float cur_x, float cur_y )
{
	int32_t divi_w  ;
	int32_t beat_w  ;
	int32_t offset_x;
	int32_t offset_y;
	int32_t target  ;

	fRECT*  p_field_rect;

	target = UnitFocus_Get();
	if( target < 0 ) return false;

	p_field_rect = if_KeyField_GetRect();

	if( cur_x <  p_field_rect->l ||
		cur_x >= p_field_rect->r ||
		cur_y <  p_field_rect->t ||
		cur_y >= p_field_rect->b ) return false;

	offset_y              = g_ScrlKeyV.GetOffset();
	g_cursor.focus        = ifCurFocus_KeyField;
	g_cursor.p_field_rect = p_field_rect;
	g_cursor.tgt_id       = target;
	g_cursor.drag_ypos    = ( (int32_t)( cur_y - p_field_rect->t  + offset_y ) / KEY_HEIGHT ) * KEY_HEIGHT;
	g_cursor.rect.t       = g_cursor.drag_ypos - offset_y + p_field_rect->t + KEY_HEIGHT/2;
	g_cursor.rect.b       = g_cursor.rect.t;

	offset_x              = g_ScrlEventH.GetOffset();
	divi_w                = if_BaseField_GetDivideWidth();
	beat_w                = if_BaseField_GetBeatWidth();
	g_cursor.drag_xpos[0] = ( (int32_t)( cur_x - p_field_rect->l + offset_x ) / divi_w ) * divi_w;
	g_cursor.drag_xpos[1] = g_cursor.drag_xpos[0] + divi_w;
	g_cursor.rect.l       = g_cursor.drag_xpos[0] + p_field_rect->l - offset_x;
	g_cursor.rect.r       = g_cursor.drag_xpos[1] + p_field_rect->l - offset_x;


	// 左クリック
	if(       KeyControl_IsClickLeftTrigger()  )
	{
		int32_t  key_index;
		float freq_rate;

		if( !g_pxtn->Unit_Get( g_cursor.tgt_id ) ) return true;
		key_index        = 87 - ( (int32_t)g_cursor.drag_ypos / KEY_HEIGHT );
		freq_rate        = g_freq->Get( ( key_index + 0x15 ) * 0x100 - EVENTDEFAULT_BASICKEY );
		if_Cursor_SetActiveTone_Unit( target, cur_x, freq_rate );
		g_cursor.action  = 1;
		g_cursor.scope   = enum_ScopeMode_Scope;
		g_cursor.start_x = cur_x;
		g_cursor.start_y = cur_y;
	}
	// 右クリック
	else if( KeyControl_IsClickRightTrigger() )
	{
		g_cursor.action  = 2;
		g_cursor.scope   = enum_ScopeMode_Scope;
		g_cursor.start_x = cur_x;
		g_cursor.start_y = cur_y;
	}

	return true;
}

static bool _Action_Drag( float cur_x, float cur_y, bool bDelete )
{
	int32_t clock1, clock2;

	if( if_Cursor_DragAction( cur_x, cur_y, &clock1, &clock2 ) )
	{
		int32_t       value;
		unsigned char u = (unsigned char)g_cursor.tgt_id;
		int32_t       count = 0;
		bool          bKeyOnly;

		if( !g_pxtn->Unit_Get( u ) ) return false;

		// アンドゥ
		UndoEvent_SetOrderClock( clock1, clock2 );
		UndoEvent_SetOrderUnit( u );
		UndoEvent_Push( true );

		bKeyOnly = if_ToolPanel_IsUseKeyOnly();

		if( !bDelete )
		{
			value = (unsigned short)( 0x6C00 + 0x6000 - ( g_cursor.drag_ypos / KEY_HEIGHT * 0x0100 ) - EVENTDEFAULT_BASICKEY ); //0x3900;

			if( bKeyOnly )
			{
				count += g_pxtn->evels->Record_Value_Set( clock1, clock2, u, EVENTKIND_KEY, value );
				count += g_pxtn->evels->Record_Add_i    ( clock1,         u, EVENTKIND_KEY, value );
			}
			else
			{
				count += g_pxtn->evels->Record_Delete( clock1, clock2, u, EVENTKIND_KEY      );
				count += g_pxtn->evels->Record_Add_i ( clock1,         u, EVENTKIND_KEY     , value );
				count += g_pxtn->evels->Record_Delete( clock1, clock2, u, EVENTKIND_VELOCITY );
				count += g_pxtn->evels->Record_Add_i ( clock1,         u, EVENTKIND_VELOCITY, (int32_t)EVENTDEFAULT_VELOCITY );
				count += g_pxtn->evels->Record_Delete( clock1, clock2, u, EVENTKIND_ON       );
				count += g_pxtn->evels->Record_Add_i ( clock1,         u, EVENTKIND_ON      , clock2 - clock1 );
			}
			g_pxtn->master->AdjustMeasNum( clock2 );

		}
		else
		{
			{
				count += g_pxtn->evels->Record_Delete( clock1, clock2, u, EVENTKIND_KEY      );
			}
			if( !bKeyOnly )
			{
				count += g_pxtn->evels->Record_Delete( clock1, clock2, u, EVENTKIND_ON       );
				count += g_pxtn->evels->Record_Delete( clock1, clock2, u, EVENTKIND_VELOCITY );
			}
		}
		if( count < 1 ) UndoEvent_Back();
		else            UndoEvent_ReleaseRedo();
	}

	return true;
}

// 音符編集カーソル
bool if_Cursor_Action_KeyField( float cur_x, float cur_y )
{
	bool b_ret = false;

	switch( g_cursor.action )
	{
	case 0: b_ret = _Action_Free( cur_x, cur_y        ); break;
	case 1: b_ret = _Action_Drag( cur_x, cur_y, false ); break;
	case 2: b_ret = _Action_Drag( cur_x, cur_y, true  ); break;
	}

	fRECT *p_field_rect;
	p_field_rect     = if_KeyField_GetRect();
	if( g_cursor.rect.l < p_field_rect->l ) g_cursor.rect.l = p_field_rect->l;
	if( g_cursor.rect.r > p_field_rect->r ) g_cursor.rect.r = p_field_rect->r;

	return b_ret;
}
