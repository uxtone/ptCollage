
#include <pxMem.h>

#include <pxwDx09Draw.h>
extern pxwDx09Draw*   g_dxdraw;

#include "../CursorKeyCtrl.h"
extern CursorKeyCtrl* g_curkey;

#include <pxtnPulse_Oscillator.h>

#include "./Interface.h"
#include "./ptuiLayout.h"

#include "./ptuiWave.h"

#define _SCREEN_X (WAVEFRAME_X + 16)
#define _SCREEN_Y (WAVEFRAME_Y + 24)

#define _MAP_RATE_X 1.0f
#define _MAP_RATE_Y 1.0f

void ptuiWave::_release()
{
	_b_init = false;
	_map_release ();
	_base_release();
}


ptuiWave::ptuiWave()
{
	_b_init = false;
	_woice  = NULL ;
}

ptuiWave::~ptuiWave()
{
	_release ();
}

bool ptuiWave::init( WoiceUnit* woice )
{
	if( _b_init ) return false;

	uint32_t* p_posi = NULL;
	uint32_t* p_nega = NULL;

	uint32_t posi_top_base  = 0xff6ecc22;
	uint32_t posi_top_line1 = 0xffa1e153;
	uint32_t posi_top_line2 = 0xff87d63a;

	uint32_t posi_btm_base  = 0xff5b9600;
	uint32_t posi_btm_line1 = 0xff8bc922;
	uint32_t posi_btm_line2 = 0xff73af11;

	uint32_t nega_base      = 0xff436600;
	uint32_t nega_line1     = 0xff438000;
	uint32_t nega_line2     = 0xff437300;

	uint32_t gray_posi_top  = 0xff808080;
	uint32_t gray_nega_top  = 0xff404040;
	uint32_t gray_posi_btm  = 0xff606060;
	uint32_t gray_nega_btm  = 0xff303030;

	int32_t  half_y = SCREEN_WAVE_H/2;

	ptuiANCHOR anc = { 0 };
	anc.flags = ptuiANCHOR_x1_ofs|ptuiANCHOR_x2_w|ptuiANCHOR_y1_ofs|ptuiANCHOR_y2_h;
	anc.x1    = _SCREEN_X    ;
	anc.x2    = SCREEN_WAVE_W;
	anc.y1    = _SCREEN_Y    ;
	anc.y2    = SCREEN_WAVE_H;

	if( !_base_init( &anc, ptvFIXNUM_WAVE_POINT ) ) goto term;

	if( !_map_init ( SCREEN_WAVE_W, SCREEN_WAVE_H, SURF_WAVE_SCREEN ) ) goto term;

	p_posi = _p_map_posi;
	p_nega = _p_map_nega;

	for( int y = 0; y < SCREEN_WAVE_H; y++ )
	{
		for( int x = 0; x < SCREEN_WAVE_W; x++ )
		{
			if( x < SCREEN_WAVE_MAIN )
			{
				if( y < half_y )
				{
					if     ( (x % 128) == 127 || (y % 64) == 63 ) *p_posi = posi_top_line1;
					else if( (x %  32) ==  31 || (y % 16) == 15 ) *p_posi = posi_top_line2;
					else                                          *p_posi = posi_top_base ;

					if     ( (x % 128) == 127 || (y % 64) == 63 ) *p_nega = nega_line1    ;
					else if( (x %  32) ==  31 || (y % 16) == 15 ) *p_nega = nega_line2    ;
					else                                          *p_nega = nega_base     ;
				}
				else
				{
					if     ( (x % 128) == 127 || (y % 64) == 63 ) *p_posi = posi_btm_line1;
					else if( (x %  32) ==  31 || (y % 16) == 15 ) *p_posi = posi_btm_line2;
					else                                          *p_posi = posi_btm_base ;

					if     ( (x % 128) == 127 || (y % 64) == 63 ) *p_nega = nega_line1    ;
					else if( (x %  32) ==  31 || (y % 16) == 15 ) *p_nega = nega_line2    ;
					else                                          *p_nega = nega_base     ;
				}
			}
			else
			{
				if( y < half_y )
				{
					*p_posi = gray_posi_top;
					*p_nega = gray_nega_top;
				}
				else
				{
					*p_posi = gray_posi_btm;
					*p_nega = gray_nega_btm;
				}

			}
			p_posi++;
			p_nega++;
		}
	}

	_id           = 133;

	_woice        = woice;
	_b_map_redraw = true ;
	_b_init       = true ;
term:
	if( !_b_init ) _release();

	return _b_init;
}



void ptuiWave::_pos_make( int32_t *px, int32_t *py, int32_t cur_x, int32_t cur_y ) const
{
	*px = (int32_t)(                       (cur_x - _rect.l)   * _MAP_RATE_X );
	*py = (int32_t)( ( SCREEN_WAVE_H / 2 - (cur_y - _rect.t) ) * _MAP_RATE_Y );
}

static void _pos_fix( int32_t *px, int32_t *py )
{
	if( *px >= SCREEN_WAVE_MAIN ) *px = SCREEN_WAVE_MAIN-1;
	if( *px <                 0 ) *px =                  0;
	if( *py >               127 ) *py =                127;
	if( *py <              -127 ) *py =               -127;
}

bool ptuiWave::_point_set( int32_t idx, int32_t x, int32_t y )
{
	const pxtnVOICEUNIT* p_vc   = _woice->get_voice();
	const pxtnVOICEWAVE* p_wave = &p_vc->wave;

	if( idx <= 0 || idx >= p_wave->num ) return false;

	_pos_fix( &x, &y );

	if( idx > 0 &&
		x < p_wave->points[ idx - 1 ].x )
		x = p_wave->points[ idx - 1 ].x;

	if( idx < p_wave->num - 1 &&
		x > p_wave->points[ idx + 1 ].x )
		x = p_wave->points[ idx + 1 ].x;

	p_wave->points[ idx ].x = x;
	p_wave->points[ idx ].y = y;

	_woice->frame_alte_set( WOICEUNIT_ALTE_WAVE, -1 );

	return true;
}

bool ptuiWave::_point_get( int32_t idx, int32_t* px, int32_t* py ) const
{
	const pxtnVOICEUNIT* p_vc   = _woice->get_voice();
	const pxtnVOICEWAVE* p_wave = &p_vc->wave;
	if( idx <= 0 || idx >= p_wave->num ) return false;
	if( px ) *px = p_wave->points[ idx ].x;
	if( py ) *py = p_wave->points[ idx ].y;
	return true;
}

bool ptuiWave::_keyctrl_move( int32_t idx, int32_t mv_x, int32_t mv_y )
{
	const pxtnVOICEUNIT* p_vc   = _woice->get_voice();
	const pxtnVOICEWAVE* p_wave = &p_vc->wave;
	if( idx <= 0 || idx >= p_wave->num ) return false;

	return _point_set( idx, p_wave->points[ idx ].x + mv_x, p_wave->points[ idx ].y + mv_y );
}

bool ptuiWave::_keyctrl_tab( bool b_back )
{
	int32_t idx = 0;
	if( g_curkey->get( &idx ) != CursorKey_wave ) return false;

	pxtnVOICEUNIT* p_vc = _woice->get_voice();

	if( b_back )
	{
		if( idx <= 1 ) return false;
		idx--;
	}
	else
	{
		if( idx >= p_vc->wave.num - 1 ) return false;
		idx++;
	}
	g_curkey->set( CursorKey_wave, idx, this );
	return true;
}

bool ptuiWave::_point_delete( int32_t idx )
{
	pxtnVOICEUNIT* p_vc   = _woice->get_voice(); if( !p_vc ) return false;
	pxtnVOICEWAVE* p_wave = &p_vc->wave;

	if( idx )
	{
		for( int32_t i = idx; i < ptvFIXNUM_WAVE_POINT - 1; i++ ) p_wave->points[ i ] = p_wave->points[ i + 1 ];
		p_wave->num--;
	}

	_woice->frame_alte_set( WOICEUNIT_ALTE_WAVE, -1 );
	return true;
}

// add..
bool ptuiWave::_point_add( const ptuiCursor* p_cur, int32_t* p_idx )
{
	pxtnVOICEUNIT* p_vc   = _woice->get_voice(); if( !p_vc ) return false;
	pxtnVOICEWAVE* p_wave = &p_vc->wave;
	int32_t        x, y;
	int32_t        idx    = 0;

	if( p_wave->num >= ptvFIXNUM_WAVE_POINT ) return false;
	if( p_vc->type  != pxtnVOICE_Coodinate  ) return false;

	{ int32_t fx, fy; p_cur->get_pos( &fx, &fy ); _pos_make( &x, &y, fx, fy ); }

	_pos_fix ( &x, &y ); if( !x ) return false;

	_woice->frame_alte_set( WOICEUNIT_ALTE_WAVE, -1 );

	for( idx = 0; idx < p_wave->num; idx++ )
	{
		// same x.
		if( x == p_wave->points[ idx ].x )
		{
			p_wave->points[ idx ].y = y;
			g_curkey->set( CursorKey_wave, idx, this );
			if( p_idx ) *p_idx = idx;
			return true;
		}
		if( x  < p_wave->points[ idx ].x )
		{
			for( int j = p_wave->num; j > idx ; j-- ) p_wave->points[ j ] = p_wave->points[ j - 1 ];
			g_curkey->set( CursorKey_wave, idx, this );
			if( p_idx ) *p_idx = idx;
			p_wave->points[ idx ].x = x;
			p_wave->points[ idx ].y = y;
			p_wave->num++;
			return true;
		}
	}

	if( idx < ptvFIXNUM_WAVE_POINT )
	{
		if( p_idx ) *p_idx = idx;
		p_wave->points[ idx ].x = x;
		p_wave->points[ idx ].y = y;
		p_wave->num++;
		return true;
	}
	return false;
}


bool ptuiWave::_point_search( const ptuiCursor* p_cur, int32_t* p_idx ) const
{
	const pxtnVOICEUNIT* p_vc   = _woice->get_voice(); if( !p_vc ) return false;
	const pxtnVOICEWAVE* p_wave = &p_vc->wave;

	int32_t x = 0, y = 0;

	if( p_wave->num <= 1                   ) return false;
	if( p_vc->type != pxtnVOICE_Coodinate  ) return false;

	{ int32_t fx, fy; p_cur->get_pos( &fx, &fy ); _pos_make( &x, &y, fx, fy ); }

	_pos_fix ( &x, &y ); if( !x ) return false;

	for( int i = p_wave->num - 1; i > 0; i-- )
	{
		if( x >= p_wave->points[ i ].x - 4 && x < p_wave->points[ i ].x + 4 &&
			y >= p_wave->points[ i ].y - 8 && y < p_wave->points[ i ].y + 8 )
		{
			if( p_idx ) *p_idx = i;
			return true;
		}
	}
	return false;
}

// 移動
bool ptuiWave::_point_drag( const ptuiCursor* p_cur )
{
	int32_t x       = 0;
	int32_t y       = 0;
	int32_t start_x = 0;
	int32_t start_y = 0;
	int32_t now_x   = 0;
	int32_t now_y   = 0;

	{ int32_t fx, fy; p_cur->get_pos      ( &fx, &fy ); _pos_make(  &now_x  , &now_y  , fx, fy ); }
	{ int32_t fx, fy; p_cur->get_pos_start( &fx, &fy ); _pos_make(  &start_x, &start_y, fx, fy ); }

	x = _drag_start_x + (now_x - start_x);
	y = _drag_start_y + (now_y - start_y);

	_pos_fix ( &x, &y );

	if( !x ) return false;

	pxtnVOICEUNIT *p_vc   = _woice->get_voice();
	pxtnVOICEWAVE *p_wave = &p_vc->wave;
	int32_t idx = 0;

	if( !p_cur->get_target_index( &idx ) ) return false;

	if( idx <= 0 || p_wave->num <= 1 || p_vc->type != pxtnVOICE_Coodinate  ) return false;

	g_curkey->set( CursorKey_wave, idx, this );

	return _point_set( idx, x, y );
}

bool ptuiWave::_try_update()
{
	if( !_b_init ) return false;

	double  work_double =     0;
	int32_t work_long   =     0;

	pxtnPulse_Oscillator osci( NULL, NULL, NULL, NULL );

	pxtnVOICEUNIT* p_vc = _woice->get_voice();

	if( !_b_map_redraw ) return false;

	osci.ReadyGetSample( p_vc->wave.points, p_vc->wave.num, p_vc->volume, SCREEN_WAVE_MAIN, p_vc->wave.reso );

	switch( p_vc->type )
	{
	case pxtnVOICE_Overtone:

		for( int32_t x = 0; x < SCREEN_WAVE_W; x++ )
		{
			work_double = osci.GetOneSample_Overtone( x );
			work_long   = (int32_t)( work_double * SCREEN_WAVE_H / 2 );
			int32_t yy = (SCREEN_WAVE_H/2) - work_long;
			if( yy <                 0 ) yy =                 0;
			if( yy > (SCREEN_WAVE_H-1) ) yy = (SCREEN_WAVE_H-1);
			for( int y =  0; y < yy           ; y++ ) *( _p_map_view + x + y *SCREEN_WAVE_W ) = *( _p_map_nega + x + y *SCREEN_WAVE_W );
			for( int y = yy; y < SCREEN_WAVE_H; y++ ) *( _p_map_view + x + y *SCREEN_WAVE_W ) = *( _p_map_posi + x + y *SCREEN_WAVE_W );
		}
		break;

	case pxtnVOICE_Coodinate:

		for( int32_t x = 0; x < SCREEN_WAVE_W; x++ )
		{
			work_double = osci.GetOneSample_Coodinate( x%SCREEN_WAVE_MAIN );
			work_long   = (int32_t)( work_double * SCREEN_WAVE_H / 2 * 2 );

			int32_t yy = (SCREEN_WAVE_H/2) - work_long;
			if( yy <                 0 ) yy =                 0;
			if( yy > (SCREEN_WAVE_H-1) ) yy = (SCREEN_WAVE_H-1);
			for( int y =  0; y < yy           ; y++ ) *( _p_map_view + x + y *SCREEN_WAVE_W ) = *( _p_map_nega + x + y *SCREEN_WAVE_W );
			for( int y = yy; y < SCREEN_WAVE_H; y++ ) *( _p_map_view + x + y *SCREEN_WAVE_W ) = *( _p_map_posi + x + y *SCREEN_WAVE_W );
		}
		break;
	}
	g_dxdraw->tex_blt( _surf_screen, _p_map_view, SCREEN_WAVE_W, SCREEN_WAVE_H );

	_b_map_redraw = false;

	return true;
}


void ptuiWave::_put( const ptuiCursor* p_cur ) const
{
	// frame.
	{
		float x         = WAVEFRAME_X;
		float y         = WAVEFRAME_Y;
		fRECT rc_top_l  = {512,  0,792, 24};
		fRECT rc_top_r  = {512, 48,776, 72};
		fRECT rc_side_l = {512,104,528,232};
		fRECT rc_side_r = {528,104,544,232};
		fRECT rc_btm_l  = {512, 72,792, 88};
		fRECT rc_btm_r  = {512, 88,776,104};

		g_dxdraw->tex_Put_View( x                                              , y, &rc_top_l , SURF_PARTS );
		g_dxdraw->tex_Put_View( x + rc_top_l.w()                               , y, &rc_top_r , SURF_PARTS ); y += rc_top_l .h();
		g_dxdraw->tex_Put_View( x                                              , y, &rc_side_l, SURF_PARTS );
		g_dxdraw->tex_Put_View( x + rc_top_l.w() + rc_top_r.w() - rc_side_r.w(), y, &rc_side_r, SURF_PARTS ); y += rc_side_l.h();
		g_dxdraw->tex_Put_View( x                                              , y, &rc_side_l, SURF_PARTS );
		g_dxdraw->tex_Put_View( x + rc_top_l.w() + rc_top_r.w() - rc_side_r.w(), y, &rc_side_r, SURF_PARTS ); y += rc_side_l.h();
		g_dxdraw->tex_Put_View( x                                              , y, &rc_btm_l , SURF_PARTS );
		g_dxdraw->tex_Put_View( x + rc_btm_l.w()                               , y, &rc_btm_r , SURF_PARTS );
	}


	// map.
	{ fRECT rc = { 0, 0, SCREEN_WAVE_W, SCREEN_WAVE_H}; g_dxdraw->tex_Put_View( _SCREEN_X, _SCREEN_Y, &rc, _surf_screen ); }

	pxtnVOICEUNIT* p_vc = _woice->get_voice();
	ptv_put_value( WAVEFRAME_X + 468, WAVEFRAME_Y + 8, p_vc->wave.num );

	if( p_vc->type == pxtnVOICE_Coodinate && ( _b_cur_on || p_cur->get_target() == this ) || g_curkey->get_tgt() == this )
	{
		fRECT rcPoint[] =
		{
			{  0,488,  8,496},
			{  8,488, 16,496},
			{ 16,488, 24,496},
		};

		float   x   = 0;
		float   y   = 0;
		int32_t idx = 0;

		for( int32_t i = 0; i < p_vc->wave.num; i++ )
		{
			const pxtnPOINT* pp = &p_vc->wave.points[ i ];
			x = _rect.l                     + pp->x / _MAP_RATE_X;
			y = _rect.t + SCREEN_WAVE_H / 2 - pp->y / _MAP_RATE_Y;
			g_dxdraw->tex_Put_Clip( x - 3, y - 3, &rcPoint[ _ani_nos[ i ] ], SURF_PARTS, &_rect );

			if( g_curkey->get( &idx ) == CursorKey_wave && idx == i )
			{
				fRECT rcKeyCtrl = {112,480,128,496};
				g_dxdraw->tex_Put_Clip( x - 7, y - 7, &rcKeyCtrl, SURF_PARTS, &_rect );
			}
		}

		for( int32_t i = 0; i < p_vc->wave.num; i++ )
		{
			const pxtnPOINT* pp = &p_vc->wave.points[ i ];
			x = _rect.l                     + pp->x / _MAP_RATE_X;
			y = _rect.t + SCREEN_WAVE_H / 2 - pp->y / _MAP_RATE_Y;

			if( _ani_nos[ i ] || (g_curkey->get( &idx ) == CursorKey_wave && idx == i ) )
			{
				ptv_put_value( x + 8, y - 16, pp->x );
				ptv_put_value( x + 8, y -  8, pp->y );
			}
		}

		// cursor pos.
		{
			int32_t cx, cy;
			_pos_make(  &cx, &cy, _cur_last_x, _cur_last_y );
			ptv_put_value( _rect.r - 4 * 4, _rect.t + 4    , cx );
			ptv_put_value( _rect.r - 4 * 4, _rect.t + 4 + 8, cy );
		}
	}
}

// cursor operation ------------------

bool ptuiWave::_cursor_free( ptuiCursor* p_cur )
{
	if( !_b_cur_on ) return false;

	int32_t idx = 0;

	// L click
	if( p_cur->is_trigger_left()  )
	{
		if( !_point_search( p_cur, &idx ) && !_point_add( p_cur, &idx ) ) return false;
		if( !_point_get( idx, &_drag_start_x, &_drag_start_y )          ) return false;

		p_cur->set_action( this, ptuiCURACT_drag_unit, idx );

		_unit_anime_set( idx, 2 );

		g_curkey->set( CursorKey_wave, idx, this );
	}
	// R click
	else if( p_cur->is_trigger_right() )
	{
		if( !_point_search( p_cur, &idx ) ) return false;
		if( !_point_delete( idx )         ) return false;

		p_cur->set_action_keep_R();
	}
	else
	{
		if( !_point_search( p_cur, &idx ) ) return false;
		_unit_anime_set( idx, 1 );
	}
	return true;
}

bool ptuiWave::_cursor_drag_unit( ptuiCursor* p_cur )
{
	int32_t idx = 0;

	if( !p_cur->get_target_index( &idx ) ) return false;

	_point_drag( p_cur );

	if( !p_cur->is_click_left() ){ _unit_anime_set( idx, 0 ); p_cur->set_action_free(); }
	else                         { _unit_anime_set( idx, 2 );                    }
	return true;
}
