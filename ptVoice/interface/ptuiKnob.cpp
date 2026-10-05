
#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../../Generic/if_Generic.h"

#include "../ptVoice.h"

#include "./ptuiKnob.h"

ptuiKnob::ptuiKnob( const PTUIKNOB* prm )
{
	_prm = *prm;
}

bool ptuiKnob::make_volume( const fRECT* rc_prnt, int32_t* pv, int32_t cur_v ) const
{
	if( _prm.b_vertical ) *pv = (int32_t)(         (rc_prnt->t + _prm.zero_y) - cur_v ) * _prm.volume_rate;
	else                  *pv = (int32_t)( cur_v - (rc_prnt->l + _prm.zero_x)         ) * _prm.volume_rate;
	return true;
}

bool ptuiKnob::is_touch( const fRECT* rc_prnt, int32_t volume, const ptuiCursor* p_cur ) const
{
	int32_t x = 0;
	int32_t y = 0;

	if( _prm.b_vertical )
	{
		x = rc_prnt->l + _prm.zero_x;
		y = rc_prnt->t + _prm.zero_y - volume/_prm.volume_rate;
	}
	else
	{
		x = rc_prnt->l + _prm.zero_x + volume/_prm.volume_rate;
		y = rc_prnt->t + _prm.zero_y;
	}

	return p_cur->is_rect(
		x - _knob_w / 2,
		y - _knob_h / 2,
		x + _knob_w / 2,
		y + _knob_h / 2 );
}

void ptuiKnob::put( const fRECT* rc_prnt, int32_t volume, int32_t ani_no ) const
{
	float   x, y;

	if( _prm.b_vertical )
	{
		fRECT rcKnob[] =
		{
			{  0,448,16,464},
			{ 16,448,32,464},
			{ 32,448,48,464},
		};
		x      = rc_prnt->l + _prm.zero_x                           - _knob_w / 2;
		y      = rc_prnt->t + _prm.zero_y - volume/_prm.volume_rate - _knob_h / 2;
		g_dxdraw->tex_Put_Clip( x, y, &rcKnob[ ani_no ], SURF_PARTS, rc_prnt );
	}
	else
	{
		fRECT rcKnob[] =
		{
			{ 0,464,16,477},
			{16,464,32,477},
			{32,464,48,477},
		};
		x      = rc_prnt->l + _prm.zero_x + volume/_prm.volume_rate - _knob_w / 2;
		y      = rc_prnt->t + _prm.zero_y                           - _knob_h / 2;
		g_dxdraw->tex_Put_Clip( x, y, &rcKnob[ ani_no ], SURF_PARTS, rc_prnt );
	}

	x = rc_prnt->l + _prm.number_x;
	y = rc_prnt->t + _prm.number_y;
	if_gen_num6_clip( x, y, volume, 3, rc_prnt );
}
