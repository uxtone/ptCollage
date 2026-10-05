

#include <pxStdDef.h>

#include <pxMem.h>

#include <pxtnEvelist.h>

#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include <pxtnPulse_Frequency.h>
extern pxtnPulse_Frequency* g_freq;

#include "../../Generic/if_Generic.h"
#include "../../Generic/KeyControl.h"

#include "./ptuiLayout.h"
#include "./ptuiOrganKey.h"

#define _MAX_KEYTOUCH 32

static const int32_t _key_table[ _MAX_KEYTOUCH ] =
{
	'Z', 'S', 'X',
	'C', 'F', 'V', 'G', 'B', 'N', 'J', 'M', 'K', 0xbc, 'L', 0xbe,
	'Q', 0x32, 'W', 0x33, 'E', 'R', 0x35, 'T', 0x36, 'Y', 0x37, 'U',
	'I', 0x39, 'O', 0x30, 'P'
};

void ptuiOrganKey::_release()
{
	_b_init = false;
	_base_release();
	SAFE_DELETE( _scrl_v );
	pxMem_free( (void**)&_keys );
}

ptuiOrganKey::ptuiOrganKey()
{
	_b_init = false;
	memset( _push_flag, 0, ptvORGANKEY_KEYNUM / 8 );
	_selected_octave =   -1;
	_scrl_v          = NULL;
	_keys            = NULL;

	_key_index_offset = 12 * 4;

	_tone_last_key    = 12 * 4;
	_tone_last_id     =      0;

}

ptuiOrganKey::~ptuiOrganKey()
{
	_release();
}

bool ptuiOrganKey::init( WoiceUnit* woice )
{
	if( _b_init ) return false;

	ptuiANCHOR anc = { 0 };
	anc.flags = ptuiANCHOR_x1_w|ptuiANCHOR_x2_ofs|ptuiANCHOR_y1_ofs|ptuiANCHOR_y2_ofs;
	anc.x1    = ORGANKEY_W      ;
	anc.x2    = SCROLLBUTTONSIZE;
	anc.y1    =                0;
	anc.y2    = VELOCITYVOLUME_H;

	if( !_base_init( &anc, 1 ) ) goto term;

	_scrl_v = new if_gen_Scroll();
	_scrl_v->Initialize( g_dxdraw, SURF_PARTS, true );
	_scrl_v->SetSize  ( ORGANKEY_KEYHEIGHT * ptvORGANKEY_KEYNUM );
	_scrl_v->SetMove  ( ORGANKEY_KEYHEIGHT, ORGANKEY_KEYHEIGHT  );
	_scrl_v->SetOffset( ORGANKEY_KEYHEIGHT * 36 / 2  );

	if( !pxMem_zero_alloc( (void**)&_keys, sizeof(ptuiORGKEY) * _MAX_KEYTOUCH ) ) goto term;

	for( int32_t k = 0; k < _MAX_KEYTOUCH; k++ ) _keys[ k ].code = _key_table[ k ];

	_woice  = woice;
	_b_init = true ;
term:
	if( !_b_init ) _release();
	return _b_init;
}


bool ptuiOrganKey::key_proc( WoiceUnit* woice )
{
	bool bDraw = false;

	for( int32_t k = 0; k < _MAX_KEYTOUCH; k++ )
	{
		if( _keys[ k ].bON )
		{
			if( !KeyControl_IsKey( _keys[ k ].code ) )
			{
				woice->strm_OFF( _keys[ k ].id, false );
				key_push( k + _key_index_offset, false );
				_keys[ k ].bON = false;
				bDraw = true;
			}
		}
		else
		{
			if( KeyControl_IsKeyTrigger( _keys[ k ].code ) )
			{
				float freq_rate = g_freq->Get( ( k + _key_index_offset + 0x15 ) * 0x100  - EVENTDEFAULT_BASICKEY );

				key_push( k + _key_index_offset, true );

				_keys[ k ].bON = true;
				_keys[ k ].id  = woice->strm_ON( freq_rate );
				bDraw = true;
			}
		}
	}

	return bDraw;
}

bool ptuiOrganKey::key_reset( WoiceUnit* woice )
{
	if( !_b_init ) return false;

	key_clear();

	for( int32_t k = 0; k < _MAX_KEYTOUCH; k++ )
	{
		if( _keys[ k ].bON )
		{
			woice->strm_OFF( _keys[ k ].id, true );
			_keys[ k ].bON = false;
		}
	}
	return true;
}

bool ptuiOrganKey::key_octave_shift( bool b_down )
{
	int32_t old = _key_index_offset;

	if( b_down ) _key_index_offset -= 12;
	else         _key_index_offset += 12;

	if( _key_index_offset <  0 ) _key_index_offset =  0;
	if( _key_index_offset > 60 ) _key_index_offset = 60;

	if( old == _key_index_offset ) return false;

	return true;
}

bool ptuiOrganKey::key_octave_set( int32_t oct )
{
	int32_t old = _key_index_offset;
	_key_index_offset = oct * 12;
	if( _key_index_offset <  0 ) _key_index_offset =  0;
	if( _key_index_offset > 60 ) _key_index_offset = 60;
	if( old == _key_index_offset ) return false;

	return true;
}

void ptuiOrganKey::_put( const ptuiCursor* p_cur) const
{
	int32_t offset_y = _scrl_v->GetOffset();
	float   y        = 0;

	fRECT rc_oct = {256,256,336,352};

	if_gen_tile_v ( &_rect, &rc_oct, 1, offset_y, SURF_PARTS ); // keyboard.

	_scrl_v->Put();

	// key numbers
	int32_t key_height;

	int32_t i;

	key_height = (int32_t)_rect.h();

	for( i = 0; i <= 8; i++ )
	{
		y = _rect.t + ( i * (ORGANKEY_KEYHEIGHT*12) ) - offset_y + 0;
		if( y >= _rect.t - 12 && y <=  _rect.b + 0 )
		{
			if_gen_num6_clip( _rect.l+16, y, 7 - i, 2, &_rect );
		}
	}

	// プッシュ
	fRECT rcPush  = {336,256,380,264};

	for( i = 0; i < 88; i++ )
	{
		if( _push_flag[ i / 8 ] & ( 0x80 >> (i%8) ) )
		{
			fRECT rc;
			rc = rcPush;
			rc.t += ( i % 12 ) * ORGANKEY_KEYHEIGHT;
			rc.b  = rc.t       + ORGANKEY_KEYHEIGHT;
			g_dxdraw->tex_Put_Clip( _rect.l +  36, i * ORGANKEY_KEYHEIGHT - offset_y + _rect.t, &rc,  SURF_PARTS, &_rect );
		}
	}

	// 基本キー
	fRECT rcBasic = {384,256,392,264};
	int32_t  key;

	key = 88 - ( EVENTDEFAULT_BASICKEY / 0x100 - 0x14 );
	g_dxdraw->tex_Put_Clip( _rect.l + 72, key * ORGANKEY_KEYHEIGHT - offset_y + _rect.t, &rcBasic, SURF_PARTS, &_rect );


	// KeyTouch
	fRECT rcTouch[] =
	{
		{392,256,400,352},
		{400,256,408,352},
		{408,256,416,352},
	};

	key = 40 - _key_index_offset + 16;
	for( i = 0; i < 3; i++ )
	{
		g_dxdraw->tex_Put_Clip( _rect.l + 14, key * ORGANKEY_KEYHEIGHT - offset_y + _rect.t + 96 * i, &rcTouch[ i ], SURF_PARTS, &_rect );
	}

	// 選択
	if( _selected_octave != -1 )
	{
		fRECT rcSelect = {416,256,432,352};
		y = ( 85 - ( _selected_octave * 12 ) - 12 ) * ORGANKEY_KEYHEIGHT + _rect.t - offset_y;
		g_dxdraw->tex_Put_Clip( _rect.l + 13, y, &rcSelect,  SURF_PARTS, &_rect );
	}

}

void ptuiOrganKey::key_push( int32_t key, bool bOn )
{
	int32_t bit;
	bit = 87 - key;// - ( key / 0x100 - 0x2f );
	if( bOn ) _push_flag[ bit / 8 ] |=  ( 0x80 >> (bit%8) );
	else      _push_flag[ bit / 8 ] &= ~( 0x80 >> (bit%8) );
}

void ptuiOrganKey::key_clear()
{
	memset( _push_flag, 0, ptvORGANKEY_KEYNUM / 8 );
}

void ptuiOrganKey::octave_set( int32_t oct )
{
}

void ptuiOrganKey::_volume_make( int32_t *py, int32_t cur_y ) const
{
	*py = cur_y - (int32_t)_rect.t + _scrl_v->GetOffset();
}

bool   ptuiOrganKey::_proc_begin_sub   ()
{
	_scrl_v->SetRect( &_rect );
	return true;
}


bool ptuiOrganKey::_cursor_free( ptuiCursor* p_cur )
{
	float   freq_rate;
	int32_t ypos_on_key = 0;

	_selected_octave = -1;

	{
		int32_t fx, fy; p_cur->get_pos( &fx, &fy );
		if( _scrl_v->Action( (int32_t)fx, (int32_t)fy, p_cur->is_click_left(), p_cur->is_trigger_left() ) )
		{
			p_cur->set_action( this, ptuiCURACT_scroll_v, 0 );
			return true;
		}
	}

	if( KeyControl_IsKeyTrigger( VK_SPACE ) )
	{
		_woice->strm_OFF( _tone_last_id, true );

		freq_rate = g_freq->Get( ( _tone_last_key + 0x15 ) * 0x100 - EVENTDEFAULT_BASICKEY );
		_tone_last_id  = _woice->strm_ON( freq_rate );

		p_cur->set_action( this, ptuiCURACT_key_hold, 0 );

		key_clear();
		key_push ( _tone_last_key, true );
		return true;
	}

	{ int32_t cur_y = 0; p_cur->get_pos( NULL, &cur_y ); _volume_make( &ypos_on_key, cur_y ); }

	if( p_cur->is_rect( _rect.l+ 36, _rect.t, _rect.r, _rect.b ) )
	{

		if( p_cur->is_trigger_left()  )
		{
			_tone_last_key = 87 - ( ypos_on_key / ORGANKEY_KEYHEIGHT );
			key_clear();
			key_push ( _tone_last_key, true );
			_woice->strm_OFF( _tone_last_id, true );
			freq_rate       = g_freq->Get( ( _tone_last_key + 0x15 ) * 0x100 - EVENTDEFAULT_BASICKEY );
			_tone_last_id        = _woice->strm_ON( freq_rate );
			p_cur->set_action( this, ptuiCURACT_keyboard, 0 );
			return true;
		}
	}
	else if( p_cur->is_rect( _rect.l+ 13, _rect.t, _rect.l+25, _rect.b ) )
	{

		int32_t oct = ( 84 - (ypos_on_key / ORGANKEY_KEYHEIGHT) ) / 12;
		if( oct < 0 ) oct = -1;
		if( oct > 5 ) oct = -1;
		_selected_octave = oct;

		if( p_cur->is_trigger_left()  )
		{
			key_octave_set( oct );
			p_cur->set_action( this, ptuiCURACT_click_hold, 0 );
			return true;
		}
	}
	return false;
}

bool   ptuiOrganKey::_cursor_scroll_v  ( ptuiCursor* p_cur )
{
	int32_t fx, fy; p_cur->get_pos( &fx, &fy );
	if( !_scrl_v->Action( (int32_t)fx, (int32_t)fy, p_cur->is_click_left(), p_cur->is_trigger_left() ) )
	{
		p_cur->set_action_free();
	}
	return true;
}

bool ptuiOrganKey::_cursor_keyboard( ptuiCursor* p_cur )
{
	// 周波数変更
	{
		int32_t ypos_on_key = 0;
		{ int32_t cur_y = 0; p_cur->get_pos( NULL, &cur_y ); _volume_make( &ypos_on_key, cur_y ); }

		float          freq_rate;

		if( ypos_on_key < 0                                         ) ypos_on_key = 0;
		if( ypos_on_key >= ORGANKEY_KEYHEIGHT * ptvORGANKEY_KEYNUM  ) ypos_on_key = ORGANKEY_KEYHEIGHT * ptvORGANKEY_KEYNUM - 1;

		_tone_last_key = 87 - ( ypos_on_key / ORGANKEY_KEYHEIGHT );

		key_clear();
		key_push ( _tone_last_key, true );
		freq_rate = g_freq->Get( ( _tone_last_key + 0x15 ) * 0x100 - EVENTDEFAULT_BASICKEY );

		_woice->strm_CHANGE( _tone_last_id, freq_rate );
	}

	if( p_cur->is_click_left() )
	{
		int32_t cur_y = 0;  p_cur->get_pos( NULL, &cur_y );
		p_cur->DragScroll( (int32_t)_rect.t, (int32_t)_rect.b, cur_y, _scrl_v );
	}
	else
	{
		_woice->strm_OFF( _tone_last_id, false );
		p_cur->set_action_free();
		key_clear();

	}
	return true;
}

bool ptuiOrganKey::_cursor_key_hold( ptuiCursor* p_cur )
{
	if( !KeyControl_IsKey( VK_SPACE ) )
	{
		_woice->strm_OFF( _tone_last_id, false );
		p_cur->set_action_free();
		key_clear();
	}
	return true;
}

bool ptuiOrganKey::_cursor_click_hold( ptuiCursor* p_cur )
{
	if( !p_cur->is_click_left() )
	{
		p_cur->set_action_free();
	}
	return true;
}
