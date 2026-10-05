
#include <pxMem.h>

#include "./ptui.h"

ptui::ptui()
{
	_b_base_init  = false;
	_b_show       = false;
	_id           =     0;

	_cur_last_x   =     0;
	_cur_last_y   =     0;
	_drag_start_x =     0;
	_drag_start_y =     0;

	_ani_nos      = NULL ;
	_ani_no_num   =     0;

	pxMem_zero( &_rect  , sizeof(_rect  ) );
	pxMem_zero( &_anchor, sizeof(_anchor) );
}

ptui::~ptui()
{
	_base_release();
}

void ptui::_base_release()
{
	_b_base_init = false;
	pxMem_free( (void**)&_ani_nos    );
}

bool ptui::_base_init( const ptuiANCHOR* p_anchor, int32_t ani_no_num )
{
	if( _b_base_init ) return false;

	_ani_no_num = ani_no_num;

	if( ani_no_num && !pxMem_zero_alloc( (void**)&_ani_nos, sizeof(int32_t) * _ani_no_num ) ) goto term;

	_anchor = *p_anchor;

	_b_base_init = true;
term:
	if( !_b_base_init ) _base_release();

	return _b_base_init;
}

void ptui::proc_begin( const fRECT *rc_view )
{
	_b_cur_on = false;
	for( int32_t i = 0; i < _ani_no_num; i++ ) _ani_nos[ i ] = 0;

	if     ( _anchor.flags & ptuiANCHOR_x1_ofs && _anchor.flags & ptuiANCHOR_x2_ofs ){ _rect.l = rc_view->l + _anchor.x1; _rect.r = rc_view->r - _anchor.x2; }
	else if( _anchor.flags & ptuiANCHOR_x1_ofs && _anchor.flags & ptuiANCHOR_x2_w   ){ _rect.l = rc_view->l + _anchor.x1; _rect.r = _rect.l    + _anchor.x2; }
	else if( _anchor.flags & ptuiANCHOR_x1_w   && _anchor.flags & ptuiANCHOR_x2_ofs ){ _rect.r = rc_view->r - _anchor.x2; _rect.l = _rect.r    - _anchor.x1; }

	if     ( _anchor.flags & ptuiANCHOR_y1_ofs && _anchor.flags & ptuiANCHOR_y2_ofs ){ _rect.t = rc_view->t + _anchor.y1; _rect.b = rc_view->b - _anchor.y2; }
	else if( _anchor.flags & ptuiANCHOR_y1_ofs && _anchor.flags & ptuiANCHOR_y2_h   ){ _rect.t = rc_view->t + _anchor.y1; _rect.b = _rect.t    + _anchor.y2; }
	else if( _anchor.flags & ptuiANCHOR_y1_h   && _anchor.flags & ptuiANCHOR_y2_ofs ){ _rect.b = rc_view->b - _anchor.y2; _rect.t = _rect.b    - _anchor.y1; }

	_proc_begin_sub();
}

bool ptui::check_on_map( const ptuiCursor* p_cur )
{
	if( _b_show )
	{
		_b_cur_on = p_cur->is_rect( _rect.l, _rect.t, _rect.r, _rect.b );
	}
	else
	{
		_b_cur_on = false;
	}

	p_cur->get_pos( &_cur_last_x, &_cur_last_y );

	return _b_cur_on;
}

bool ptui::_unit_anime_set( int32_t idx, int32_t no )
{
	if( idx < 0 || idx >= _ani_no_num ) return false;
	_ani_nos[ idx ] = no;
	return true;
}

bool ptui::_proc_begin_sub   ()
{
	return true;
}

bool ptui::_cursor_free      ( ptuiCursor* p_cur )
{
	return false;
}

bool ptui::_cursor_drag_unit ( ptuiCursor* p_cur )
{
	return false;
}

bool ptui::_cursor_click_hold ( ptuiCursor* p_cur )
{
	return false;
}

bool ptui::_cursor_click_hold2( ptuiCursor* p_cur )
{
	return false;
}

bool ptui::_cursor_key_hold( ptuiCursor* p_cur )
{
	return false;
}

bool ptui::_cursor_keyboard( ptuiCursor* p_cur )
{
	return false;
}

bool ptui::_cursor_scroll_h( ptuiCursor* p_cur )
{
	return false;
}

bool ptui::_cursor_scroll_v( ptuiCursor* p_cur )
{
	return false;
}

bool ptui::show_set( bool b )
{
	if( !_b_base_init ) return false;
	_b_show = b;
	return true;
}

bool ptui::cursor_action( ptuiCursor* p_cur )
{
	if( !_b_show ) return false;

	bool b_ret = false;

	switch( p_cur->get_action() )
	{
	case ptuiCURACT_free       : b_ret = _cursor_free       ( p_cur ); break;
	case ptuiCURACT_drag_unit  : b_ret = _cursor_drag_unit  ( p_cur ); break;
	case ptuiCURACT_click_hold : b_ret = _cursor_click_hold ( p_cur ); break;
	case ptuiCURACT_click_hold2: b_ret = _cursor_click_hold2( p_cur ); break;
	case ptuiCURACT_key_hold   : b_ret = _cursor_key_hold   ( p_cur ); break;
	case ptuiCURACT_keyboard   : b_ret = _cursor_keyboard   ( p_cur ); break;
	case ptuiCURACT_scroll_h   : b_ret = _cursor_scroll_h   ( p_cur ); break;
	case ptuiCURACT_scroll_v   : b_ret = _cursor_scroll_v   ( p_cur ); break;
	}

	return b_ret;
}

void ptui::put          ( const ptuiCursor* p_cur ) const
{
	if( !_b_show ) return;
	_put( p_cur );
}
