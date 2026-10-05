// '17/02/24 if_ReleaseTime.cpp/h -> ptuiReleaseTime class.

#ifndef ptuiReleaseTime_H
#define ptuiReleaseTime_H

#include "../WoiceUnit.h"

#include "./ptuiCursor.h"
#include "./ptui.h"
#include "./ptuiKnob.h"

class ptuiReleaseTime: public ptui
{
private:

bool       _b_init      ;
int32_t    _start_volume;
WoiceUnit* _woice       ;
ptuiKnob*  _knob        ;

bool _cursor_free      ( ptuiCursor* p_cur ) override;
bool _cursor_drag_unit ( ptuiCursor* p_cur ) override;
bool _cursor_click_hold( ptuiCursor* p_cur ) override;

void _put              ( const ptuiCursor *p_cur ) const override;

bool _search_button( const ptuiCursor* p_cur, int32_t* p_tgt_id ) const;

bool _get_volume_int( int32_t* p_volume ) const;
bool _set_volume_int( int32_t    volume );

bool _set_volume_default();

void _release();

public:

ptuiReleaseTime();
~ptuiReleaseTime();

bool init( WoiceUnit* woice );
};


#endif
