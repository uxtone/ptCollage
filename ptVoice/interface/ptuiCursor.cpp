
#include <pxMem.h>

#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "./ptui.h"

#include "./ptuiCursor.h"

#define DRAGSCROLLWAIT 5

ptuiCursor::ptuiCursor()
{
	_b_init         = false;

	pxMem_zero( &_rc, sizeof(_rc) );
	_p_tgt          = 0;
	_action         = ptuiCURACT_free;
	_tgt_idx        = 0;

	_drag_ypos      = 0;
	_scroll_count   = 0;
	_active_tone_id = 0;

	_now_x          = 0;
	_now_y          = 0;

	_start_x        = 0;
	_start_y        = 0;

	_bits_now       = 0;
	_bits_trg       = 0;
}


bool ptuiCursor::init()
{
	if( _b_init ) return false;
	_b_init = true;
	return _b_init;
}

bool ptuiCursor::set_mouse_status( const pxMouse* mouse )
{
	if( !mouse ) return false;
	_now_x    = mouse->get_x  ();
	_now_y    = mouse->get_y  ();
	_bits_now = mouse->get_now();
	_bits_trg = mouse->get_trg();
	if( _bits_trg )
	{
		int a = 0;
		a++;
	}
	return true;
}


void ptuiCursor::DragScroll( int32_t frame1, int32_t frame2, int32_t cur, if_gen_Scroll* scrl )
{
	if( cur <  frame1 || cur >= frame2 )
	{
		if( !_scroll_count )
		{
			if( cur <  frame1 ) scrl->Scroll( frame2 - frame1, -1 );
			if( cur >= frame2 ) scrl->Scroll( frame2 - frame1,  1 );
			_scroll_count = DRAGSCROLLWAIT;
		}
		else
		{
			_scroll_count--;
		}
	}else{
		_scroll_count = 0;
	}
}

bool ptuiCursor::action_free_try( void* p_ptui )
{
	return static_cast<ptui*>(p_ptui)->cursor_action( this );
}

bool ptuiCursor::action_busy()
{
	if( _action == ptuiCURACT_keep_R ){ if( !is_click_right() ) _action = ptuiCURACT_free; }
	else                              { static_cast<ptui*>(_p_tgt)->cursor_action( this );          }
	return true;
}

bool ptuiCursor::set_action( void* p_tgt, ptuiCURACT action, int32_t tgt_idx )
{
	_p_tgt   = p_tgt  ;
	_action  = action ;
	_tgt_idx = tgt_idx;
	_start_x = _now_x ;
	_start_y = _now_y ;
	return true;
}

void ptuiCursor::set_action_free()
{
	_p_tgt   = NULL;
	_action  = ptuiCURACT_free;
	_tgt_idx = 0;
}

void ptuiCursor::set_action_keep_R()
{
	_p_tgt   = NULL;
	_action  = ptuiCURACT_keep_R;
	_tgt_idx = 0;
}

bool ptuiCursor::is_rect( float l, float t, float r, float b ) const
{
	if( _now_x >= l &&
		_now_y >= t &&
		_now_x <  r &&
		_now_y <  b ) return true;
	return false;
}

bool ptuiCursor::is_rect( const fRECT* p_rc                  ) const
{
	if( _now_x >= p_rc->l &&
		_now_y >= p_rc->t &&
		_now_x <  p_rc->r &&
		_now_y <  p_rc->b ) return true;

	return true;
}


bool ptuiCursor::get_pos( int32_t* px, int32_t* py ) const
{
	if( px ) *px = _now_x;
	if( py ) *py = _now_y;
	return true;
}

bool ptuiCursor::get_pos_start( int32_t* px, int32_t* py ) const
{
	if( px ) *px = _start_x;
	if( py ) *py = _start_y;
	return true;
}

ptuiCURACT ptuiCursor::get_action() const
{
	return _action;
}

bool ptuiCursor::get_target_index ( int32_t* p_idx ) const
{
	if( !_b_init ) return false;
	if( p_idx ) *p_idx = _tgt_idx;
	return true;
}

void* ptuiCursor::get_target() const
{
	return _p_tgt;
}

bool ptuiCursor::is_click_left   () const
{
	if( _bits_now & pxMOUSEBIT_L ) return true;
	return false;
}

bool ptuiCursor::is_click_right  () const
{
	if( _bits_now & pxMOUSEBIT_R ) return true;
	return false;
}

bool ptuiCursor::is_trigger_left () const
{
	if( _bits_trg & pxMOUSEBIT_L ) return true;
	return false;
}

bool ptuiCursor::is_trigger_right() const
{
	if( _bits_trg & pxMOUSEBIT_R ) return true;
	return false;
}

