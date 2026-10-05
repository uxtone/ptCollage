
#include <pxtnService.h>
extern pxtnService *g_pxtn;


#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../../Generic/if_gen_Scroll.h"

#include "../UnitFocus.h"

#include "if_ToolPanel.h"
#include "if_VolumePanel.h"
#include "Interface.h"
#include "SurfaceNo.h"

extern if_gen_Scroll g_ScrlUnitV;

static uint32_t _color_unit_lane[ 2 ] =
{
	0xff400070,
	0xff200040,
};

static uint32_t _color_note[ 2 ] =
{
	0xffF08000,
	0xff804000,
};

static uint32_t _color_delete = 0xffF00000;

static fRECT _rcField;

bool if_UnitField_init( const pxPalette* palpng )
{
	if( !palpng ) return false;

	if( !palpng->get_color( COLORINDEX_unit_lane_0, &_color_unit_lane[ 0 ] ) ) return false;
	if( !palpng->get_color( COLORINDEX_unit_lane_1, &_color_unit_lane[ 1 ] ) ) return false;
	if( !palpng->get_color( COLORINDEX_note_cur_0 , &_color_note     [ 0 ] ) ) return false;
	if( !palpng->get_color( COLORINDEX_note_cur_2 , &_color_note     [ 1 ] ) ) return false;
	if( !palpng->get_color( COLORINDEX_delete_0   , &_color_delete         ) ) return false;

	return true;
}

void if_UnitField_SetRect( const fRECT *rc_view )
{
	_rcField.l = rc_view->l + NAMETRAY_WIDTH;
	_rcField.r = rc_view->r - SCROLLBUTTONSIZE;
	_rcField.t = rc_view->t + HEADER_H + TOOLPANEL_HEIGHT;

	switch( if_ToolPanel_GetMode() )
	{
	case enum_ToolMode_U    :
		if( if_VolumePanel_IsUse() ) _rcField.b = rc_view->b - SCROLL_HEIGHT - VOLUME_HEIGHT;
		else                         _rcField.b = rc_view->b - SCROLL_HEIGHT;
		break;
	case enum_ToolMode_K:
		_rcField.b = rc_view->t + HEADER_H + TOOLPANEL_HEIGHT + UNIT_HEIGHT;
		break;
	default: _rcField.b = _rcField.t;
	}

	g_ScrlUnitV.SetRect( &_rcField );
}

fRECT* if_UnitField_GetRect()
{
	return &_rcField;
}

// ユニットボディを表示
void if_UnitField_Put( bool bScroll )
{
	int32_t u;
	int32_t unit_num;
	float   ypos;
	float   offset;
	int32_t ani_no;
	int32_t focus;

	fRECT rcBody;

	fRECT rc_dum     = {496,272,512,288};//{144, 96,160,112}; ..? '16/04/18

	if( bScroll ) g_ScrlUnitV.Put();
	else          g_dxdraw->tex_Put_View( _rcField.r, _rcField.t, &rc_dum, SURF_FIELDS );

	focus     = UnitFocus_Get(); if( focus < 0 ) return;
	offset    = g_ScrlUnitV.GetOffset();
	unit_num  = g_pxtn->Unit_Num();

	for( u = 0; u < unit_num; u++ )
	{
		if( UnitFocus_IsFocusedOrOperated( u ) ) ani_no = 0;
		else                                     ani_no = 1;
		ypos     = _rcField.t + u * UNIT_HEIGHT - offset;
		rcBody   = _rcField;
		rcBody.t = ypos +  1;
		rcBody.b = ypos + 16;
		if( ypos > rcBody.t - UNIT_HEIGHT && ypos < rcBody.b )
		{
			if( rcBody.b > _rcField.b ) rcBody.b = _rcField.b;
			if( rcBody.t < _rcField.t ) rcBody.t = _rcField.t;

			g_dxdraw->FillRect_view( &rcBody, _color_unit_lane[ ani_no ] );
		}
	}
}


#include "if_BaseField.h"

extern if_gen_Scroll g_ScrlEventH;
extern if_gen_Scroll g_ScrlUnitV;
extern if_gen_Scroll g_ScrlKeyV;

void if_PutUnitEvent()
{
	float ypos;
	float xpos;

	int32_t unit_num;

	fRECT rcHead[] =
	{
		{0,480,2,496},
		{0,496,2,512},
	};
	fRECT rcTail[] =
	{
		{2,480,4,496},
		{2,496,4,512},
	};

	fRECT rc_vol_event[] =
	{
		{120,496,122,498},
		{122,496,124,498},
	};
	fRECT rc;

	const EVERECORD* p_eve;
	int32_t beat_w;
	float   offset_x;
	float   offset_y;
	int32_t ani_no;
	int32_t beat_clock;
	int32_t focus;

	float   field_height;

	focus = UnitFocus_Get(); if( focus < 0 ) return;

	beat_w       = if_BaseField_GetBeatWidth();
	offset_x     = g_ScrlEventH.GetOffset();
	offset_y     = g_ScrlUnitV.GetOffset();
	field_height = _rcField.b - _rcField.t;
	beat_clock   = g_pxtn->master->get_beat_clock();
	unit_num     = g_pxtn->Unit_Num();

	for( int u = 0; u < unit_num; u++ )
	{
		if( u * UNIT_HEIGHT + UNIT_HEIGHT >= offset_y                &&
			u * UNIT_HEIGHT               <  offset_y + field_height )
		{

			ypos = _rcField.t + u * UNIT_HEIGHT + UNIT_HEIGHT/2 - offset_y;
			// 範囲

			if( int eve_num = if_BaseField_Event_get_View( u, &p_eve ) )
			{
				if( UnitFocus_IsFocusedOrOperated( u ) ) ani_no = 0;
				else                                     ani_no = 1;

				for( int e = 0; e < eve_num; e++, p_eve++ )
				{
					if( p_eve->unit_no == u )
					{
						if( p_eve->kind == EVENTKIND_ON )
						{
							xpos = ( p_eve->clock * beat_w / beat_clock ) - offset_x + _rcField.l;

							rc.l = (  p_eve->clock                 * beat_w / beat_clock ) - offset_x + _rcField.l + 2;
							rc.r = ( (p_eve->clock + p_eve->value) * beat_w / beat_clock ) - offset_x + _rcField.l - 2;
							rc.t = ypos - 2;
							rc.b = ypos + 2;
							if( rc.l < _rcField.l ) rc.l = _rcField.l;
							if( rc.r > _rcField.r ) rc.r = _rcField.r;
							if( rc.b > _rcField.b ) rc.b = _rcField.b;
							if( rc.t < _rcField.t ) rc.t = _rcField.t;
							g_dxdraw->FillRect_view( &rc, _color_note[ ani_no ] );
							if( xpos >= _rcField.l - 2 ) g_dxdraw->tex_Put_Clip( xpos, ypos - 8, &rcHead[ani_no], SURF_FIELDS, &_rcField );
							if( rc.r <  _rcField.r     ) g_dxdraw->tex_Put_Clip( rc.r, ypos - 8, &rcTail[ani_no], SURF_FIELDS, &_rcField );
						}
						else if( p_eve->kind != EVENTKIND_VELOCITY && p_eve->kind != EVENTKIND_KEY )
						{
							xpos = ( p_eve->clock * beat_w / beat_clock ) - offset_x + _rcField.l;
							g_dxdraw->tex_Put_Clip( xpos, ypos + 4, &rc_vol_event[ ani_no ], SURF_FIELDS, &_rcField );
						}
					}
				}

				if_BaseField_Event_get_View_Unlock();
			}

		}

	}
}




