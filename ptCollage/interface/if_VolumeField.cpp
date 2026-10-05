
#include <pxtnService.h>
extern pxtnService *g_pxtn;


#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../../Generic/if_gen_Scroll.h"
#include "../../Generic/if_Generic.h"

#include "../UnitFocus.h"

#include "SurfaceNo.h"
#include "Interface.h"
#include "if_BaseField.h"
#include "if_VolumePanel.h"
#include "if_ToolPanel.h"


extern if_gen_Scroll  g_ScrlEventH;

static uint32_t _color_velocity[ 2 ] =
{
	0xff00F080,
	0xff006060,
};

static fRECT _rcField = {0};

bool if_VolumeField_init( const pxPalette* palpng )
{
	if( !palpng ) return false;

	if( !palpng->get_color( COLORINDEX_velocity_0, &_color_velocity[ 0 ] ) ) return false;
	if( !palpng->get_color( COLORINDEX_velocity_1, &_color_velocity[ 1 ] ) ) return false;
	return true;
}


void if_VolumeField_SetRect( const fRECT *rc_view )
{
	_rcField.l = rc_view->l + NAMETRAY_WIDTH;
	_rcField.r = rc_view->r - SCROLLBUTTONSIZE;
	_rcField.b = rc_view->b - SCROLL_HEIGHT;
	if( if_ToolPanel_GetMode() != enum_ToolMode_W && if_VolumePanel_IsUse() )
		_rcField.t = rc_view->b - SCROLL_HEIGHT - VOLUME_HEIGHT;
	else
		_rcField.t = rc_view->b - SCROLL_HEIGHT;
}

fRECT* if_VolumeField_GetRect()
{
	return &_rcField;
}

// ボリューム表示
void if_VolumeField_Put()
{

	float              y_now ;
	float              y_next;
	float              x;
	fRECT              rc;
	fRECT              rcName        = {  0,  0,WOICENAME_WIDTH,   0 };

	static const fRECT rcScale      = {  0,192,240,320};

	static const fRECT rcNote       = {  8,480, 12,496};

	static const fRECT rcHead       = {  8,480, 10,496};
	static const fRECT rcTail       = { 10,480, 12,496};
	static const fRECT rcZero       = { 12,480, 16,496};

	static const fRECT rc_box_voice = {384,256,512,272};

	static const fRECT rc_box_group = {384,272,408,288};
	static const fRECT rc_box_crrct = {432,272,496,288};

	int32_t            name_surf;
	float              last_x;

	const EVERECORD*   p_eve;
	int32_t            kind_tgt;
	int32_t            beat_w;
	float              offset_x;
	int32_t            beat_clock;

	VOLUMEMODE  volume_mode;

	int32_t             u;

	if_gen_tile_h( &_rcField, &rcScale, 1, 0, SURF_FIELDS );

	u           = UnitFocus_Get(); if( u < 0 ) return;

	beat_w      = if_BaseField_GetBeatWidth();
	offset_x    = (float)g_ScrlEventH.GetOffset();
	volume_mode = if_GetVolumeMode();
	beat_clock  = g_pxtn->master->get_beat_clock();

	// 範囲
	switch( volume_mode )
	{
	case VOLUMEMODE_Velocity  : kind_tgt = EVENTKIND_VELOCITY  ; break;
	case VOLUMEMODE_Pan_Time  : kind_tgt = EVENTKIND_PAN_TIME  ; break;
	case VOLUMEMODE_Pan_Volume: kind_tgt = EVENTKIND_PAN_VOLUME; break;
	case VOLUMEMODE_Volume    : kind_tgt = EVENTKIND_VOLUME    ; break;
	case VOLUMEMODE_Portament : kind_tgt = EVENTKIND_PORTAMENT ; break;
	case VOLUMEMODE_VoiceNo   : kind_tgt = EVENTKIND_VOICENO   ; break;
	case VOLUMEMODE_GroupNo   : kind_tgt = EVENTKIND_GROUPNO   ; break;
	case VOLUMEMODE_Tuning    : kind_tgt = EVENTKIND_TUNING    ; break;
	}

	switch( volume_mode )
	{
	case VOLUMEMODE_Velocity:

		if( int eve_num = if_BaseField_Event_get_View( u, kind_tgt, &p_eve ) )
		{
			for( int e = 0; e < eve_num; e++, p_eve++ )
			{
				if( p_eve->unit_no == u && p_eve->kind == kind_tgt )
				{
					x     = ( p_eve->clock * beat_w / beat_clock ) - offset_x + _rcField.l;
					y_now = _rcField.b - p_eve->value;
					if( x >= _rcField.l - 8 ) g_dxdraw->tex_Put_Clip( x, y_now - 8, &rcNote, SURF_FIELDS, &_rcField );
				}
			}
			if_BaseField_Event_get_View_Unlock();
		}
		break;

	case VOLUMEMODE_Pan_Time:
	case VOLUMEMODE_Pan_Volume :
	case VOLUMEMODE_Volume :

		switch( volume_mode )
		{
		case VOLUMEMODE_Pan_Volume: y_now = _rcField.b - EVENTDEFAULT_PAN_VOLUME; break;
		case VOLUMEMODE_Pan_Time  : y_now = _rcField.b - EVENTDEFAULT_PAN_TIME  ; break;
		case VOLUMEMODE_Volume    : y_now = _rcField.b - EVENTDEFAULT_VOLUME    ; break;
		}
		rc.l = _rcField.l;
		rc.r = _rcField.l;

		if( int32_t eve_num = if_BaseField_Event_get_View( u, kind_tgt, &p_eve ) )
		{
			for( int32_t e = 0; e < eve_num; e++, p_eve++ )
			{
				if( p_eve->unit_no == u && p_eve->kind == kind_tgt )
				{
					// 横線
					rc.t = y_now - 2;
					rc.b = y_now + 2;
					rc.r = ( p_eve->clock * beat_w / beat_clock ) - offset_x + _rcField.l;
					g_dxdraw->FillRect_clip( &rc, _color_velocity[ 1 ], &_rcField );

					// 縦線
					if( rc.r >= _rcField.l )
					{
						rc.l = rc.r;
						rc.r = rc.r + 2;
					}
					else
					{
						rc.r = _rcField.l;
					}
					y_next = _rcField.b - p_eve->value;
					if( y_next < y_now ) rc.t= y_next + 2;
					else                 rc.b= y_next - 2;
					g_dxdraw->FillRect_clip( &rc, _color_velocity[ 1 ], &_rcField );

					// 点
					y_now = y_next;
					rc.t  = y_now - 2;
					rc.b  = y_now + 2;
					g_dxdraw->FillRect_clip( &rc, _color_velocity[ 0 ], &_rcField );

					rc.l += 2;
				}
			}
			if_BaseField_Event_get_View_Unlock();
		}

		rc.l = rc.r      ;
		rc.r = _rcField.r;
		rc.t = y_now - 2 ;
		rc.b = y_now + 2 ;
		g_dxdraw->FillRect_clip( &rc, _color_velocity[ 1 ], &_rcField );
		break;

	case VOLUMEMODE_Portament:

		y_now = _rcField.b - VOLUME_HEIGHT/2;

		if( int eve_num = if_BaseField_Event_get_View( u, kind_tgt, &p_eve ) )
		{
			for( int e = 0; e < eve_num; e++, p_eve++ )
			{
				if( p_eve->unit_no == u && p_eve->kind == kind_tgt )
				{
					x = ( p_eve->clock * beat_w / beat_clock ) - offset_x + _rcField.l;

					if( p_eve->value )
					{
						rc.l = (  p_eve->clock                 * beat_w / beat_clock ) - offset_x + _rcField.l + 2;
						rc.r = ( (p_eve->clock + p_eve->value) * beat_w / beat_clock ) - offset_x + _rcField.l - 2;
						rc.t = y_now - 2;
						rc.b = y_now + 2;
						if( rc.l < _rcField.l ) rc.l = _rcField.l;
						if( rc.r > _rcField.r ) rc.r = _rcField.r;
						if( rc.b > _rcField.b ) rc.b = _rcField.b;
						if( rc.t < _rcField.t ) rc.t = _rcField.t;

						g_dxdraw->FillRect_view( &rc, _color_velocity[ 0 ] );
						if( x    >= _rcField.l - 2 ) g_dxdraw->tex_Put_Clip( x,    y_now - 8, &rcHead, SURF_FIELDS, &_rcField );
						if( rc.r <  _rcField.r - 2 ) g_dxdraw->tex_Put_Clip( rc.r, y_now - 8, &rcTail, SURF_FIELDS, &_rcField );
					}
					else
					{
						g_dxdraw->tex_Put_Clip( x, y_now - 8, &rcZero, SURF_FIELDS, &_rcField );
					}
				}
			}
			if_BaseField_Event_get_View_Unlock();
		}
		break;

	case VOLUMEMODE_VoiceNo:

		if( int32_t eve_num = if_BaseField_Event_get_View( u, kind_tgt, &p_eve ) )
		{
			for( int32_t e = 0; e < eve_num; e++, p_eve++ )
			{
				if( p_eve->unit_no == u && p_eve->kind == kind_tgt )
				{
					x = ( p_eve->clock * beat_w / beat_clock ) - offset_x + _rcField.l;
					if( e && x <= last_x + NAMEBOX_WIDTH )
					{
						y_now -= NAMEBOX_HEIGHT;
						if( y_now < _rcField.t ) y_now = _rcField.b - WOICENAME_HEIGHT;
					}
					else
					{
						y_now = _rcField.b - NAMEBOX_HEIGHT;
					}

					g_dxdraw->tex_Put_Clip( x     , y_now    , &rc_box_voice, SURF_FIELDS, &_rcField );
					if_gen_num6_clip      ( x + 4 , y_now + 4, p_eve->value, 2,            &_rcField );

					int32_t w = p_eve->value;

					if( w < g_pxtn->Woice_Num() )
					{
						rcName.t  = ( w % 10 ) * WOICENAME_HEIGHT;
						rcName.b  = rcName.t   + WOICENAME_HEIGHT - 2;
						name_surf = SURF_WOICENAME + w / 10;
						g_dxdraw->tex_Put_Clip( x + 24, y_now + 2, &rcName, name_surf, &_rcField );
					}
					last_x = x;
				}
			}
			if_BaseField_Event_get_View_Unlock();
		}
		break;

	case VOLUMEMODE_GroupNo:

		if( int32_t eve_num = if_BaseField_Event_get_View( u, kind_tgt, &p_eve ) )
		{
			for( int32_t e = 0; e < eve_num; e++, p_eve++ )
			{
				if( p_eve->unit_no == u && p_eve->kind == kind_tgt )
				{
					x = ( p_eve->clock * beat_w / beat_clock ) - offset_x + _rcField.l;
					if( e && x <= last_x + NOBOX_WIDTH )
					{
						y_now -= NOBOX_HEIGHT;
						if( y_now < _rcField.t ) y_now = _rcField.b - NOBOX_HEIGHT;
					}
					else
					{
						y_now = _rcField.b - NOBOX_HEIGHT;
					}

					g_dxdraw->tex_Put_Clip( x, y_now, &rc_box_group, SURF_FIELDS, &_rcField );
					if_gen_num6_clip( x + 4 , y_now + 4, p_eve->value, 2,         &_rcField );

					last_x = x;
				}
			}
			if_BaseField_Event_get_View_Unlock();
		}
		break;

	case VOLUMEMODE_Tuning:

		if( int32_t eve_num = if_BaseField_Event_get_View( u, kind_tgt, &p_eve ) )
		{
			for( int32_t e = 0; e < eve_num; e++, p_eve++ )
			{
				if( p_eve->unit_no == u && p_eve->kind == kind_tgt )
				{
					x = ( p_eve->clock * beat_w / beat_clock ) - offset_x + _rcField.l;
					if( e && x <= last_x + TUNINGBOX_WIDTH )
					{
						y_now -= TUNINGBOX_HEIGHT;
						if( y_now < _rcField.t ) y_now = _rcField.b - TUNINGBOX_HEIGHT;
					}
					else
					{
						y_now = _rcField.b - TUNINGBOX_HEIGHT;
					}

					g_dxdraw->tex_Put_Clip( x, y_now, &rc_box_crrct, SURF_FIELDS , &_rcField );
					if_gen_float6_clip( x + 4 , y_now + 4, *(float*)&p_eve->value, &_rcField );

					last_x = x;
				}
			}
			if_BaseField_Event_get_View_Unlock();
		}
		break;
	}
}

