
#include <pxStdDef.h>

#include <pxMem.h>

#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../ptVoice.h"

#include "./ptuiLayout.h"

#include "./ptuiRackHalf.h"

void ptuiRackHalf::_release()
{
	_b_init = false;
	pxMem_free( (void**)&_ani_nos );

	SAFE_DELETE( _knob_volume );

	_base_release();
}

ptuiRackHalf::ptuiRackHalf()
{
	_b_init       = false;
	_woice        = NULL ;

	_start_volume =     0;
	_knob_volume  = NULL ;
}

ptuiRackHalf::~ptuiRackHalf()
{
	_release();
}

bool ptuiRackHalf::init( WoiceUnit* woice )
{
	if( _b_init  ) return false;

	ptuiANCHOR anc = { 0 };
	anc.flags = ptuiANCHOR_x1_ofs|ptuiANCHOR_x2_w|ptuiANCHOR_y1_ofs|ptuiANCHOR_y2_h;
	anc.x1    = RACK_X;
	anc.x2    = RACK_W;
	anc.y1    = RACK_Y;
	anc.y2    = RACKHALF_H;

	if( !_base_init( &anc, _KNOB_num ) ) goto term;

	{
		PTUIKNOB prm;
		prm.b_vertical  = false;
		prm.zero_x      =  400;
		prm.zero_y      =   36;
		prm.volume_rate = 0.5f;
		prm.number_x    =  360;
		prm.number_y    =   32;
		_knob_volume = new ptuiKnob( &prm );
	}

	_woice    = woice   ;
	_b_init   = true    ;
term:
	return _b_init;
}

bool ptuiRackHalf::_search_button( const ptuiCursor* p_cur, int32_t* p_tgt_id ) const
{
	if( !p_tgt_id ) return false;

	const pxtnVOICEUNIT* p_vc = _woice->get_voice();

	if     ( _knob_volume->is_touch( &_rect, p_vc->volume, p_cur ) ){ *p_tgt_id = _KNOB_volume; return true; }

	return false;
}

bool ptuiRackHalf::_volume_get( _KNOB_ID id, int32_t* p_volume ) const
{
	if( !p_volume ) return false;
	const pxtnVOICEUNIT* p_vc = _woice->get_voice(); if( !p_vc ) return false;

	switch( id )
	{
	case _KNOB_volume:
		*p_volume = p_vc->volume;
		break;
	}
	return true;
}

bool ptuiRackHalf::_volume_set( _KNOB_ID id, int32_t volume )
{
	if( !_b_init ) return false;
	pxtnVOICEUNIT* p_vc = _woice->get_voice();

	uint32_t alte_flag = 0;

	switch( id )
	{
	case _KNOB_volume:

		if( volume <   0 ) volume =   0;
		if( volume >  64 ) volume =  64;
		_woice->get_voice()->volume = volume;
		alte_flag    = WOICEUNIT_ALTE_WAVE;
		break;

	default: return false;
	}

	_woice->frame_alte_set( alte_flag, -1 );
	return true;
}

bool ptuiRackHalf::_volume_default( _KNOB_ID id )
{
	switch( id )
	{
	case _KNOB_volume:

		switch( _woice->get_voice()->type )
		{
		case pxtnVOICE_Coodinate: return _volume_set( _KNOB_volume, 64 );
		case pxtnVOICE_Overtone : return _volume_set( _KNOB_volume, 32 );
		default: break;
		}
		return false;

	default: break;
	}
	return false;
}

bool ptuiRackHalf::_cursor_free(  ptuiCursor* p_cur )
{
	int32_t idx = 0;

	if( !p_cur->is_rect( _rect.l, _rect.t, _rect.r, _rect.b ) ) return false;

	if( !_search_button( p_cur, &idx ) ) return false;

	// L click.
	if( p_cur->is_trigger_left()  )
	{
		_volume_get( (_KNOB_ID)idx, &_start_volume );
		_unit_anime_set( idx, 2 );
		p_cur->set_action( this, ptuiCURACT_drag_unit, idx );
	}
	else
	{
		_unit_anime_set( idx, 1 );
	}

	return true;
}

bool ptuiRackHalf::_cursor_drag_unit(  ptuiCursor* p_cur )
{
	if( !_b_init ) return false;

	int32_t idx = 0;
	if( !p_cur->get_target_index( &idx ) ) return false;

	// drag cancel.
	if( p_cur->is_click_left() && p_cur->is_click_right() )
	{
		_volume_default( (_KNOB_ID)idx );
		_unit_anime_set( idx, 0 );
		p_cur->set_action_free();
		return true;
	}

	int32_t now_x  ;
	int32_t start_x;

	switch( idx )
	{
	case _KNOB_volume:
	{ int32_t fx; p_cur->get_pos      ( &fx, NULL ); _knob_volume->make_volume( &_rect, &now_x  , fx ); }
		{ int32_t fx; p_cur->get_pos_start( &fx, NULL ); _knob_volume->make_volume( &_rect, &start_x, fx ); }
		_volume_set( (_KNOB_ID)idx, _start_volume + ( now_x - start_x ) );
		break;

	default: return false;
	}

	// drag end.
	if( !p_cur->is_click_left() )
	{
		_unit_anime_set( idx, 0 );
		p_cur->set_action_free();
	}
	return true;
}

void ptuiRackHalf::_put( const ptuiCursor *p_cur ) const
{
	{ fRECT rc = {  0,656,576,728}; g_dxdraw->tex_Put_View( _rect.l, _rect.t, &rc, SURF_PARTS ); }

	// unit no.
	{
		fRECT rc = {136,144,208,160};
		float h  = rc.h();
		rc.t += h * _woice->channel_get();
		rc.b  = rc.t + h;
		g_dxdraw->tex_Put_View( _rect.l + 24, _rect.t +  8, &rc, SURF_PARTS );
	}

	// mode.
	{
		fRECT rc = { 16,776,296,800};
		float h  = rc.h();
		if( _woice->mode_get() == WOICEUNIT_MODE_envelope )
		{
			rc.t += h;
			rc.b  = rc.t + h;
		}
		g_dxdraw->tex_Put_View( _rect.l + 16, _rect.t + 32, &rc, SURF_PARTS );
	}

	const pxtnVOICEUNIT* p_vc   = _woice->get_voice();

	_knob_volume->put( &_rect, p_vc->volume, _ani_nos[ _KNOB_volume ] );

	if( p_vc->volume )
	{
		fRECT rc_sine = {  0,480, 16,488};
		if( _woice->get_woice()->get_instance( _woice->channel_get() )->b_sine_over )
		{
			rc_sine.l += 16;
			rc_sine.r += 16;
		}
		g_dxdraw->tex_Put_View( _rect.l + 328, _rect.t + 32, &rc_sine, SURF_PARTS );
	}
}
