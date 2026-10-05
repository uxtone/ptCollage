
#include <pxtnService.h>
extern pxtnService *g_pxtn;

#include "../../Generic/KeyControl.h"
#include "../../Generic/if_gen_Scroll.h"

#include "../UndoEvent.h"
#include "../UnitFocus.h"

#include "Interface.h"
#include "if_Cursor.h"
#include "if_BaseField.h"
#include "if_VolumePanel.h"
#include "if_VolumeField.h"
#include "if_SelectNo_Woice.h"
#include "if_SelectNo_Group.h"
#include "if_Tenkey_Tuning.h"

extern if_gen_Scroll    g_ScrlEventH;

static bool _Action_Free( float cur_x, float cur_y )
{
	int32_t divi_w;
	int32_t beat_w;
	float   offset_x;
	int32_t target = UnitFocus_Get();
	int32_t grid   = 2;

	fRECT*  p_field_rect;

	if( target < 0 ) return false;

	p_field_rect     = if_VolumeField_GetRect();

	if( cur_x <  p_field_rect->l ||
		cur_x >= p_field_rect->r ||
		cur_y <  p_field_rect->t ||
		cur_y >= p_field_rect->b ) return false;

	if( KeyControl_IsKey( VK_CONTROL ) ) grid = 8;

	g_cursor.focus        = ifCurFocus_VolumeField;
	g_cursor.p_field_rect = p_field_rect;
	g_cursor.tgt_id       = target;
	g_cursor.drag_ypos    = p_field_rect->t + (int32_t)(cur_y + grid/2 - p_field_rect->t) / grid * grid;
	g_cursor.rect.t       = g_cursor.drag_ypos;
	g_cursor.rect.b       = g_cursor.drag_ypos;

	offset_x              = (float)g_ScrlEventH.GetOffset();
	divi_w                = if_BaseField_GetDivideWidth();
	beat_w                = if_BaseField_GetBeatWidth();
	g_cursor.drag_xpos[0] = ( (int32_t)( cur_x - p_field_rect->l + offset_x ) / divi_w ) * divi_w;
	g_cursor.drag_xpos[1] = g_cursor.drag_xpos[0] + divi_w;
	g_cursor.rect.l       = g_cursor.drag_xpos[0] + p_field_rect->l - offset_x;
	g_cursor.rect.r       = g_cursor.drag_xpos[1] + p_field_rect->l - offset_x;

	if(       KeyControl_IsClickLeftTrigger()  )
	{
		g_cursor.action  = 1;

		switch( if_GetVolumeMode() )
		{
		case VOLUMEMODE_Velocity :
		case VOLUMEMODE_Volume   :
		case VOLUMEMODE_Pan_Volume   :
		case VOLUMEMODE_Pan_Time  :
			g_cursor.scope   = enum_ScopeMode_Scope;
			break;
		case VOLUMEMODE_Portament:
		case VOLUMEMODE_GroupNo  :
		case VOLUMEMODE_VoiceNo  :
		case VOLUMEMODE_Tuning   :
			g_cursor.scope   = enum_ScopeMode_Point;
			break;
		}

		g_cursor.start_x = cur_x;
		g_cursor.start_y = cur_y;

		int32_t volume;
		volume = (unsigned short)( p_field_rect->b - g_cursor.drag_ypos );
		switch( if_GetVolumeMode() )
		{
		case VOLUMEMODE_Velocity  :
		case VOLUMEMODE_Volume    : if_Cursor_SetActiveTone_Unit( target, cur_x, -1, -1, volume, -1 ); break;
		case VOLUMEMODE_Pan_Volume: if_Cursor_SetActiveTone_Unit( target, cur_x, -1, volume, -1, -1 ); break;
		case VOLUMEMODE_Pan_Time  : if_Cursor_SetActiveTone_Unit( target, cur_x, -1, -1, -1, volume ); break;
		case VOLUMEMODE_Portament : break;
		case VOLUMEMODE_VoiceNo   : if_SelectNo_Woice_Start( g_cursor.rect.l, cur_y ); break;
		case VOLUMEMODE_GroupNo   : if_SelectNo_Group_Start( g_cursor.rect.l, cur_y ); break;
		case VOLUMEMODE_Tuning    : break;
		}
	}
	else if( KeyControl_IsClickRightTrigger() )
	{
		if( if_GetVolumeMode() != VOLUMEMODE_Velocity ){ }
		g_cursor.action  = 2;
		g_cursor.scope   = enum_ScopeMode_Scope;
		g_cursor.start_x = cur_x;
		g_cursor.start_y = cur_y;
	}

	return true;
}

static bool _Action_Drag( float cur_x, float cur_y, bool bDelete )
{
	int32_t clock1, clock2, grid = 2;

	if_SelectNo_Woice_Update( cur_x, cur_y );
	if_SelectNo_Group_Update( cur_y        );

	if( KeyControl_IsKey( VK_CONTROL ) ) grid = 8;

	if( if_Cursor_DragAction( cur_x, cur_y, &clock1, &clock2 ) )
	{
		int32_t        value;
		int32_t        count = 0;
		unsigned char  u     = (unsigned char)g_cursor.tgt_id;

		// アンドゥ
		UndoEvent_SetOrderClock( clock1, clock2 );
		UndoEvent_SetOrderUnit(      u     );
		UndoEvent_Push( true );

		value = (unsigned short)( g_cursor.p_field_rect->b - g_cursor.drag_ypos );

		switch( if_GetVolumeMode() )
		{
		case VOLUMEMODE_Velocity:// ベロシティ
		{
			if( value > 128 ) value = 128;
			count += g_pxtn->evels->Record_Value_Set( clock1, clock2, u, EVENTKIND_VELOCITY, value );
		}
		break;

		case VOLUMEMODE_Pan_Volume:// パン
			if( bDelete )
			{
				count += g_pxtn->evels->Record_Delete( clock1, clock2, u, EVENTKIND_PAN_VOLUME );
			}
			else
			{
				if( value > 128 ) value = 128;
				count += g_pxtn->evels->Record_Add_i    ( clock1,         u, EVENTKIND_PAN_VOLUME, value );
				count += g_pxtn->evels->Record_Value_Set( clock1, clock2, u, EVENTKIND_PAN_VOLUME, value );
				g_pxtn->master->AdjustMeasNum( clock2 );
			}
			break;

		case VOLUMEMODE_Pan_Time:// パン
			if( bDelete )
			{
				count += g_pxtn->evels->Record_Delete( clock1, clock2, u, EVENTKIND_PAN_TIME );
			}
			else
			{
				if( value > 128 ) value = 128;
				count += g_pxtn->evels->Record_Add_i    ( clock1,         u, EVENTKIND_PAN_TIME, value );
				count += g_pxtn->evels->Record_Value_Set( clock1, clock2, u, EVENTKIND_PAN_TIME, value );
				g_pxtn->master->AdjustMeasNum( clock2 );
			}
			break;

		case VOLUMEMODE_Volume:// ボリューム
			if( bDelete ){
				count += g_pxtn->evels->Record_Delete( clock1, clock2, u, EVENTKIND_VOLUME );
			}else{
				if( value > 128 ) value = 128;
				count += g_pxtn->evels->Record_Add_i    ( clock1,         u, EVENTKIND_VOLUME, value );
				count += g_pxtn->evels->Record_Value_Set( clock1, clock2, u, EVENTKIND_VOLUME, value );
				g_pxtn->master->AdjustMeasNum( clock2 );
			}
			break;
		case VOLUMEMODE_Portament:// ポルタメント
			if( bDelete ){
				count += g_pxtn->evels->Record_Delete( clock1, clock2, u, EVENTKIND_PORTAMENT );
			}else{
				count += g_pxtn->evels->Record_Add_i ( clock1,         u, EVENTKIND_PORTAMENT, clock2 - clock1 );
				g_pxtn->master->AdjustMeasNum( clock2 );
			}
			break;
		case VOLUMEMODE_VoiceNo:// ボイスNO
			if( bDelete )
			{
				count += g_pxtn->evels->Record_Delete( clock1, clock2, u, EVENTKIND_VOICENO );
			}
			else
			{
				value = if_SelectNo_Woice_End( cur_y );
				count += g_pxtn->evels->Record_Add_i(    clock1,         u, EVENTKIND_VOICENO, value );
				g_pxtn->master->AdjustMeasNum( clock1 + 1 );
			}
			break;
		case VOLUMEMODE_GroupNo:// グループNO
			if( bDelete )
			{
				count += g_pxtn->evels->Record_Delete( clock1, clock2, u, EVENTKIND_GROUPNO );
			}else{
				value = if_SelectNo_Group_End( cur_y );
				count += g_pxtn->evels->Record_Add_i(    clock1,         u, EVENTKIND_GROUPNO, value );
				g_pxtn->master->AdjustMeasNum( clock1 + 1 );
			}
			break;
		case VOLUMEMODE_Tuning:// 補正値
			if( bDelete ){
				count += g_pxtn->evels->Record_Delete( clock1, clock2, u, EVENTKIND_TUNING );
			}else{
				if_Tenkey_Tuning_SetActive( cur_x, cur_y, g_cursor.tgt_id, clock1 );
			}
			break;
		}

		if( count < 1 ) UndoEvent_Back();
		else            UndoEvent_ReleaseRedo();

	}
	else
	{
		fRECT*  p_field_rect;
		int32_t min_y, max_y;
		p_field_rect = g_cursor.p_field_rect;
		g_cursor.drag_ypos    = (float)  ( p_field_rect->t + (int32_t)( cur_y               + grid/2 - p_field_rect->t ) / grid * grid );
		min_y                 = (int32_t)( p_field_rect->t + (int32_t)( p_field_rect->t     + grid/2 - p_field_rect->t ) / grid * grid );
		max_y                 = (int32_t)( p_field_rect->t + (int32_t)( p_field_rect->b - 1 + grid/2 - p_field_rect->t ) / grid * grid );
		if( g_cursor.drag_ypos < min_y ) g_cursor.drag_ypos = (float)min_y;
		if( g_cursor.drag_ypos > max_y ) g_cursor.drag_ypos = (float)max_y;
		g_cursor.rect.t       = g_cursor.drag_ypos;
		g_cursor.rect.b       = g_cursor.drag_ypos;
	}
	return true;
}



// ボリューム編集カーソル
bool if_Cursor_Action_VolumeField( float cur_x, float cur_y )
{
	bool b_ret = false;

	switch( g_cursor.action )
	{
	// free --------
	case 0: b_ret = _Action_Free( cur_x, cur_y        ); break;
	case 1: b_ret = _Action_Drag( cur_x, cur_y, false ); break;
	case 2: b_ret = _Action_Drag( cur_x, cur_y, true  ); break;
	}

	fRECT* p_field_rect;
	p_field_rect = if_VolumeField_GetRect();
	if( g_cursor.rect.l < p_field_rect->l ) g_cursor.rect.l = p_field_rect->l;
	if( g_cursor.rect.r > p_field_rect->r ) g_cursor.rect.r = p_field_rect->r;

	return b_ret;
}


