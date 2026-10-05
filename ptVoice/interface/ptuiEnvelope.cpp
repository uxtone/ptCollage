
#include <pxMem.h>

#include <pxwDx09Draw.h>
extern pxwDx09Draw*   g_dxdraw;

#include "../CursorKeyCtrl.h"
extern CursorKeyCtrl* g_curkey;

#include "./Interface.h"
#include "./ptuiLayout.h"

#include "./ptuiEnvelope.h"

#define _SCREEN_X (ENVEFRAME_X + 16)
#define _SCREEN_Y (ENVEFRAME_Y + 24)

#define _MAP_RATE_Y 0.5f

#define _SAMPLING_MAP_X_LOOP 100


void ptuiEnvelope::_release()
{
	_b_init = false;
	SAFE_DELETE( _scrl_h );
	_map_release ();
	_base_release();
}

ptuiEnvelope:: ptuiEnvelope()
{
	_b_init = false;
	_woice  = NULL ;
	_scrl_h = NULL ;
}

ptuiEnvelope::~ptuiEnvelope()
{
	_release();
}

bool ptuiEnvelope::init( WoiceUnit* woice )
{
	if( _b_init ) return false;

	uint32_t* p_posi = NULL;
	uint32_t* p_nega = NULL;

	uint32_t posi_top_base  = 0xff226ecc;
	uint32_t posi_top_line1 = 0xff53a1e1;
	uint32_t posi_top_line2 = 0xff3a87d6;

	uint32_t nega_base      = 0xff004366;
	uint32_t nega_line1     = 0xff004380;
	uint32_t nega_line2     = 0xff004373;

	int32_t w = SCREEN_ENVE_W;
	int32_t h = SCREEN_ENVE_H;

	ptuiANCHOR anc = { 0 };
	anc.flags = ptuiANCHOR_x1_ofs|ptuiANCHOR_x2_w|ptuiANCHOR_y1_ofs|ptuiANCHOR_y2_h;
	anc.x1    = _SCREEN_X    ;
	anc.x2    = w;
	anc.y1    = _SCREEN_Y    ;
	anc.y2    = h;

	if( !_base_init( &anc, ptvMAX_ENVELOPEPOINT ) ) goto term;

	if( !_map_init ( w, h, SURF_ENVE_SCREEN ) ) goto term;

	p_posi = _p_map_posi;
	p_nega = _p_map_nega;

	for( int y = 0; y < h; y++ )
	{
		for( int x = 0; x < w; x++ )
		{
			if     ( (x % 100) ==  0 || (y % 64) == 63 ) *p_posi = posi_top_line1;
			else if( (x %  25) ==  0 || (y % 16) == 15 ) *p_posi = posi_top_line2;
			else                                         *p_posi = posi_top_base ;

			if     ( (x % 100) ==  0 || (y % 64) == 63 ) *p_nega = nega_line1    ;
			else if( (x %  25) ==  0 || (y % 16) == 15 ) *p_nega = nega_line2    ;
			else                                         *p_nega = nega_base     ;

			p_posi++;
			p_nega++;
		}
	}

	_scrl_h = new if_gen_Scroll();
	_scrl_h->Initialize( g_dxdraw, SURF_PARTS, false );
	_scrl_h->SetOffset( 0 );


	_woice        = woice;
	_b_map_redraw = true ;
	_b_init       = true ;
term:
	if( !_b_init ) _release();

	return _b_init;
}

void ptuiEnvelope::_pos_make( int32_t *px, int32_t *py, int32_t cur_x, int32_t cur_y ) const
{
	float map_rate_x = 1 / _woice->zoom_get();

	*px = (int32_t)( ( cur_x - _rect.l + _scrl_h->GetOffset() ) * map_rate_x );
	*py = (int32_t)( (         _rect.b - cur_y                ) * _MAP_RATE_Y );
}

static void _pos_fix( int32_t *px, int32_t *py )
{
	if( *px <   0 ) *px =   0;
	if( *py <   0 ) *py =   0;
	if( *py > 128 ) *py = 128;
}

bool ptuiEnvelope::_keyctrl_move( int32_t idx, int32_t mv_x, int32_t mv_y )
{
	const pxtnVOICEUNIT* p_vc   = _woice->get_voice();
	const pxtnPOINT*     points = p_vc->envelope.points;
	if( idx < 0 || idx >= ptvMAX_ENVELOPEPOINT ) return false;

	return _point_set( idx, points[ idx ].x + mv_x, points[ idx ].y + mv_y );
}

bool ptuiEnvelope::_keyctrl_tab( bool b_back )
{
	int32_t idx = 0;
	if( g_curkey->get( &idx ) != CursorKey_envelope ) return false;

	pxtnVOICEUNIT* p_vc = _woice->get_voice();

	if( b_back )
	{
		if( idx <= 0 ) return false;
		idx--;
	}
	else
	{
		if( idx >= ptvMAX_ENVELOPEPOINT - 1     ) return false;
		if( !p_vc->envelope.points[ idx + 1 ].x ) return false;
		idx++;
	}
	g_curkey->set( CursorKey_envelope, idx, this );
	return true;
}

bool ptuiEnvelope::_point_search( const ptuiCursor* p_cur, int32_t* p_idx ) const
{
	int32_t              cur_x, cur_y;
	int32_t              point_x = 0;
	int32_t              point_y = 0;
	const pxtnVOICEUNIT* p_vc    = _woice->get_voice();
	const pxtnPOINT*     points  = p_vc->envelope.points;

	int32_t rang_x = (int32_t)( 3 / _woice->zoom_get() );
	int32_t rang_y = (int32_t)( 3 /                  1 );

	{ int32_t fx, fy; p_cur->get_pos( &fx, &fy ); _pos_make( &cur_x, &cur_y, fx, fy ); }

	for( int i = 0; i < p_vc->envelope.head_num; i++ )
	{
		point_x += points[ i ].x;
		point_y  = points[ i ].y;

		if( cur_x >= point_x - rang_x &&
			cur_x <  point_x + rang_x &&
			cur_y >= point_y - rang_y &&
			cur_y <  point_y + rang_y )
		{
			if( p_idx ) *p_idx = i;
			return true;
		}
	}
	return false;
}

bool ptuiEnvelope::_point_add( const ptuiCursor* p_cur, int32_t* p_idx )
{
	bool           b_ret = false;

	int32_t        cur_x, cur_y;
	pxtnVOICEUNIT* p_vc    = _woice->get_voice();
	pxtnPOINT*     points  = p_vc->envelope.points;
	int32_t        idx     = 0;

	{ int32_t fx, fy; p_cur->get_pos( &fx, &fy ); fx; _pos_make( &cur_x, &cur_y, fx, fy ); }

	_pos_fix ( &cur_x, &cur_y );

	for( idx = 1; idx < ptvMAX_ENVELOPEPOINT; idx++ ){ if( !points[ idx ].x ) break; }
	if ( idx == ptvMAX_ENVELOPEPOINT ) goto term;

	for( idx = 0; idx < ptvMAX_ENVELOPEPOINT - 1; idx++ )
	{
		// add last.
		if( idx && !points[ idx ].x && !points[ idx ].y )
		{
			points[ idx     ].x  = cur_x;
			points[ idx     ].y  = cur_y;
			if( p_idx ) *p_idx = idx;
			b_ret = true;
			goto term;
		}
		// insert.
		else if( points[ idx ].x >  cur_x )
		{
			for( int i = ptvMAX_ENVELOPEPOINT - 1; i > idx; i-- ) points[ i ] = points[ i - 1 ];
			points[ idx + 1 ].x -= cur_x;
			points[ idx     ].x  = cur_x;
			points[ idx     ].y  = cur_y;
			if( p_idx ) *p_idx = idx;
			b_ret = true;
			goto term;
		}
		// update.
		else if( points[ idx ].x == cur_x )
		{
			points[ idx     ].y = cur_y;
			if( p_idx ) *p_idx = idx;
			b_ret = true;
			goto term;
		}
		cur_x -= points[ idx ].x;
	}
	if( cur_x <= 0 ) goto term;

	b_ret = true;
term:
	if( b_ret )
	{
		_woice->frame_alte_set( WOICEUNIT_ALTE_ENVE, -1 );
		_b_map_redraw = true;
	}
	return b_ret;
}

bool ptuiEnvelope::_point_delete( int32_t idx )
{
	if( idx < 0 || idx >= ptvMAX_ENVELOPEPOINT ) return false;

	pxtnVOICEUNIT* p_vc   = _woice->get_voice();
	pxtnPOINT*     points = p_vc->envelope.points;

	if( !points[ 1 ].x ) return false;

	g_curkey->clear();

	if( idx < ptvMAX_ENVELOPEPOINT - 1 &&
		( points[ idx + 1 ].x || points[ idx + 1 ].y ) ) points[ idx + 1 ].x += points[ idx ].x;

	// shift..
	for( int i = idx; i < ptvMAX_ENVELOPEPOINT - 1; i++ )
	{
		points[ i ].x = points[ i + 1 ].x;
		points[ i ].y = points[ i + 1 ].y;
	}
	points[ ptvMAX_ENVELOPEPOINT - 1 ].x = 0;
	points[ ptvMAX_ENVELOPEPOINT - 1 ].y = 0;

	_woice->frame_alte_set( WOICEUNIT_ALTE_ENVE, -1 );
	_b_map_redraw = true;
	return true;
}


bool ptuiEnvelope::_point_get( int32_t idx, int32_t* px, int32_t* py ) const
{
	if( idx < 0 || idx >= ptvMAX_ENVELOPEPOINT ) return false;
	const pxtnVOICEUNIT* p_vc   = _woice->get_voice();
	const pxtnPOINT*     points = p_vc->envelope.points;
	if( px ) *px = points[ idx ].x;
	if( py ) *py = points[ idx ].y;
	return true;
}

bool ptuiEnvelope::_point_set      ( int32_t idx, int32_t x, int32_t y )
{
	if( idx < 0 || idx >= ptvMAX_ENVELOPEPOINT ) return false;
	pxtnVOICEUNIT* p_vc   = _woice->get_voice();
	pxtnPOINT*     points = p_vc->envelope.points;

	_pos_fix( &x, &y );

	if( idx && x < 0 ) x = 0;

	points[ idx ].x = x;
	points[ idx ].y = y;

	_woice->frame_alte_set( WOICEUNIT_ALTE_ENVE, -1 );
	_b_map_redraw = true;
	return true;
}

bool ptuiEnvelope::_point_drag      ( const ptuiCursor* p_cur )
{
	int32_t idx = 0;

	if( !p_cur->get_target_index( &idx )       ) return false;
	if( idx < 0 || idx >= ptvMAX_ENVELOPEPOINT ) return false;

	pxtnVOICEUNIT* p_vc   = _woice->get_voice();
	pxtnPOINT*     points = p_vc->envelope.points;

	int32_t cur_start_x;
	int32_t cur_start_y;
	int32_t cur_now_x  ;
	int32_t cur_now_y  ;

	{ int32_t fx, fy; p_cur->get_pos      ( &fx, &fy ); _pos_make( &cur_now_x  , &cur_now_y  , fx, fy ); }
	{ int32_t fx, fy; p_cur->get_pos_start( &fx, &fy ); _pos_make( &cur_start_x, &cur_start_y, fx, fy ); }

	int32_t ofs_x = cur_now_x - cur_start_x;
	int32_t ofs_y = cur_now_y - cur_start_y;

	return _point_set( idx, _drag_start_x + ofs_x, _drag_start_y + ofs_y );
}


bool ptuiEnvelope::_try_update()
{
	int32_t e, e_num;
	int32_t y_now;
	int32_t x_now;
	int32_t zoom_stride;

	int32_t w = SCREEN_ENVE_W;
	int32_t h = SCREEN_ENVE_H;

	const pxtnVOICEUNIT* p_vc   = _woice->get_voice();
	const pxtnPOINT*     points = p_vc->envelope.points;

	if     ( _woice->zoom_get() == 0.1f ) zoom_stride = 10;
	else if( _woice->zoom_get() == 0.5f ) zoom_stride =  2;
	else if( _woice->zoom_get() == 1.0f ) zoom_stride =  1;
	else return false;

	for( e_num = 1; e_num < ptvMAX_ENVELOPEPOINT; e_num++ ){ if( !points[ e_num ].x && !points[ e_num ].y ) break; }

	// redraw..
	if( !_b_map_redraw ) return true;

	_scrl_h->SetSize( p_vc->envelope.fps / zoom_stride *        10 ); // 10 sec.
	_scrl_h->SetMove( p_vc->envelope.fps / 2, p_vc->envelope.fps/2 );

	uint32_t *p_src = _p_map_view;
	int32_t  offset = _scrl_h->GetOffset();
	int32_t  src_x;

	e     = 0;
	x_now = offset * zoom_stride;
	y_now = 0;

	for( int x = 0; x < w; x++ )
	{
		src_x = ( offset + x ) % _SAMPLING_MAP_X_LOOP;

		while( e < e_num && points[ e ].x - x_now <= 0 ){ x_now -= points[ e ].x; y_now = points[ e ].y; e++; }

		if(     !e         ) y_now =                       points[ e ].y                       * x_now / points[ e ].x;
		else if( e < e_num ) y_now = points[ e - 1 ].y + ( points[ e ].y - points[ e - 1 ].y ) * x_now / points[ e ].x;

		int32_t t = h - (int32_t)( (float)y_now / _MAP_RATE_Y );
		int32_t b = h;
		if( t < 0 ) t = 0;
		uint32_t *p = _p_map_view;
		for( int32_t y = 0; y < t; y++ ) *(_p_map_view + x + y * w ) =  *(_p_map_nega + src_x + y * w );
		for( int32_t y = t; y < b; y++ ) *(_p_map_view + x + y * w ) =  *(_p_map_posi + src_x + y * w );
		x_now += zoom_stride;
	}

	g_dxdraw->tex_blt( _surf_screen, _p_map_view, w, h );

	_b_map_redraw = false;

	return true;
}

void ptuiEnvelope::_put_time_value() const
{
	int32_t value_stride = 100;
	int32_t pos_stride   = 100;
	int32_t offset       = _scrl_h->GetOffset();

	float   pos_x        =   0;
	float   value_x      =   0;

	float   z = _woice->zoom_get();

	if     ( z == 0.5f )
	{
		pos_stride   = (int32_t)( ((float)value_stride*z) ) *  5;
		value_stride = value_stride                         *  5;
	}
	else if( z == 0.1f )
	{
		pos_stride   = (int32_t)( ((float)value_stride*z) ) * 10;
		value_stride = value_stride                         * 10;
	}

	while( pos_x - offset < -16 )
	{
		pos_x   += pos_stride  ;
		value_x += value_stride;
	}

	for( int i = 0; i < 6; i++ )
	{
		ptv_put_envesec( pos_x - offset + _SCREEN_X, _SCREEN_Y, value_x/100, &_rect );
		pos_x   += pos_stride  ;
		value_x += value_stride;
	}
}

void ptuiEnvelope::_put( const ptuiCursor* p_cur ) const
{
	int32_t w = SCREEN_ENVE_W;
	int32_t h = SCREEN_ENVE_H;

	{
		float x         = ENVEFRAME_X;
		float y         = ENVEFRAME_Y;

		fRECT rc_top_l  = {512, 24,792, 48};
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

	_scrl_h->Put();

	{ fRECT rc = { 0, 0, w, h}; g_dxdraw->tex_Put_Clip( _SCREEN_X, _SCREEN_Y, &rc, _surf_screen, &_rect ); }

	_put_time_value();

	const pxtnVOICEUNIT* p_vc   = _woice->get_voice();
	const pxtnPOINT*     points = p_vc->envelope.points;

	float zoom = _woice->zoom_get();

	int  e_num = 0;
	for( e_num = 1; e_num < ptvMAX_ENVELOPEPOINT; e_num++ ){ if( !points[ e_num ].x ) break; }

	ptv_put_value( ENVEFRAME_X + 468, ENVEFRAME_Y + 8, e_num );

	if( _b_cur_on || p_cur->get_target() == this || g_curkey->get_tgt() == this )
	{
		fRECT rcPoint[] =
		{
			{  0,488,  8,496},
			{  8,488, 16,496},
			{ 16,488, 24,496},
		};

		float   map_rate_x = 1.0f / _woice->zoom_get();
		int32_t idx = 0;

		float   fx = _SCREEN_X - _scrl_h->GetOffset();
		int32_t x  = 0;
		int32_t y  = 0;

		// points(mark) ---------------
		for( int e = 0; e < ptvMAX_ENVELOPEPOINT; e++ )
		{
			if( !e || points[ e ].x || points[ e ].y )
			{
				fx +=                    (float)points[ e ].x /  map_rate_x;
				x  = (int32_t)fx;
				y  = (int32_t)( _SCREEN_Y + h - points[ e ].y / _MAP_RATE_Y);
				g_dxdraw->tex_Put_Clip( (float)x - 3, (float)y - 3, &rcPoint[ _ani_nos[ e ] ], SURF_PARTS, &_rect );

				if( g_curkey->get( &idx ) == CursorKey_envelope && idx == e )
				{
					fRECT rcKeyCtrl = {112,480,128,496};
					g_dxdraw->tex_Put_Clip( (float)x - 7, (float)y - 7, &rcKeyCtrl, SURF_PARTS, &_rect );
				}
			}
			else break;
		}

		// points(value) ---------------
		fx = _SCREEN_X - _scrl_h->GetOffset();
		for( int e = 0; e < ptvMAX_ENVELOPEPOINT; e++ )
		{
			if( !e || points[ e ].x || points[ e ].y )
			{
				fx +=                    (float)points[ e ].x /  map_rate_x;
				x  = (int32_t)fx;
				y  = (int32_t)( _SCREEN_Y + h - points[ e ].y / _MAP_RATE_Y);

				if( _ani_nos[ e ] || (g_curkey->get( &idx ) == CursorKey_envelope && idx == e ) )
				{
					ptv_put_value( (float)x + 8, (float)y - 16, points[ e ].x, &_rect );
					ptv_put_value( (float)x + 8, (float)y -  8, points[ e ].y, &_rect );
				}
			}
			else break;
		}

		// cursor pos.
		{
			int32_t cx, cy;
			_pos_make(  &cx, &cy, _cur_last_x, _cur_last_y );
//			ptv_put_value( _rect.r - 4 * 4, _rect.t + 4    , cx );
//			ptv_put_value( _rect.r - 4 * 4, _rect.t + 4 + 8, cy );
		}
	}
}

bool ptuiEnvelope::_proc_begin_sub()
{
	_scrl_h->SetRect( &_rect );
	return true;
}

// cursor action ------------------------

bool ptuiEnvelope::_cursor_free( ptuiCursor* p_cur )
{
	{
		int32_t fx, fy; p_cur->get_pos( &fx, &fy );
		int32_t old_ofs = _scrl_h->GetOffset();
		if( _scrl_h->Action( fx, fy, p_cur->is_click_left(), p_cur->is_trigger_left() ) )
		{
			if( old_ofs != _scrl_h->GetOffset() ) _woice->frame_alte_set( WOICEUNIT_ALTE_enve_screen, -1 );
			p_cur->set_action( this, ptuiCURACT_scroll_h, 0 );
			return true;
		}
	}

	if( !_b_cur_on ) return false;

	int32_t idx = 0;

	// click L
	if( p_cur->is_trigger_left() )
	{
		if( !_point_search( p_cur, &idx ) && !_point_add( p_cur, &idx ) ) return false;
		if( !_point_get( idx, &_drag_start_x, &_drag_start_y ) ) return false;

		p_cur->set_action( this, ptuiCURACT_drag_unit, idx );

		_unit_anime_set( idx, 2 );

		g_curkey->set( CursorKey_envelope, idx, this );
	}
	// click R
	else if( p_cur->is_trigger_right() )
	{
		if( !_point_search( p_cur, &idx ) ) return false;
		if( !_point_delete(         idx ) ) return false;

		p_cur->set_action_keep_R();
	}
	else
	{
		if( !_point_search( p_cur, &idx ) ) return false;
		_unit_anime_set( idx, 1 );
		return false;
	}

	return true;
}

bool ptuiEnvelope::_cursor_drag_unit( ptuiCursor* p_cur )
{
	int32_t idx = 0;

	if( !p_cur->get_target_index( &idx ) ) return false;

	_point_drag( p_cur );

	if( !p_cur->is_click_left() ){ _unit_anime_set( idx, 0 ); p_cur->set_action_free(); }
	else                         { _unit_anime_set( idx, 2 );                    }

	return true;
}

bool   ptuiEnvelope::_cursor_scroll_h  ( ptuiCursor* p_cur )
{
	int32_t fx, fy; p_cur->get_pos( &fx, &fy );

	if( !_scrl_h->Action( fx, fy, p_cur->is_click_left(), p_cur->is_trigger_left() ) )
	{
		p_cur->set_action_free();
	}

	if( _scrl_h->is_frame_update() )
		_woice->frame_alte_set( WOICEUNIT_ALTE_enve_screen, -1 );
	return true;
}
