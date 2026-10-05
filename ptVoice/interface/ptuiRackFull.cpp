
#include <pxStdDef.h>

#include <pxMem.h>

#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../ptVoice.h"

#include "./ptuiLayout.h"

#include "./ptuiRackFull.h"

#define _DISABLE_H 32

void ptuiRackFull::_release()
{
	_b_init = false;
	pxMem_free( (void**)&_ani_nos );

	SAFE_DELETE( _knob_volume );
	SAFE_DELETE( _knob_pan    );
	SAFE_DELETE( _knob_tuning );

	_base_release();
}

ptuiRackFull::ptuiRackFull()
{
	_b_init       = false;
	_woice        = NULL ;
	_unit_idx     =     0;
	_start_volume =     0;
	_knob_volume  = NULL ;
	_knob_pan     = NULL ;
	_knob_tuning  = NULL ;
	_btn_mute     = NULL ;
	_btn_wave     = NULL ;
	_btn_enve     = NULL ;
	_btn_add_unit = NULL ;
}

ptuiRackFull::~ptuiRackFull()
{
	_release();
}

static const fRECT _rc_btn_mute = {200,  0,224, 16};
static const fRECT _rc_btn_edit = {136,  0,160, 16};
static const fRECT _rc_btn_add  = {160,  0,184, 16};

bool ptuiRackFull::init( WoiceUnit* woice, int32_t unit_idx )
{
	if( _b_init  ) return false;

	ptuiANCHOR anc = { 0 };
	anc.flags = ptuiANCHOR_x1_ofs|ptuiANCHOR_x2_w|ptuiANCHOR_y1_ofs|ptuiANCHOR_y2_h;
	anc.x1    = RACK_X;
	anc.x2    = RACK_W;
	anc.y1    = RACK_Y + RACK_H * unit_idx;
	anc.y2    = RACK_H;

	if( !_base_init( &anc, _UNIT_num ) ) goto term;

	{
		PTUIKNOB prm = {0};
		prm.b_vertical  = false;
		prm.zero_x      =  400;
		prm.zero_y      =   52;
		prm.volume_rate = 0.5f;
		prm.number_x    =  360;
		prm.number_y    =   48;
		_knob_volume = new ptuiKnob( &prm );
	}

	{
		PTUIKNOB prm = {0};
		prm.b_vertical  = false;
		prm.zero_x      = 400;
		prm.zero_y      =  92;
		prm.volume_rate =   1;
		prm.number_x    = 360;
		prm.number_y    =  88;
		_knob_pan = new ptuiKnob( &prm );
	}

	{
		PTUIKNOB prm = {0};
		prm.b_vertical  = false;
		prm.zero_x      = 464;
		prm.zero_y      = 124;
		prm.volume_rate =   2;
		prm.number_x    = 360;
		prm.number_y    = 120;
		_knob_tuning = new ptuiKnob( &prm );
	}

	{
		PTUISWITCH prm = {0};
		prm.p_rc_src = &_rc_btn_mute;
		prm.x        = 520;
		prm.y        =   8;
		_btn_mute    = new ptuiSwitch( &prm );
	}

	{
		PTUISWITCH prm = {0};
		prm.p_rc_src = &_rc_btn_edit;
		prm.x        =  80;
		prm.y        =  36;
		_btn_wave    = new ptuiSwitch( &prm );
	}

	{
		PTUISWITCH prm = {0};
		prm.p_rc_src = &_rc_btn_edit;
		prm.x        = 264;
		prm.y        =  36;
		_btn_enve    = new ptuiSwitch( &prm );
	}

	{
		PTUISWITCH prm = {0};
		prm.p_rc_src  = &_rc_btn_add;
		prm.x         = 520;
		prm.y         =   8;
		_btn_add_unit = new ptuiSwitch( &prm );
	}


	_woice    = woice   ;
	_unit_idx = unit_idx;
	_b_init   = true    ;
term:
	return _b_init;
}

bool ptuiRackFull::_search_a_knob( const ptuiCursor* p_cur, int32_t* p_tgt_id ) const
{
	if( !p_tgt_id ) return false;

	const pxtnVOICEUNIT* p_vc = _woice->get_voice( _unit_idx );
	int32_t tuning = 0;

	if( p_vc->tuning < 1 ) tuning = (int32_t)( ( p_vc->tuning - 1.0f ) * ptvTUNING_ACCURACY - 0.5 );
	else                   tuning = (int32_t)( ( p_vc->tuning - 1.0f ) * ptvTUNING_ACCURACY + 0.5 );

	if     ( _knob_volume->is_touch( &_rect, p_vc->volume, p_cur ) ){ *p_tgt_id = _UNIT_KNOB_volume; return true; }
	else if( _knob_pan   ->is_touch( &_rect, p_vc->pan   , p_cur ) ){ *p_tgt_id = _UNIT_KNOB_pan   ; return true; }
	else if( _knob_tuning->is_touch( &_rect, tuning      , p_cur ) ){ *p_tgt_id = _UNIT_KNOB_tuning; return true; }

	return false;
}

bool ptuiRackFull::_volume_get( _UNIT_ID id, int32_t* p_volume ) const
{
	if( !p_volume ) return false;
	const pxtnVOICEUNIT* p_vc = _woice->get_voice( _unit_idx ); if( !p_vc ) return false;

	switch( id )
	{
	case _UNIT_KNOB_volume:
		*p_volume = p_vc->volume;
		break;

	case _UNIT_KNOB_pan   :
		*p_volume = p_vc->pan   ;
		break;

	case _UNIT_KNOB_tuning:
		if( p_vc->tuning < 1 ) *p_volume = (int32_t)( ( p_vc->tuning - 1.0f ) * ptvTUNING_ACCURACY - 0.5 );
		else                   *p_volume = (int32_t)( ( p_vc->tuning - 1.0f ) * ptvTUNING_ACCURACY + 0.5 );
		break;
	}
	return true;
}

bool ptuiRackFull::_volume_set( _UNIT_ID id, int32_t volume )
{
	if( !_b_init ) return false;
	pxtnVOICEUNIT* p_vc = _woice->get_voice( _unit_idx );

	uint32_t alte_flag = 0;

	switch( id )
	{
	case _UNIT_KNOB_volume:

		if( volume <   0 ) volume =   0;
		if( volume >  64 ) volume =  64;
		_woice->get_voice( _unit_idx )->volume = volume;
		alte_flag    = WOICEUNIT_ALTE_WAVE;
		break;

	case _UNIT_KNOB_pan   :
		if( volume <    0 ) volume =    0;
		if( volume >  128 ) volume =  128;
		_woice->get_voice( _unit_idx )->pan = volume;
		alte_flag    = WOICEUNIT_ALTE_etc ;
		break;

	case _UNIT_KNOB_tuning:
		if( volume < -128 ) volume = -128;
		if( volume >  128 ) volume =  128;
		p_vc->tuning = 1.0f + (float)volume / ptvTUNING_ACCURACY;
		alte_flag    = WOICEUNIT_ALTE_etc;
		break;
	default: return false;
	}

	_woice->frame_alte_set( alte_flag, _unit_idx );
	return true;
}

bool ptuiRackFull::_volume_default( _UNIT_ID id )
{
	switch( id )
	{
	case _UNIT_KNOB_volume:

		switch( _woice->get_voice( _unit_idx )->type )
		{
		case pxtnVOICE_Coodinate: return _volume_set( _UNIT_KNOB_volume, 64 );
		case pxtnVOICE_Overtone : return _volume_set( _UNIT_KNOB_volume, 32 );
		default: break;
		}
		return false;

	case _UNIT_KNOB_pan   : return _volume_set( _UNIT_KNOB_pan   , 64 );
	case _UNIT_KNOB_tuning: return _volume_set( _UNIT_KNOB_tuning,  0 );
	default: break;
	}
	return false;
}

bool ptuiRackFull::_cursor_free(  ptuiCursor* p_cur )
{
	int32_t idx = 0;

	// not enable( add only )
	if( !is_enable() )
	{
		fRECT rc = _rect;
		rc.b = rc.t + _DISABLE_H;
		if( !p_cur->is_rect( &rc ) ) return false;
		if( _btn_add_unit->is_touch( &rc, p_cur ) )
		{
			if( p_cur->is_trigger_left()  ){ _unit_anime_set( _UNIT_BTN_add_unit, 2 ); p_cur->set_action( this, ptuiCURACT_click_hold, _UNIT_BTN_add_unit ); }
			else                           { _unit_anime_set( _UNIT_BTN_add_unit, 1 ); }
			return true;
		}
		return false;
	}

	if( !p_cur->is_rect( _rect.l, _rect.t, _rect.r, _rect.b ) ) return false;

	if( _search_a_knob( p_cur, &idx ) )
	{
		// L click.
		if( p_cur->is_trigger_left()  )
		{
			_volume_get( (_UNIT_ID)idx, &_start_volume );
			_unit_anime_set( idx, 2 );
			p_cur->set_action( this, ptuiCURACT_drag_unit, idx );
		}
		else
		{
			_unit_anime_set( idx, 1 );
		}
		return true;
	}

	if( _btn_mute->is_touch( &_rect, p_cur ) )
	{
		// L click.
		if( p_cur->is_trigger_left()  ){ _unit_anime_set( _UNIT_BTN_mute, 2 ); p_cur->set_action( this, ptuiCURACT_click_hold, _UNIT_BTN_mute ); }
		else                           { _unit_anime_set( _UNIT_BTN_mute, 1 ); }
		return true;
	}

	if( _btn_wave->is_touch( &_rect, p_cur ) )
	{
		// L click.
		if( p_cur->is_trigger_left()  ){ _unit_anime_set( _UNIT_BTN_wave, 2 ); p_cur->set_action( this, ptuiCURACT_click_hold, _UNIT_BTN_wave ); }
		else                           { _unit_anime_set( _UNIT_BTN_wave, 1 ); }
		return true;
	}

	if( _btn_enve->is_touch( &_rect, p_cur ) )
	{
		// L click.
		if( p_cur->is_trigger_left()  ){ _unit_anime_set( _UNIT_BTN_enve, 2 ); p_cur->set_action( this, ptuiCURACT_click_hold, _UNIT_BTN_enve ); }
		else                           { _unit_anime_set( _UNIT_BTN_enve, 1 ); }
		return true;
	}

	return false;
}

bool ptuiRackFull::_cursor_drag_unit(  ptuiCursor* p_cur )
{
	if( !_b_init ) return false;

	int32_t idx = 0;
	if( !p_cur->get_target_index( &idx ) ) return false;

	// drag cancel.
	if( p_cur->is_click_left() && p_cur->is_click_right() )
	{
		_volume_default( (_UNIT_ID)idx );
		_unit_anime_set( idx, 0 );
		p_cur->set_action_free();
		return true;
	}

	int32_t now_x  ;
	int32_t start_x;

	switch( idx )
	{
	case _UNIT_KNOB_volume:
	{ int32_t fx; p_cur->get_pos      ( &fx, NULL ); _knob_volume->make_volume( &_rect, &now_x  , fx ); }
		{ int32_t fx; p_cur->get_pos_start( &fx, NULL ); _knob_volume->make_volume( &_rect, &start_x, fx ); }
		_volume_set( (_UNIT_ID)idx, _start_volume + ( now_x - start_x ) );
		break;

	case _UNIT_KNOB_pan   :
	{ int32_t fx; p_cur->get_pos      ( &fx, NULL ); _knob_pan   ->make_volume( &_rect, &now_x  , fx ); }
		{ int32_t fx; p_cur->get_pos_start( &fx, NULL ); _knob_pan   ->make_volume( &_rect, &start_x, fx ); }
		_volume_set( (_UNIT_ID)idx, _start_volume + ( now_x - start_x ) );
		break;

	case _UNIT_KNOB_tuning:
	{ int32_t fx; p_cur->get_pos      ( &fx, NULL ); _knob_tuning->make_volume( &_rect, &now_x  , fx ); }
		{ int32_t fx; p_cur->get_pos_start( &fx, NULL ); _knob_tuning->make_volume( &_rect, &start_x, fx ); }
		_volume_set( (_UNIT_ID)idx, _start_volume + ( now_x - start_x ) );
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


bool ptuiRackFull::_cursor_click_hold( ptuiCursor* p_cur )
{
	int32_t idx = 0;

	if( !p_cur->get_target_index( &idx ) ) return false;

	if( p_cur->is_click_left() && p_cur->is_click_right() )
	{
		p_cur->set_action_free();
		_unit_anime_set( idx, 0 );
		return true;
	}

	if( !p_cur->is_click_left() )
	{
		p_cur->set_action_free();

		switch( idx )
		{
		case _UNIT_BTN_mute    : _woice->mute_switch( _unit_idx ); break;
		case _UNIT_BTN_wave    : _woice->mode_set   ( WOICEUNIT_MODE_wave     ); break;
		case _UNIT_BTN_enve    : _woice->mode_set   ( WOICEUNIT_MODE_envelope ); break;
		case _UNIT_BTN_add_unit: _woice->get_voice( 1 )->volume = 32; break;
		}
		_woice->channel_set( _unit_idx );
		_unit_anime_set( idx, 0 );
	}
	return true;
}

bool ptuiRackFull::is_enable        () const
{
	if( _unit_idx == 0 ) return true;
	if( _woice->get_voice( _unit_idx )->volume ) return true;
	return false;
}

void ptuiRackFull::mute_button_update()
{
	_btn_mute->set_img_idx( _woice->mute_get( _unit_idx ) ? 1 : 0 );
}


void ptuiRackFull::_put( const ptuiCursor *p_cur ) const
{
	fRECT rc_name = {136,144,208,160};
	{ float h  = rc_name.h(); rc_name.t += h * _unit_idx; rc_name.b  = rc_name.t + h; }

	if( !is_enable() )
	{
		fRECT rc_body = {  0,512,560,536};
		fRECT rc_btm  = {  0,648,560,656};
		g_dxdraw->tex_Put_View( _rect.l     , _rect.t              , &rc_body, SURF_PARTS );
		g_dxdraw->tex_Put_View( _rect.l     , _rect.t + rc_body.h(), &rc_btm , SURF_PARTS );
		g_dxdraw->tex_Put_View( _rect.l + 24, _rect.t + 8, &rc_name, SURF_PARTS );
		_btn_add_unit->put( &_rect, _ani_nos[ _UNIT_BTN_add_unit ] );
		return;
	}

	{ fRECT rc = {  0,512,576,656}; g_dxdraw->tex_Put_View( _rect.l, _rect.t, &rc, SURF_PARTS ); }

	{
		g_dxdraw->tex_Put_View( _rect.l + 24, _rect.t + 8, &rc_name, SURF_PARTS );
	}

	const pxtnVOICEUNIT* p_vc   = _woice->get_voice( _unit_idx );
	int32_t              tuning = 0;


	if( p_vc->tuning < 1 ) tuning = (int32_t)( ( p_vc->tuning - 1.0f ) * ptvTUNING_ACCURACY - 0.5 );
	else                   tuning = (int32_t)( ( p_vc->tuning - 1.0f ) * ptvTUNING_ACCURACY + 0.5 );

	_knob_volume->put( &_rect, p_vc->volume, _ani_nos[ _UNIT_KNOB_volume ] );
	_knob_pan   ->put( &_rect, p_vc->pan   , _ani_nos[ _UNIT_KNOB_pan    ] );
	_knob_tuning->put( &_rect, tuning      , _ani_nos[ _UNIT_KNOB_tuning ] );

	_btn_wave   ->put( &_rect,               _ani_nos[ _UNIT_BTN_wave    ] );
	_btn_enve   ->put( &_rect,               _ani_nos[ _UNIT_BTN_enve    ] );

	_btn_mute   ->put( &_rect,               _ani_nos[ _UNIT_BTN_mute    ] );


	if( p_vc->volume )
	{
		fRECT rc_sine = {  0,480, 16,488};
		if( _woice->get_woice()->get_instance( _woice->channel_get() )->b_sine_over )
		{
			rc_sine.l += 16;
			rc_sine.r += 16;
		}
		g_dxdraw->tex_Put_View( _rect.l + 328, _rect.t + 48, &rc_sine, SURF_PARTS );
	}
}
