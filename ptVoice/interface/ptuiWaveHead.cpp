


#include <pxMem.h>

#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../ptVoice.h"

#include "./ptuiLayout.h"

#include "./ptuiWaveHead.h"


void ptuiWaveHead::_release()
{
	_b_init = false;
	if( _btns )
	{
		for( int i = 0; i < _UNIT_num; i++ ) SAFE_DELETE( _btns[ i ] );
		pxMem_free( (void**)&_btns );
	}
	_base_release();
}

ptuiWaveHead::ptuiWaveHead()
{
	_b_init = false;
	_woice  = NULL ;
	_btns   = NULL ;
}

ptuiWaveHead::~ptuiWaveHead()
{
	_release();
}

static const fRECT _rc_btns  = {280,160,320,176};
static const fRECT _rc_close = {136, 48,216, 64};

bool ptuiWaveHead::init( WoiceUnit* woice )
{
	if( _b_init ) return false;

	ptuiANCHOR anc = { 0 };
	anc.flags = ptuiANCHOR_x1_ofs|ptuiANCHOR_x2_w|ptuiANCHOR_y1_ofs|ptuiANCHOR_y2_h;
	anc.x1    = WAVEHEAD_X;
	anc.x2    = WAVEHEAD_W;
	anc.y1    = WAVEHEAD_Y;
	anc.y2    = WAVEHEAD_H;

	if( !_base_init( &anc, _UNIT_num ) ) goto term;

	if( !pxMem_zero_alloc( (void**)&_btns, sizeof(ptuiSwitch*) * _UNIT_num ) ) goto term;

	{
		PTUISWITCH prm = {0};
		prm.x        =        16;
		prm.y        =         4;
		prm.p_rc_src = &_rc_btns;

		for( int i = 0; i < 2; i++ )
		{
			prm.img_idx = i;
			_btns[ i ] = new ptuiSwitch( &prm );
			prm.x += 48;
		}
	}

	{
		PTUISWITCH prm = {0};
		prm.x        =        240;
		prm.y        =          0;
		prm.p_rc_src = &_rc_close;
		_btns[ _UNIT_btn_close ] = new ptuiSwitch( &prm );
	}

	_woice  = woice;
	_b_init = true ;
term:
	if( !_b_init ) _release();

	return _b_init;
}

void ptuiWaveHead::_put( const ptuiCursor *p_cur ) const
{
	if( !_b_init ) return;
	const pxtnVOICEUNIT* p_vc = _woice->get_voice();

	switch( p_vc->type )
	{
	case pxtnVOICE_Coodinate:
		_btns[ _UNIT_cood ]->put( &_rect,             2 );
		_btns[ _UNIT_osci ]->put( &_rect, _ani_nos[ 1 ] );
		break;
	case pxtnVOICE_Overtone:
		_btns[ _UNIT_cood ]->put( &_rect, _ani_nos[ 0 ] );
		_btns[ _UNIT_osci ]->put( &_rect,             2 );
		break;
	}

	_btns[ _UNIT_btn_close ]->put( &_rect, _ani_nos[ _UNIT_btn_close ] );
}

bool ptuiWaveHead::_search_a_button( const ptuiCursor* p_cur, int32_t* p_idx )
{
	for( int i = 0; i < _UNIT_num; i++ )
	{
		if( _btns[ i ]->is_touch( &_rect, p_cur ) )
		{
			if( p_idx ) *p_idx = i;
			return true;
		}
	}
	return false;

}

bool ptuiWaveHead::_cursor_free( ptuiCursor* p_cur )
{
	if( !_b_init ) return false;

	int32_t idx = 0;

	if( !p_cur->is_rect( &_rect )      ) return false;

	if( _search_a_button( p_cur, &idx ) )
	{
		if( p_cur->is_trigger_left()  ){ _unit_anime_set( idx, 2 ); p_cur->set_action( this, ptuiCURACT_click_hold, idx ); }
		else                           { _unit_anime_set( idx, 1 ); }
		return true;
	}

	return false;
}

bool ptuiWaveHead::_cursor_click_hold( ptuiCursor* p_cur )
{
	int32_t idx = 0;
	if( !p_cur->get_target_index( &idx ) ) return false;

	if( p_cur->is_click_left() && p_cur->is_click_right() )
	{
		p_cur->set_action_free();
		_unit_anime_set( idx, 0 );
		return true;
	}
	else if( !p_cur->is_click_left() )
	{
		p_cur->set_action_free();

		pxtnVOICEUNIT* p_vc = _woice->get_voice(); if( !p_vc ) return false;

		switch( idx )
		{
		case _UNIT_cood     : _set_wave_tool  ( pxtnVOICE_Coodinate ); break;
		case _UNIT_osci     : _set_wave_tool  ( pxtnVOICE_Overtone  ); break;
		case _UNIT_btn_close: _woice->mode_set( WOICEUNIT_MODE_rack ); break;
		default: return false;
		}
		_unit_anime_set( idx, 0 );
	}
	else
	{
		_unit_anime_set( idx, 2 );
	}
	return true;
}

#include "../Generic/Japanese.h"

extern HWND g_hWnd_Main;
#include "../CursorKeyCtrl.h"

bool ptuiWaveHead::_set_wave_tool( pxtnVOICETYPE type )
{
	pxtnVOICEUNIT* p_vc = _woice->get_voice();
	if( type == p_vc->type ) return false;

	const char* p_msg   = "Change edit mode";
	const char* p_title = "are you sure?";

	if( Japanese_Is() )
	{
		p_msg   = "編集モードを変更します";
		p_title = "いいですか？";
	}

	if( MessageBox( g_hWnd_Main, uxT( p_msg ), uxT( p_title ), MB_OKCANCEL|MB_ICONQUESTION ) != IDOK ) return false;

	switch( type )
	{
	case pxtnVOICE_Coodinate:

		p_vc->type   = pxtnVOICE_Coodinate;
		_woice->default_coodinate( &p_vc->wave, false );
		p_vc->volume = 64;
		break;

	case pxtnVOICE_Overtone:

		p_vc->type = pxtnVOICE_Overtone;
		_woice->default_overtone(  &p_vc->wave, false );
		p_vc->volume = 32;
		break;
	default:
		return false;
	}
	_woice->frame_alte_set( WOICEUNIT_ALTE_WAVE|WOICEUNIT_ALTE_etc, -1 );
	return true;
}
