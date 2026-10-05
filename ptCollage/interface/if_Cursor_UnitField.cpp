
#include <pxtnService.h>
extern pxtnService *g_pxtn;

#include "../../Generic/KeyControl.h"
#include "../../Generic/if_gen_Scroll.h"

#include "../UndoEvent.h"
#include "../ActiveTone.h"

#include "Interface.h"
#include "if_Cursor.h"
#include "if_BaseField.h"
#include "if_UnitField.h"

#include "if_ToolPanel.h"

extern if_gen_Scroll    g_ScrlEventH;
extern if_gen_Scroll    g_ScrlUnitV;


static bool _Action_Free( float cur_x, float cur_y )
{
	int32_t divi_w;
	int32_t beat_w;
	int32_t offset_x;
	int32_t offset_y;

	int32_t unit_num;
	int32_t u;
	fRECT*  p_field_rect;

	p_field_rect = if_UnitField_GetRect();

	if( cur_x <  p_field_rect->l ||
		cur_x >= p_field_rect->r ||
		cur_y <  p_field_rect->t ||
		cur_y >= p_field_rect->b ) return false;

	offset_y     = g_ScrlUnitV.GetOffset();
	unit_num     = g_pxtn->Unit_Num();

	// どのユニットか
	for( u = 0; u < unit_num; u++ )
	{
		if( g_pxtn->Unit_Get( u )                                               &&
			cur_y >= p_field_rect->t + u * UNIT_HEIGHT               - offset_y &&
			cur_y <  p_field_rect->t + u * UNIT_HEIGHT + UNIT_HEIGHT - offset_y ) break;
	}
	if( u == unit_num )
	{
		g_cursor.focus = ifCurFocus_None;
		return true;
	}

	g_cursor.focus        = ifCurFocus_UnitField;
	g_cursor.p_field_rect = p_field_rect;
	g_cursor.tgt_id       = u;
	g_cursor.drag_ypos    = 0;
	g_cursor.rect.t       = p_field_rect->t + ( u * UNIT_HEIGHT ) - offset_y + UNIT_HEIGHT / 2;
	g_cursor.rect.b       = g_cursor.rect.t;

	beat_w                = if_BaseField_GetBeatWidth();
	divi_w                = if_BaseField_GetDivideWidth();
	offset_x              = g_ScrlEventH.GetOffset();
	g_cursor.drag_xpos[0] = ( (int32_t)( cur_x - p_field_rect->l + offset_x ) / divi_w ) * divi_w;
	g_cursor.drag_xpos[1] = g_cursor.drag_xpos[0] + divi_w;
	g_cursor.rect.l       = g_cursor.drag_xpos[0] + p_field_rect->l - offset_x;
	g_cursor.rect.r       = g_cursor.drag_xpos[1] + p_field_rect->l - offset_x;

	if(       KeyControl_IsClickLeftTrigger() )
	{
		g_cursor.action  = 1;
		g_cursor.scope   = enum_ScopeMode_Scope;
		g_cursor.start_x = cur_x;
		g_cursor.start_y = cur_y;
		if( if_ToolPanel_GetMode() == enum_ToolMode_U )
		{
			if_Cursor_SetActiveTone_Unit( u, cur_x );
		}
	}
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

	if( if_Cursor_DragAction( cur_x, cur_y, &clock1, &clock2 ) ){

		int32_t           count = 0;
		unsigned char  u = (unsigned char)g_cursor.tgt_id;

		// アンドゥ
		UndoEvent_SetOrderClock( clock1, clock2 );
		UndoEvent_SetOrderUnit( u          );
		UndoEvent_Push( true );

		count += g_pxtn->evels->Record_Delete( clock1, clock2, u, EVENTKIND_ON       );
		count += g_pxtn->evels->Record_Delete( clock1, clock2, u, EVENTKIND_VELOCITY );
		if( bDelete )
		{
			count += g_pxtn->evels->Record_Delete( clock1, clock2, u, EVENTKIND_KEY );
		}
		else
		{
			count += g_pxtn->evels->Record_Add_i( clock1, u, EVENTKIND_VELOCITY, (int32_t)EVENTDEFAULT_VELOCITY );
			count += g_pxtn->evels->Record_Add_i( clock1, u, EVENTKIND_ON      , clock2 - clock1             );

			g_pxtn->master->AdjustMeasNum( clock2 );
		}

		if( count < 1 ) UndoEvent_Back(       );
		else            UndoEvent_ReleaseRedo();
	}

	return true;
}


// 音符編集カーソル
bool if_Cursor_Action_UnitField( float cur_x, float cur_y )
{
	bool b_ret = false;

	switch( g_cursor.action ){
	// フリー --------
	case 0: b_ret =_Action_Free( cur_x, cur_y        ); break;
	case 1: b_ret =_Action_Drag( cur_x, cur_y, false ); break;
	case 2: b_ret =_Action_Drag( cur_x, cur_y, true  ); break;
	}

	fRECT* p_field_rect = if_UnitField_GetRect();
	if( g_cursor.rect.l < p_field_rect->l ) g_cursor.rect.l = p_field_rect->l;
	if( g_cursor.rect.r > p_field_rect->r ) g_cursor.rect.r = p_field_rect->r;

	return b_ret;
}
