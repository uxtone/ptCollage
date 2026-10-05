
#include <pxwDx09Draw.h>
extern pxwDx09Draw*         g_dxdraw  ;

#include <pxtnPulse_Frequency.h>
extern pxtnPulse_Frequency* g_freq    ;

#include <pxtnService.h>
extern pxtnService*         g_pxtn    ;

#include <pxtonewinXA2.h>
extern pxtonewinXA2*        g_strm_xa2;


#include "../../Generic/KeyControl.h"
#include "../../Generic/if_gen_Scroll.h"

#include "../UndoEvent.h"
#include "../ActiveTone.h"

#include "Interface.h"
#include "SurfaceNo.h"
#include "if_BaseField.h"
#include "if_VolumePanel.h"
#include "if_ToolPanel.h"
#include "if_Cursor.h"

#include "if_VolumeField.h"
#include "if_KeyField.h"
#include "if_UnitField.h"
#include "if_WoiceField.h"
#include "if_ScopeField.h"

#include "if_SelectNo_Woice.h"
#include "if_SelectNo_Group.h"


// ドラッグスクロールウエイト
#define DRAGSCROLLWAIT 2

static uint32_t _color_note     = 0xffF08000;
static uint32_t _color_velocity = 0xff00F080;
static uint32_t _color_delete   = 0xffF00000;
static uint32_t _color_scope    = 0xff00F080;

extern if_gen_Scroll    g_ScrlEventH;
extern if_gen_Scroll    g_ScrlWoiceH;
extern if_gen_Scroll    g_ScrlWoiceV;
extern if_gen_Scroll    g_ScrlUnitV;
extern if_gen_Scroll    g_ScrlKeyV;

bool if_Cursor_Action_Panel_Comment(  float cur_x, float cur_y );
bool if_Cursor_Action_Panel_Rack(     float cur_x, float cur_y );
bool if_Cursor_Action_Panel_Scale(    float cur_x, float cur_y );
bool if_Cursor_Action_UnitField(      float cur_x, float cur_y );
bool if_Cursor_Action_UnitTray(       float cur_x, float cur_y );
bool if_Cursor_Action_WoiceTray(      float cur_x, float cur_y );
bool if_Cursor_Action_VolumeField(    float cur_x, float cur_y );
bool if_Cursor_Action_KeyField(       float cur_x, float cur_y );
bool if_Cursor_Action_Keyboard(       float cur_x, float cur_y );
bool if_Cursor_Action_ScopeField(     float cur_x, float cur_y );
bool if_Cursor_Action_Menu_Scope(     float cur_x, float cur_y );
bool if_Cursor_Action_Menu_Play(      float cur_x, float cur_y );
bool if_Cursor_Action_Menu_Unit(      float cur_x, float cur_y );
bool if_Cursor_Action_Menu_Woice(     float cur_x, float cur_y );
bool if_Cursor_Action_Menu_Division(  float cur_x, float cur_y );
bool if_Cursor_Action_Menu_Zoom(      float cur_x, float cur_y );
bool if_Cursor_Action_PlayField(      float cur_x, float cur_y );
bool if_Cursor_Action_ToolPanel(      float cur_x, float cur_y );
bool if_Cursor_Action_VolumePanel(    float cur_x, float cur_y );
bool if_Cursor_Action_Tenkey_Tuning(  float cur_x, float cur_y );

bool if_Cursor_Action_Player_Priority();
bool if_Cursor_Action_Player(         float cur_x, float cur_y );
bool if_Cursor_Action_Copier(         float cur_x, float cur_y );
bool if_Cursor_Action_Projector(      float cur_x, float cur_y );
bool if_Cursor_Action_Effector(       float cur_x, float cur_y );
bool if_Cursor_Action_SubWindow(      float cur_x, float cur_y );

ACTIONCURSOR g_cursor;

bool if_Cursor_init( const pxPalette* palpng )
{
	if( !palpng ) return false;

	if( !palpng->get_color( COLORINDEX_note_cur_0, &_color_note     ) ) return false;
	if( !palpng->get_color( COLORINDEX_velocity_0, &_color_velocity ) ) return false;
	if( !palpng->get_color( COLORINDEX_delete_0  , &_color_delete   ) ) return false;
	if( !palpng->get_color( COLORINDEX_scope_line, &_color_scope    ) ) return false;

	g_cursor.tgt_id         =     0;
	g_cursor.action         =     0;
	g_cursor.active_tone_id =     0;
	g_cursor.active_tone_on = false;

	return true;
}

void if_Cursor_DragScroll( float frame1, float frame2, float cur, if_gen_Scroll* scrl )
{
	if( g_cursor.focus == ifCurFocus_VolumeField )
	{
		switch( if_GetVolumeMode() )
		{
		case VOLUMEMODE_GroupNo: return;
		case VOLUMEMODE_VoiceNo: return;
		case VOLUMEMODE_Tuning : return;
		}
	}

	if( cur <  frame1 || cur >= frame2 )
	{
		if( !g_cursor.scroll_count )
		{
			if( cur <  frame1 ) scrl->Scroll( (int32_t)( frame2 - frame1 ), -1 );
			if( cur >= frame2 ) scrl->Scroll( (int32_t)( frame2 - frame1 ),  1 );
			g_cursor.scroll_count = DRAGSCROLLWAIT;
		}
		else
		{
			g_cursor.scroll_count--;
		}
	}else{
		g_cursor.scroll_count = 0;
	}
}

void if_Cursor_SetActiveTone_Unit( int32_t u, float cur_x, float freq_rate, int32_t pan_vol, int32_t velocity, int32_t pan_time )
{
	if( g_cursor.active_tone_on )
	{
		ActiveTone_stop( g_cursor.active_tone_id );
		g_cursor.active_tone_on = false;
	}

	int32_t velos[ pxtnMAX_UNITCONTROLVOICE ];
	for( int i = 0; i < pxtnMAX_UNITCONTROLVOICE; i++ ) velos[ i ] = velocity;
	g_cursor.active_tone_id =  ActiveTone_set_unit( u, cur_x, freq_rate, pan_vol, velos, pan_time );

	if( -1 == g_cursor.active_tone_id ) return;
	g_cursor.active_tone_on = true ;
}

void if_Cursor_SetActiveTone_Unit( int32_t u, float cur_x, float freq_rate )
{
	if_Cursor_SetActiveTone_Unit( u, cur_x, freq_rate, -1, -1, -1 );
}

void if_Cursor_SetActiveTone_Unit( int32_t u, float cur_x )
{
	if_Cursor_SetActiveTone_Unit( u, cur_x, -1, -1, -1, -1 );
}

void if_Cursor_SetActiveTone_Woice( int32_t w )
{
	if( g_cursor.active_tone_on )
	{
		ActiveTone_stop( g_cursor.active_tone_id );
		g_cursor.active_tone_on = false;
	}
	g_cursor.active_tone_id = ActiveTone_set_woice( w );
	if( g_cursor.active_tone_id != -1 ) g_cursor.active_tone_on = true;

}

void if_Cursor_ChangeActiveTone( float freq_rate )
{
	if( !g_cursor.active_tone_on ) return;
	ActiveTone_change_freq( g_cursor.active_tone_id, freq_rate );
}

void if_Cursor_StopActiveTone()
{
	if( !g_cursor.active_tone_on ) return;
	ActiveTone_stop( g_cursor.active_tone_id );
	g_cursor.active_tone_on = false;
}

bool if_Cursor_DragAction( float cur_x, float cur_y, int32_t *p_clock1, int32_t *p_clock2 )
{
	int32_t divi_w       = if_BaseField_GetDivideWidth();
	int32_t beat_w       = if_BaseField_GetBeatWidth();
	int32_t offset_x     = g_ScrlEventH.GetOffset();
	fRECT*  p_field_rect = g_cursor.p_field_rect;
	float   xpos         = (float)( ( (int32_t)( cur_x - p_field_rect->l + offset_x ) / divi_w ) * divi_w );

	// cancel drag.
	if( KeyControl_IsClickLeft() && KeyControl_IsClickRight() )
	{
		if_Cursor_StopActiveTone();
		g_cursor.action = 0;
		g_cursor.scope  = enum_ScopeMode_Point;
		if_SelectNo_Woice_End( 0 );
		if_SelectNo_Group_End( 0 );
		return false;
	}

	if( KeyControl_IsClickLeft() || KeyControl_IsClickRight() )
	{
		if( cur_x != g_cursor.start_x || cur_y != g_cursor.start_y ) g_cursor.scope = enum_ScopeMode_Scope;

		switch( g_cursor.scope )
		{
		case enum_ScopeMode_Point:
			g_cursor.rect.l = g_cursor.drag_xpos[0] + p_field_rect->l - offset_x;
			g_cursor.rect.r = g_cursor.rect.l + 2;
			break;
		case enum_ScopeMode_Scope:

			if( xpos >= g_cursor.drag_xpos[ 0 ] ){
				g_cursor.rect.l = g_cursor.drag_xpos[ 0 ] + p_field_rect->l - offset_x;
				g_cursor.rect.r = xpos + divi_w           + p_field_rect->l - offset_x;
			}else{
				g_cursor.rect.l = xpos                    + p_field_rect->l - offset_x;
				g_cursor.rect.r = g_cursor.drag_xpos[ 1 ] + p_field_rect->l - offset_x;
			}
			break;
		}

		if_Cursor_DragScroll( p_field_rect->l, p_field_rect->r, cur_x, &g_ScrlEventH );

		return false;
	}

	int32_t beat_clock;

	beat_clock  = g_pxtn->master->get_beat_clock();

	if(    xpos >= g_cursor.drag_xpos[ 0 ] )
	{
		*p_clock1 = ( (int32_t)g_cursor.drag_xpos[ 0 ] ) * beat_clock / beat_w;
		*p_clock2 = ( (int32_t)xpos + divi_w           ) * beat_clock / beat_w;
	}
	else
	{
		*p_clock1 = ( (int32_t)xpos                    ) * beat_clock / beat_w;
		*p_clock2 = ( (int32_t)g_cursor.drag_xpos[ 1 ] ) * beat_clock / beat_w;
	}
	if( *p_clock1 < 0 ) *p_clock1 = 0;
	if( *p_clock2 < 0 ) *p_clock2 = 0;

	if( g_cursor.scope == enum_ScopeMode_Point ) *p_clock2 = *p_clock1;

	if_Cursor_StopActiveTone();
	g_cursor.action = 0;
	g_cursor.scope  = enum_ScopeMode_Point;

	return true;
}

bool if_Cursor_Action( float cur_x, float cur_y )
{
	bool b_l_clk = KeyControl_IsClickLeft       ();
	bool b_l_trg = KeyControl_IsClickLeftTrigger();

	// フリー ========================================
	if( !g_cursor.action )
	{
		g_cursor.focus = ifCurFocus_None;
		while( 1 )
		{
			if( if_Cursor_Action_Player_Priority(            ) ) break;
			if( if_Cursor_Action_Menu_Division( cur_x, cur_y ) ) break;
			if( if_Cursor_Action_Menu_Zoom(     cur_x, cur_y ) ) break;

			// WOICE MODE ----
			if( if_ToolPanel_GetMode() == enum_ToolMode_W )
			{
				if( if_Cursor_Action_Menu_Woice(     cur_x, cur_y ) ) break;

				if( if_Cursor_Action_SubWindow(      cur_x, cur_y ) ) break;

				if( if_Cursor_Action_Panel_Rack(     cur_x, cur_y ) ) break;
				if( if_Cursor_Action_Panel_Comment(  cur_x, cur_y ) ) break;
				if( if_Cursor_Action_Panel_Scale(    cur_x, cur_y ) ) break;

				if( g_ScrlWoiceH.Action( (int32_t)cur_x, (int32_t)cur_y, b_l_clk, b_l_trg ) ){ g_cursor.focus = ifCurFocus_WoiceHScroll; g_cursor.action = 1; break; }
				if( g_ScrlWoiceV.Action( (int32_t)cur_x, (int32_t)cur_y, b_l_clk, b_l_trg ) ){ g_cursor.focus = ifCurFocus_WoiceScroll ; g_cursor.action = 1; break; }

				if( if_Cursor_Action_WoiceTray(      cur_x, cur_y ) ) break;
				if( if_Cursor_Action_ToolPanel(      cur_x, cur_y ) ) break;

				if( g_strm_xa2->tune_is_sampling()                      ) break;

			}
			// UNIT / KEY MODE ----
			else
			{
				if( if_Cursor_Action_Menu_Play    ( cur_x, cur_y ) ) break;
				if( if_Cursor_Action_Menu_Scope   ( cur_x, cur_y ) ) break;
				if( if_Cursor_Action_Menu_Unit    ( cur_x, cur_y ) ) break;
				if( if_Cursor_Action_Tenkey_Tuning( cur_x, cur_y ) ) break;

				if( if_Cursor_Action_SubWindow    ( cur_x, cur_y ) ) break;

				if( if_Cursor_Action_Panel_Rack   ( cur_x, cur_y ) ) break;
				if( if_Cursor_Action_Panel_Comment( cur_x, cur_y ) ) break;
				if( if_Cursor_Action_Panel_Scale  ( cur_x, cur_y ) ) break;

				if( g_ScrlEventH.Action( (int32_t)cur_x, (int32_t)cur_y, b_l_clk, b_l_trg ) ){ g_cursor.focus = ifCurFocus_EventHScroll; g_cursor.action = 1; break; }
				if( g_ScrlKeyV.Action(   (int32_t)cur_x, (int32_t)cur_y, b_l_clk, b_l_trg ) ){ g_cursor.focus = ifCurFocus_KeyScroll   ; g_cursor.action = 1; break; }
				if( g_ScrlUnitV.Action(  (int32_t)cur_x, (int32_t)cur_y, b_l_clk, b_l_trg ) ){ g_cursor.focus = ifCurFocus_UnitScroll  ; g_cursor.action = 1; break; }
				if( g_ScrlWoiceV.Action( (int32_t)cur_x, (int32_t)cur_y, b_l_clk, b_l_trg ) ){ g_cursor.focus = ifCurFocus_WoiceScroll ; g_cursor.action = 1; break; }

				if( if_Cursor_Action_UnitTray(       cur_x, cur_y ) ) break;
				if( if_Cursor_Action_Keyboard(       cur_x, cur_y ) ) break;
				if( if_Cursor_Action_ToolPanel(      cur_x, cur_y ) ) break;
				if( if_Cursor_Action_VolumePanel(    cur_x, cur_y ) ) break;


				if( g_strm_xa2->tune_is_sampling()                    ) break;

				if( if_Cursor_Action_KeyField(       cur_x, cur_y ) ) break;
				if( if_Cursor_Action_PlayField(      cur_x, cur_y ) ) break;
				if( if_Cursor_Action_ScopeField(     cur_x, cur_y ) ) break;
				if( if_Cursor_Action_UnitField(      cur_x, cur_y ) ) break;
				if( if_Cursor_Action_VolumeField(    cur_x, cur_y ) ) break;
			}

			break;
		}

	}
	// ビジー ========================================
	else
	{
		bool bDrag = false;

		if( !KeyControl_IsClickLeft() )
			bDrag = bDrag;

		switch( g_cursor.focus )
		{
		case ifCurFocus_EventHScroll : if( g_ScrlEventH.Action( (int32_t)cur_x, (int32_t)cur_y, b_l_clk, b_l_trg ) ) bDrag = true; else g_cursor.action  = 0; break;
		case ifCurFocus_WoiceHScroll : if( g_ScrlWoiceH.Action( (int32_t)cur_x, (int32_t)cur_y, b_l_clk, b_l_trg ) ) bDrag = true; else g_cursor.action  = 0; break;
		case ifCurFocus_KeyScroll    : if( g_ScrlKeyV.Action(   (int32_t)cur_x, (int32_t)cur_y, b_l_clk, b_l_trg ) ) bDrag = true; else g_cursor.action  = 0; break;
		case ifCurFocus_UnitScroll   : if( g_ScrlUnitV.Action(  (int32_t)cur_x, (int32_t)cur_y, b_l_clk, b_l_trg ) ) bDrag = true; else g_cursor.action  = 0; break;
		case ifCurFocus_WoiceScroll  : if( g_ScrlWoiceV.Action( (int32_t)cur_x, (int32_t)cur_y, b_l_clk, b_l_trg ) ) bDrag = true; else g_cursor.action  = 0; break;

		case ifCurFocus_Tenkey_Tuning: bDrag = if_Cursor_Action_Tenkey_Tuning( cur_x, cur_y ); break;

		case ifCurFocus_Player       : bDrag = if_Cursor_Action_Player     ( cur_x, cur_y ); break;
		case ifCurFocus_Copier       : bDrag = if_Cursor_Action_Copier     ( cur_x, cur_y ); break;
		case ifCurFocus_Projector    : bDrag = if_Cursor_Action_Projector  ( cur_x, cur_y ); break;
		case ifCurFocus_Effector     : bDrag = if_Cursor_Action_Effector   ( cur_x, cur_y ); break;

		case ifCurFocus_UnitField    : bDrag = if_Cursor_Action_UnitField  ( cur_x, cur_y ); break;
		case ifCurFocus_VolumeField  : bDrag = if_Cursor_Action_VolumeField( cur_x, cur_y ); break;
		case ifCurFocus_UnitTray     : bDrag = if_Cursor_Action_UnitTray   ( cur_x, cur_y ); break;
		case ifCurFocus_WoiceTray    : bDrag = if_Cursor_Action_WoiceTray  ( cur_x, cur_y ); break;
		case ifCurFocus_KeyField     : bDrag = if_Cursor_Action_KeyField   ( cur_x, cur_y ); break;
		case ifCurFocus_Keyboard     : bDrag = if_Cursor_Action_Keyboard   ( cur_x, cur_y ); break;
		case ifCurFocus_PlayField    : bDrag = if_Cursor_Action_PlayField  ( cur_x, cur_y ); break;
		case ifCurFocus_ScopeField   : bDrag = if_Cursor_Action_ScopeField ( cur_x, cur_y ); break;
		case ifCurFocus_ToolPanel    : bDrag = if_Cursor_Action_ToolPanel  ( cur_x, cur_y ); break;
		case ifCurFocus_VolumePanel  : bDrag = if_Cursor_Action_VolumePanel( cur_x, cur_y ); break;

		case ifCurFocus_Panel_Rack   : bDrag = if_Cursor_Action_Panel_Rack ( cur_x, cur_y ); break;

		}
		return true;
	}

	if( g_cursor.focus == ifCurFocus_None ) return false;
	return true;
}


void if_Cursor_Put()
{
	fRECT rect;

	fRECT rc_drag_frame = {384,208,512,224};

	fRECT rc_cursor     = {  4,480,  8,496};
	fRECT rcStart       = { 16,504, 56,512};
	fRECT rcEnd         = { 56,504, 96,512};

	uint32_t color;
	int32_t          name_surf;

	if(	g_cursor.focus == ifCurFocus_None ) return;

	rect    = g_cursor.rect;
	rect.t -= 4;
	rect.b += 4;

	switch( g_cursor.focus )
	{
	case ifCurFocus_UnitField  : color = _color_note    ; break;
	case ifCurFocus_KeyField   : color = _color_note    ; break;
	case ifCurFocus_ScopeField : color = _color_scope   ; break;
	case ifCurFocus_VolumeField: color = _color_velocity; break;
	case ifCurFocus_UnitTray   :                          break;
	}

	switch( g_cursor.focus )
	{
	case ifCurFocus_UnitField  :
	case ifCurFocus_KeyField   :
	case ifCurFocus_VolumeField:
	case ifCurFocus_ScopeField :

		if( g_cursor.focus == ifCurFocus_VolumeField )
		{
			rect    = g_cursor.rect;
			rect.t -= 1;
			rect.b += 1;
		}

		if( g_cursor.scope == enum_ScopeMode_Point )
		{
			g_dxdraw->tex_Put_Clip( g_cursor.rect.l, g_cursor.rect.t - 8, &rc_cursor, SURF_FIELDS, g_cursor.p_field_rect );
			rect.r = rect.l + 2;
		}
		else
		{
			if( g_cursor.action == 2 ) color = _color_delete;
			if( rect.b > g_cursor.p_field_rect->b )
				rect.b = g_cursor.p_field_rect->b;
			if( rect.t < g_cursor.p_field_rect->t )
				rect.t = g_cursor.p_field_rect->t;
			g_dxdraw->FillRect_view( &rect, color );
		}
		break;

	case ifCurFocus_UnitTray:

		if( g_cursor.action == 1 &&
			g_cursor.rect.t > g_cursor.p_field_rect->t - UNIT_HEIGHT &&
			g_cursor.rect.t < g_cursor.p_field_rect->b )
		{
			int32_t u = g_cursor.tgt_id;

			g_dxdraw->tex_Put_View( g_cursor.rect.l, g_cursor.rect.t, &rc_drag_frame, SURF_FIELDS );

			rect.t = (float)( (u       % 10) * UNITNAME_HEIGHT );
			rect.b = (float)( rect.t + UNITNAME_HEIGHT - 2     );
			rect.l = 0;
			rect.r = UNITNAME_WIDTH;
			name_surf = SURF_UNITNAME + u       / 10;
			g_dxdraw->tex_Put_View( g_cursor.rect.l + 16 + UNITNAME_WIDTH, g_cursor.rect.t + 2, &rect, name_surf );
		}
		break;

	case ifCurFocus_WoiceTray:

		if( g_cursor.action == 1 &&
			g_cursor.rect.t > g_cursor.p_field_rect->t - WOICE_HEIGHT &&
			g_cursor.rect.t < g_cursor.p_field_rect->b )
		{
			int32_t w = g_cursor.tgt_id;

			g_dxdraw->tex_Put_View( g_cursor.rect.l, g_cursor.rect.t, &rc_drag_frame, SURF_FIELDS );
			rect.t    = (float)( (w % 10) * WOICENAME_HEIGHT   );
			rect.b    = (float)( rect.t + WOICENAME_HEIGHT - 2 );
			rect.l    = 0;
			rect.r    = WOICENAME_WIDTH;
			name_surf = SURF_WOICENAME + w / 10;
			g_dxdraw->tex_Put_View( g_cursor.rect.l + 16 + WOICENAME_WIDTH, g_cursor.rect.t + 2, &rect, name_surf );
		}
		break;

	case ifCurFocus_PlayField:
		if( g_cursor.rect.l ) g_dxdraw->tex_Put_Clip( g_cursor.rect.l,      g_cursor.rect.t, &rcStart, SURF_FIELDS, g_cursor.p_field_rect );
		else                  g_dxdraw->tex_Put_Clip( g_cursor.rect.r - 40, g_cursor.rect.t, &rcEnd,   SURF_FIELDS, g_cursor.p_field_rect );

		break;
	}
}