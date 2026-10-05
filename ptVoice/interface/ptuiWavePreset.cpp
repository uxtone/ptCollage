
#include <pxStdDef.h>

#include <pxMem.h>

#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../../Generic/if_Generic.h"

#include "../ptVoice.h"

#include "./ptuiLayout.h"

#include "./ptuiWavePreset.h"


void ptuiWavePreset::_release()
{
	_b_init = false;
	pxMem_free( (void**)&_btns );
	_base_release();
}

ptuiWavePreset::ptuiWavePreset()
{
	_b_init = false;
	_woice  = NULL ;
	_btns   = NULL ;
}

ptuiWavePreset::~ptuiWavePreset()
{
	_release();
}

static const fRECT _rc_btns = {280, 64,296, 80};

bool ptuiWavePreset::init( WoiceUnit* woice )
{
	if( _b_init  ) return false;

	ptuiANCHOR anc = { 0 };
	anc.flags = ptuiANCHOR_x1_ofs|ptuiANCHOR_x2_w|ptuiANCHOR_y1_ofs|ptuiANCHOR_y2_h;
	anc.x1    = WAVEPRESET_X;
	anc.x2    = WAVEPRESET_W;
	anc.y1    = WAVEPRESET_Y;
	anc.y2    = WAVEPRESET_H;

	if( !_base_init( &anc, ptvWAVEPRESET_num ) ) goto term;

	if( !pxMem_zero_alloc( (void**)&_btns, sizeof(ptuiSwitch*) * ptvWAVEPRESET_num ) ) goto term;

	{
		PTUISWITCH prm = {0};
		prm.img_idx  =         0;
		prm.x        =        64;
		prm.y        =         8;
		prm.p_rc_src = &_rc_btns;

		for( int i = 0; i < ptvWAVEPRESET_num; i++ )
		{
			prm.img_idx = i;
			_btns[ i ]  = new ptuiSwitch( &prm );
			prm.x += 16;
		}
	}
	_woice = woice;

	_b_init  = true   ;
term:
	return _b_init;
}

bool ptuiWavePreset::_search_button( const ptuiCursor* p_cur, int32_t* p_idx ) const
{
	for( int i = 0; i < ptvWAVEPRESET_num; i++ )
	{
		if( _btns[ i ]->is_touch( &_rect, p_cur ) )
		{
			if( p_idx ) *p_idx = i;
			return true;
		}
	}
	return false;
}

bool ptuiWavePreset::_cursor_free(  ptuiCursor* p_cur )
{
	int32_t idx = 0;

	if( !p_cur->is_rect( _rect.l, _rect.t, _rect.r, _rect.b ) ) return false;

	if( !_search_button( p_cur, &idx ) ) return false;

	// L click.
	if( p_cur->is_trigger_left()  )
	{
		_unit_anime_set( idx, 2 );
		p_cur->set_action( this, ptuiCURACT_click_hold, idx );
	}
	else
	{
		_unit_anime_set( idx, 1 );
	}

	return true;
}

bool ptuiWavePreset::_cursor_click_hold(  ptuiCursor* p_cur )
{
	if( !_b_init ) return false;

	int32_t idx = 0;
	if( !p_cur->get_target_index( &idx ) ) return false;

	// drag cancel.
	if( p_cur->is_click_left() && p_cur->is_click_right() )
	{
		_unit_anime_set( idx, 0 );
		p_cur->set_action_free();
		return true;
	}

	// drag end.
	if( !p_cur->is_click_left() )
	{
		_unit_anime_set( idx, 0 );
		p_cur->set_action_free();
		_do_preset( (ptvWAVEPRESET)idx );
		_woice->strm_ready_sample();
	}
	return true;
}

void ptuiWavePreset::_put( const ptuiCursor *p_cur ) const
{
	fRECT   rcButton = {168, 32,184, 48};


	{ fRECT rc = {280,112,464,144}; g_dxdraw->tex_Put_View( _rect.l, _rect.t, &rc, SURF_PARTS ); }
/*
	fRECT   rc;
	{
		fRECT rc_t = {248,  0,280,  8};
		fRECT rc_m = {248,  8,280,152};
		fRECT rc_b = {248,152,280,160};
		rc = _rect; rc.t += 8; rc.b -= 8;
		if_gen_tile_v( &rc, &rc_m, 1, 0, SURF_PARTS );
		g_dxdraw->tex_Put_View( _rect.l, _rect.t  , &rc_t, SURF_PARTS );
		g_dxdraw->tex_Put_View( _rect.l, _rect.b-8, &rc_b, SURF_PARTS );
	}
*/
	for( int i = 0; i < ptvWAVEPRESET_num; i++ )
	{
		_btns[ i ]->put( &_rect, _ani_nos[ i ] );
	}
}









static int32_t _Random( int32_t min, int32_t max )
{
	int32_t    range = max - min + 1;
	SYSTEMTIME st    = { 0 };

	GetLocalTime( &st );

	for( int a = 0; a < st.wSecond%10; a++ ) rand();

	return( ( rand() % range ) + min );
}

bool ptuiWavePreset::_module_overtone ( pxtnVOICEUNIT *p_vc, ptvWAVEPRESET index )
{
	int32_t s;
	float   v;
	int32_t i;
	int32_t work;

	switch( index )
	{
	case ptvWAVEPRESET_Sine:
		p_vc->volume             =  32;
		p_vc->wave.points[ 0 ].y = 128;
		for( s = 1; s < p_vc->wave.num; s++ ) p_vc->wave.points[ s ].y = 0;
		break;

	case ptvWAVEPRESET_Triangle:
		p_vc->volume = 28;
		v = 128;
		i =   0;
		for( s = 0; s < ptvFIXNUM_WAVE_POINT; s++ )
		{
			if( s % 2 ){
				p_vc->wave.points[ s ].y = 0;
			}else{
				if( i % 2 ) p_vc->wave.points[ s ].y = -(int32_t)v;
				else        p_vc->wave.points[ s ].y =  (int32_t)v;
				v = v / 3;
				i++;
			}
		}
		break;

	case ptvWAVEPRESET_Sow:
		p_vc->volume = 16;
		for( s = 0; s < ptvFIXNUM_WAVE_POINT; s++ ) p_vc->wave.points[ s ].y = 128;
		break;

	case ptvWAVEPRESET_Rectangle:
		p_vc->volume = 24;
		for( s = 0; s < ptvFIXNUM_WAVE_POINT; s++ ){
			if( s % 2 ) p_vc->wave.points[ s ].y =   0;
			else        p_vc->wave.points[ s ].y = 128;
		}
		break;

	case ptvWAVEPRESET_Random:
		p_vc->volume = 28;
		work = _Random( 1, ptvFIXNUM_WAVE_POINT - 1 );
		for( s = 0; s < work; s++ ){
			p_vc->wave.points[ s ].y = _Random( -128, 128 );
		}
		for(      ; s < ptvFIXNUM_WAVE_POINT; s++ ) p_vc->wave.points[ s ].y  =  0;
		break;

	case ptvWAVEPRESET_Reverse:
		for( s = 0; s < ptvFIXNUM_WAVE_POINT; s++ ) p_vc->wave.points[ s ].y *= -1;
		break;

	default: return false;
	}
	return true;
}

bool ptuiWavePreset::_module_coodinate( pxtnVOICEUNIT *p_vc, ptvWAVEPRESET index )
{
	int32_t   s  =  0 ;
	int32_t   t  =  0 ;
	pxtnPOINT pt = {0};

	switch( index )
	{
	case ptvWAVEPRESET_Sine:
		p_vc->volume             =  64;
		for( s = 0; s < ptvFIXNUM_WAVE_POINT; s++ ) p_vc->wave.points[ s ].y = 0;
		s = 1;
		p_vc->wave.points[ s ].x =  16; p_vc->wave.points[ s ].y =  33; s++;
		p_vc->wave.points[ s ].x =  32; p_vc->wave.points[ s ].y =  52; s++;
		p_vc->wave.points[ s ].x =  50; p_vc->wave.points[ s ].y =  61; s++;
		p_vc->wave.points[ s ].x =  64; p_vc->wave.points[ s ].y =  64; s++;
		p_vc->wave.points[ s ].x =  80; p_vc->wave.points[ s ].y =  60; s++;
		p_vc->wave.points[ s ].x =  96; p_vc->wave.points[ s ].y =  52; s++;
		p_vc->wave.points[ s ].x = 110; p_vc->wave.points[ s ].y =  34; s++;
		p_vc->wave.points[ s ].x = 141; p_vc->wave.points[ s ].y = -31; s++;
		p_vc->wave.points[ s ].x = 160; p_vc->wave.points[ s ].y = -52; s++;
		p_vc->wave.points[ s ].x = 174; p_vc->wave.points[ s ].y = -60; s++;
		p_vc->wave.points[ s ].x = 192; p_vc->wave.points[ s ].y = -64; s++;
		p_vc->wave.points[ s ].x = 208; p_vc->wave.points[ s ].y = -59; s++;
		p_vc->wave.points[ s ].x = 224; p_vc->wave.points[ s ].y = -52; s++;
		p_vc->wave.points[ s ].x = 242; p_vc->wave.points[ s ].y = -33; s++;
		p_vc->wave.num = s;
		break;
	case ptvWAVEPRESET_Triangle:
		p_vc->volume             =  64;
		for( s = 0; s < ptvFIXNUM_WAVE_POINT; s++ ) p_vc->wave.points[ s ].y = 0;
		p_vc->wave.points[ 1 ].x =  64; p_vc->wave.points[ 1 ].y =   64;
		p_vc->wave.points[ 2 ].x = 192; p_vc->wave.points[ 2 ].y =  -64;
		p_vc->wave.num = 3;
		break;
	case ptvWAVEPRESET_Sow:
		p_vc->volume             =  64;
		for( s = 0; s < ptvFIXNUM_WAVE_POINT; s++ ) p_vc->wave.points[ s ].y = 0;
		p_vc->wave.points[ 1 ].x =   0; p_vc->wave.points[ 1 ].y =   32;
		p_vc->wave.points[ 2 ].x = 255; p_vc->wave.points[ 2 ].y =  -32;
		p_vc->wave.num = 3;
		break;
	case ptvWAVEPRESET_Rectangle:
		p_vc->volume             =  64;
		for( s = 0; s < ptvFIXNUM_WAVE_POINT; s++ ) p_vc->wave.points[ s ].y = 0;
		p_vc->wave.points[ 1 ].x =   0; p_vc->wave.points[ 1 ].y =   32;
		p_vc->wave.points[ 2 ].x = 128; p_vc->wave.points[ 2 ].y =   32;
		p_vc->wave.points[ 3 ].x = 128; p_vc->wave.points[ 3 ].y =  -32;
		p_vc->wave.points[ 4 ].x = 255; p_vc->wave.points[ 4 ].y =  -32;
		p_vc->wave.num = 5;
		break;

	case ptvWAVEPRESET_Random:
		p_vc->volume   = 64;
		p_vc->wave.num = _Random( 1, ptvFIXNUM_WAVE_POINT - 1 );
		for( s = 1; s < p_vc->wave.num; s++ ){
			p_vc->wave.points[ s ].x = _Random(    1, 199 );
			p_vc->wave.points[ s ].y = _Random( -100, 100 );
		}
		t = p_vc->wave.num;
		while( t > 1 )
		{
			for( s = 1; s < t - 1; s++ )
			{
				if( p_vc->wave.points[ s ].x > p_vc->wave.points[ s + 1 ].x ){
					pt                         = p_vc->wave.points[ s     ];
					p_vc->wave.points[ s     ] = p_vc->wave.points[ s + 1 ];
					p_vc->wave.points[ s + 1 ] = pt;
				}
			}
			t--;
		}

		break;

	case ptvWAVEPRESET_Reverse:
		for( s = 0; s < ptvFIXNUM_WAVE_POINT; s++ ) p_vc->wave.points[ s ].y *= -1;
		break;

	default: return false;
	}

	return true;
}

void ptuiWavePreset::_do_preset( ptvWAVEPRESET index )
{
	pxtnVOICEUNIT *p_vc = _woice->get_voice();

	if( !p_vc ) return;

	switch( p_vc->type )
	{
	case pxtnVOICE_Coodinate: _module_coodinate( p_vc, index ); break;
	case pxtnVOICE_Overtone : _module_overtone ( p_vc, index ); break;
	}
	_woice->frame_alte_set( WOICEUNIT_ALTE_WAVE, -1 );
}
