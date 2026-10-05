// '17/02/24 if_SineVolume.cpp/h -> ptuiSine Volume class.
// '17/02/26 -> "ptuiSines"

#ifndef ptuiSines_H
#define ptuiSines_H

#include "../WoiceUnit.h"

#include "./ptuiCursor.h"
#include "./ptui.h"

#include "../../Generic/if_gen_Scroll.h"

class ptuiSines: public ptui
{
private:

bool           _b_init      ;
WoiceUnit*     _woice       ;
int32_t        _start_volume;
if_gen_Scroll* _scrl_v      ;

void _pos_make         ( int32_t *px, int32_t* py, int32_t cur_x, int32_t cur_y ) const;

bool _proc_begin_sub   () override;
bool _cursor_free      (       ptuiCursor* p_cur ) override;
bool _cursor_drag_unit (       ptuiCursor* p_cur ) override;
bool _cursor_click_hold(       ptuiCursor* p_cur ) override;
bool _cursor_scroll_v  (       ptuiCursor* p_cur ) override;

void _put              ( const ptuiCursor *p_cur ) const override;


bool _search_button( const ptuiCursor* p_cur, int32_t* p_tgt_id ) const;

bool _get_volume_int    ( int32_t idx, int32_t* p_volume ) const;
bool _set_volume_int    ( int32_t idx, int32_t    volume );
bool _set_volume_default( int32_t idx );

void _release();

public:

ptuiSines();
~ptuiSines();

bool init( WoiceUnit* woice );

};

#endif
