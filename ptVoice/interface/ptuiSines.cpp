
#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;


#include "../../Generic/if_Generic.h"

#include "../ptVoice.h"

#include "./ptuiLayout.h"


#define _VOLUME_RATE  1.0f

#define _KNOB_WIDTH     40
#define _KNOB_HEIGHT    14

#define _VOLUMENO_X     24
#define _VOLUMENO_Y      8

#define _VOLUMEZERO_X (64+128)
#define _VOLUMEZERO_Y   12

#define _SLIDER_W      200
#define _SLIDER_H       24


#include "./ptuiSines.h"

void ptuiSines::_release()
{
	_b_init = false;
	SAFE_DELETE( _scrl_v );
	_base_release();
}

ptuiSines::ptuiSines()
{
	_b_init       = false;
	_start_volume =     0;
	_scrl_v       = NULL ;
}

ptuiSines::~ptuiSines()
{
	_release();
}

bool ptuiSines::init( WoiceUnit* woice )
{
	if( _b_init ) return false;

	ptuiANCHOR anc = { 0 };
	anc.flags = ptuiANCHOR_x1_ofs|ptuiANCHOR_x2_w|ptuiANCHOR_y1_ofs|ptuiANCHOR_y2_ofs;
	anc.x1    = SINEVOLUME_X;
	anc.x2    = SINEVOLUME_W;
	anc.y1    = SINEVOLUME_Y;
	anc.y2    =            0;

	if( !_base_init( &anc, ptvFIXNUM_WAVE_POINT ) ) goto term;

	_scrl_v = new if_gen_Scroll();
	_scrl_v->Initialize( g_dxdraw, SURF_PARTS, true );
	_scrl_v->SetSize   ( _SLIDER_H * ptvFIXNUM_WAVE_POINT );
	_scrl_v->SetMove   ( _SLIDER_H, _SLIDER_H    );

	_woice  = woice;
	_b_init = true ;
term:
	if( !_b_init ) _release();
	return _b_init;
}

void ptuiSines::_put( const ptuiCursor* p_cur ) const
{
	fRECT rcSlider = {  0,728,344,776};
	fRECT rcBottom = {  0,248,232,264};

	fRECT rcKnob[] =
	{
		{ 48,448, 88,464},
		{ 88,448,128,464},
		{ 48,448, 88,464},
	};

	int32_t n;
	float   x, y, offset_y = _scrl_v->GetOffset();
	int32_t volume;

	const pxtnVOICEUNIT *p_vc = _woice->get_voice();

	if_gen_tile_v( &_rect, &rcSlider, 1, offset_y, SURF_PARTS );

	_scrl_v->Put();

	g_dxdraw->tex_Put_View( _rect.l - 16, _rect.b, &rcBottom, SURF_PARTS );

	_scrl_v->Put();

	for( n = 0; n < p_vc->wave.num; n++ )
	{
		volume = p_vc->wave.points[ n ].y;
		x = _rect.l + _VOLUMEZERO_X + volume / _VOLUME_RATE - _KNOB_WIDTH  / 2;
		y = _rect.t + _VOLUMEZERO_Y + _SLIDER_H * n - _KNOB_HEIGHT / 2 - offset_y;
		g_dxdraw->tex_Put_Clip( x, y, &rcKnob[ _ani_nos[ n ] ], SURF_PARTS, &_rect );

		x += 12;
		y +=  3;
		if_gen_num6_clip( x, y, volume, 3, &_rect );

		x = _rect.l + _VOLUMENO_X;
		y = _rect.t + _VOLUMENO_Y    + _SLIDER_H * n - offset_y;
		if_gen_num6_clip( x, y, n + 1,      2, &_rect );
	}
}

bool ptuiSines::_search_button( const ptuiCursor* p_cur, int32_t* p_tgt_id ) const
{
	int32_t        volume = 0;
	const pxtnVOICEUNIT* p_vc   = _woice->get_voice(); if( !p_vc ) return false;

	int32_t idx = 0;
	int32_t fx  = 0;
	int32_t fy  = 0;

	p_cur->get_pos( &fx, &fy );

	_pos_make( NULL, &idx, 0, fy );
	if( idx < 0 || idx >= ptvFIXNUM_WAVE_POINT ) return false;

	int32_t knob_x = _rect.l + _VOLUMEZERO_X + p_vc->wave.points[ idx ].y/_VOLUME_RATE;
	int32_t knob_y = _rect.t + (_SLIDER_H * idx) - _scrl_v->GetOffset();

	if( fx >= knob_x - _KNOB_WIDTH  / 2 &&
		fy >= knob_y +                4 &&
		fx <  knob_x + _KNOB_WIDTH  / 2 &&
		fy <  knob_y + 4 + 16 )
	{
		if( p_tgt_id ) *p_tgt_id = idx;
		return true;
	}
	return false;
}

bool ptuiSines::_get_volume_int( int32_t idx, int32_t* p_volume ) const
{
	if( !p_volume ) return false;
	if( idx < 0 || idx >= ptvFIXNUM_WAVE_POINT ) return false;
	const pxtnVOICEUNIT* p_vc   = _woice->get_voice(); if( !p_vc ) return false;
	*p_volume = p_vc->wave.points[ idx ].y;
	return true;
}

bool ptuiSines::_set_volume_int( int32_t idx, int32_t    volume )
{
	if( !_b_init ) return false;
	if( idx < 0 || idx >= ptvFIXNUM_WAVE_POINT ) return false;
	pxtnVOICEUNIT* p_vc   = _woice->get_voice(); if( !p_vc ) return false;

	if( volume < -128 ) volume = -128;
	if( volume >  128 ) volume =  128;

	p_vc->wave.points[ idx ].y = volume;

	_woice->frame_alte_set( WOICEUNIT_ALTE_WAVE, -1 );
	return true;
}

bool ptuiSines::_set_volume_default( int32_t idx )
{
	return _set_volume_int( idx, 0 );
}

void ptuiSines::_pos_make( int32_t *px, int32_t* py, int32_t cur_x, int32_t cur_y ) const
{
	if( px ) *px = (int32_t)( ( cur_x - (_rect.l + _VOLUMEZERO_X)   )     * _VOLUME_RATE );
	if( py ) *py = (int32_t)( ( cur_y -  _rect.t + _scrl_v->GetOffset() ) / _SLIDER_H    );
}


bool   ptuiSines::_proc_begin_sub   ()
{
	_scrl_v->SetRect( &_rect );
	return true;
}


bool ptuiSines::_cursor_free( ptuiCursor* p_cur )
{
	int32_t idx = 0;

	{
		int32_t fx, fy; p_cur->get_pos( &fx, &fy );
		if( _scrl_v->Action( (int32_t)fx, (int32_t)fy, p_cur->is_click_left(), p_cur->is_trigger_left() ) )
		{
			p_cur->set_action( this, ptuiCURACT_scroll_v, 0 );
			return true;
		}
	}

	if( _woice->get_voice()->type != pxtnVOICE_Overtone ) return false;

	if( !p_cur->is_rect( &_rect )      ) return false;
	if( !_search_button( p_cur, &idx ) ) return false;

	if( p_cur->is_trigger_left() )
	{
		_get_volume_int( idx, &_start_volume );
		p_cur->set_action( this, ptuiCURACT_drag_unit , idx );
		_unit_anime_set( idx, 2 );
	}
	else if( p_cur->is_trigger_right() )
	{
		p_cur->set_action( this, ptuiCURACT_click_hold, idx );
		_set_volume_default( idx );
		_unit_anime_set( idx, 2 );
	}
	else
	{
		_unit_anime_set( idx, 1 );
	}

	return true;
}

bool   ptuiSines::_cursor_scroll_v  ( ptuiCursor* p_cur )
{
	int32_t fx, fy; p_cur->get_pos( &fx, &fy );
	if( !_scrl_v->Action( (int32_t)fx, (int32_t)fy, p_cur->is_click_left(), p_cur->is_trigger_left() ) )
	{
		p_cur->set_action_free();
	}
	return true;
}

bool ptuiSines::_cursor_drag_unit( ptuiCursor* p_cur )
{
	int32_t idx = 0;
	p_cur->get_target_index( &idx );

	if( p_cur->is_click_left() && p_cur->is_click_right() )
	{
		_set_volume_int( idx, _start_volume );
		_unit_anime_set( idx, 0 );
		p_cur->set_action_free();
		return true;
	}
	else
	{
		int32_t now_x  ;
		int32_t start_x;
		{ int32_t fx; p_cur->get_pos      ( &fx, NULL ); _pos_make( &now_x  , NULL, fx, 0 ); }
		{ int32_t fx; p_cur->get_pos_start( &fx, NULL ); _pos_make( &start_x, NULL, fx, 0 ); }
		_set_volume_int( idx, _start_volume + ( now_x - start_x ) );

		if( !p_cur->is_click_left() )
		{
			_unit_anime_set( idx, 0 );
			p_cur->set_action_free();
			_woice->frame_alte_set( WOICEUNIT_ALTE_WAVE, -1 );
		}
	}
	return true;
}

bool ptuiSines::_cursor_click_hold( ptuiCursor* p_cur )
{
	int32_t idx = 0;
	p_cur->get_target_index( &idx );

	if( p_cur->is_click_left() && p_cur->is_click_right() )
	{
		_set_volume_int( idx, _start_volume );
		_unit_anime_set( idx, 0 );
		p_cur->set_action_free();
		return true;
	}

	if( !p_cur->is_click_left() )
	{
		_unit_anime_set( idx, 0 );
		p_cur->set_action_free();
	}
	return true;
}
