
#include <pxtnService.h>
extern pxtnService *g_pxtn;

#include "../../Generic/KeyControl.h"
#include "../../Generic/if_gen_Scroll.h"

#include "../UndoEvent.h"

#include "Interface.h"
#include "if_Cursor.h"
#include "if_BaseField.h"
#include "if_PlayField.h"
#include "if_Menu_Play.h"

extern if_gen_Scroll    g_ScrlEventH;

static bool _Action_Free( float cur_x, float cur_y )
{
	int32_t meas_w;
	float   offset_x;

	fRECT* p_field_rect;

	p_field_rect = if_PlayField_GetRect();

	if( cur_x <  p_field_rect->l ||
		cur_x >= p_field_rect->r ||
		cur_y <  p_field_rect->t ||
		cur_y >= p_field_rect->b ) return false;


	g_cursor.focus        = ifCurFocus_PlayField;
	g_cursor.p_field_rect = p_field_rect;
	g_cursor.tgt_id       = 0;
	g_cursor.drag_ypos    = 0;
	g_cursor.rect.t       = ( p_field_rect->b + p_field_rect->t ) / 2;
	g_cursor.rect.b       = g_cursor.rect.t;

	offset_x              = (float)g_ScrlEventH.GetOffset();
	meas_w                = g_pxtn->master->get_beat_num() * if_BaseField_GetBeatWidth();
	g_cursor.drag_xpos[0] = (int32_t)( cur_x - g_cursor.p_field_rect->l + offset_x ) / meas_w;
	g_cursor.drag_xpos[1] = g_cursor.drag_xpos[0] + 1;

	if( (int32_t)( cur_x - g_cursor.p_field_rect->l + offset_x ) % meas_w < meas_w / 2 )
	{
		g_cursor.rect.l = g_cursor.drag_xpos[0] * meas_w + p_field_rect->l - offset_x;
		g_cursor.rect.r = 0;
	}else{
		g_cursor.rect.r = g_cursor.drag_xpos[1] * meas_w + p_field_rect->l - offset_x;
		g_cursor.rect.l = 0;
	}

	if( g_cursor.drag_xpos[0] >= g_pxtn->master->get_meas_num() )
	{
		g_cursor.focus        = ifCurFocus_None;
		return false;
	}

	if(      KeyControl_IsClickLeftTrigger()  )
		g_cursor.action = 1;
	else if( KeyControl_IsClickRightTrigger() )
		g_cursor.action = 2;

	return true;
}

static bool _Action_Drag1( float cur_x, float cur_y )
{
	int32_t meas_w;
	float   offset_x;

	offset_x              = (float)g_ScrlEventH.GetOffset();
	meas_w                = g_pxtn->master->get_beat_num() * if_BaseField_GetBeatWidth();
	g_cursor.drag_xpos[0] = (int32_t)( cur_x - g_cursor.p_field_rect->l + offset_x ) / meas_w;
	g_cursor.drag_xpos[1] = g_cursor.drag_xpos[0] + 1;

	if( (int32_t)( cur_x - g_cursor.p_field_rect->l + offset_x ) % meas_w < meas_w / 2 )
	{
		g_cursor.rect.l = g_cursor.drag_xpos[0] * meas_w + g_cursor.p_field_rect->l - offset_x;
		g_cursor.rect.r = 0;
	}else{
		g_cursor.rect.r = g_cursor.drag_xpos[1] * meas_w + g_cursor.p_field_rect->l - offset_x;
		g_cursor.rect.l = 0;
	}

	if( KeyControl_IsClickLeft() && KeyControl_IsClickRight() ){ g_cursor.action = 0; return true; }
	if( !KeyControl_IsClickLeft() )
	{
		if( g_cursor.rect.l ) if_PlayField_SetStartMeas( (int32_t)g_cursor.drag_xpos[0], true );
		else                  if_PlayField_SetEndMeas(   (int32_t)g_cursor.drag_xpos[1], true );
		g_cursor.action = 0;
	}

	return true;
}

static bool _Action_Drag2( float cur_x, float cur_y )
{
	int32_t meas_w;
	float   offset_x;

	offset_x              = (float)g_ScrlEventH.GetOffset();
	meas_w                = g_pxtn->master->get_beat_num() * if_BaseField_GetBeatWidth();
	g_cursor.drag_xpos[0] = ( cur_x - g_cursor.p_field_rect->l + offset_x ) / meas_w;

	if( g_cursor.drag_xpos[0] >= g_pxtn->master->get_meas_num() ){ g_cursor.action = 0; return true; }
	if( KeyControl_IsClickLeft() && KeyControl_IsClickRight()   ){ g_cursor.action = 0; return true; }

	if( !KeyControl_IsClickRight() )
	{
		if_Menu_Play_SetActive( true );
		if_Menu_Play_SetPosition( cur_x, cur_y, true );
		g_cursor.action = 0;
	}

	return true;
}

bool if_Cursor_Action_PlayField( float cur_x, float cur_y )
{
	bool b_ret = false;

	switch( g_cursor.action ){
	// フリー --------
	case 0: b_ret = _Action_Free(  cur_x, cur_y ); break;
	case 1: b_ret = _Action_Drag1( cur_x, cur_y ); break;
	case 2: b_ret = _Action_Drag2( cur_x, cur_y ); break;
	}
	return b_ret;
}
