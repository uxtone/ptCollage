// '17/02/17 if-Cursor -> ptuiCursor.

#ifndef ptuiCursor_H
#define ptuiCursor_H

#include <pxMouse.h>

#include "../../Generic/if_gen_Scroll.h"

enum ptuiCURACT
{
	ptuiCURACT_free   = 0 ,
	ptuiCURACT_drag_unit  ,
	ptuiCURACT_click_hold ,
	ptuiCURACT_click_hold2,
	ptuiCURACT_key_hold   ,
	ptuiCURACT_keyboard   ,
	ptuiCURACT_scroll_v   ,
	ptuiCURACT_scroll_h   ,

	ptuiCURACT_keep_R     ,
};

class ptuiCursor
{
private:

bool         _b_init  ;

fRECT        _rc      ;

void*        _p_tgt   ;
ptuiCURACT   _action  ;
int32_t      _tgt_idx ;

float        _drag_ypos;
int32_t      _scroll_count;
int32_t      _active_tone_id;

int32_t      _bits_now;
int32_t      _bits_trg;

int32_t      _now_x   ;
int32_t      _now_y   ;

int32_t      _start_x ;
int32_t      _start_y ;

public:
ptuiCursor();

bool init      ();
void DragScroll( int32_t frame1, int32_t frame2, int32_t cur, if_gen_Scroll* scrl );

bool set_mouse_status( const pxMouse* mosue );

bool action_free_try ( void* ptui );
bool action_busy     ();

void put();

bool set_action( void* p_tgt, ptuiCURACT action, int32_t tgt_idx );
void set_action_free  ();
void set_action_keep_R();

ptuiCURACT get_action() const;
void*      get_target() const;

bool is_rect( float l, float t, float r, float b ) const;
bool is_rect( const fRECT* p_rc                  ) const;
bool get_pos      ( int32_t* px, int32_t* py ) const;
bool get_pos_start( int32_t* px, int32_t* py ) const;
bool get_target_index( int32_t* p_idx ) const;


bool is_click_left   () const;
bool is_click_right  () const;
bool is_trigger_left () const;
bool is_trigger_right() const;

};

#endif
