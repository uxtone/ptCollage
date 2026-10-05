// '17/02/13 if_EnvelopePoint.cpp/h -> ptv-EnveTable class.
// '17/02/15 -> ptuiEnvelope.
// '17/02/18 ptuiクラスを継承


#ifndef ptuiEnvelope_H
#define ptuiEnvelope_H

#include <pxStdDef.h>

#include "../../Generic/if_gen_Scroll.h"

#include "../ptVoice.h"
#include "../WoiceUnit.h"

#include "./ptuiMap.h"

class ptuiEnvelope: public ptuiMap
{
private:

bool           _b_init;
WoiceUnit*     _woice ;
if_gen_Scroll* _scrl_h;

void _pos_make        ( int32_t *px, int32_t *py, int32_t cur_x, int32_t cur_y ) const;

bool _point_search    ( const ptuiCursor* p_cur, int32_t* p_idx ) const;
bool _point_add       ( const ptuiCursor* p_cur, int32_t* p_idx );
bool _point_drag      ( const ptuiCursor* p_cur );

bool _point_get       ( int32_t idx, int32_t* px, int32_t* py ) const;
bool _point_set       ( int32_t idx, int32_t   x, int32_t   y );
bool _point_delete    ( int32_t idx );

bool _proc_begin_sub  () override;
bool _cursor_free     (       ptuiCursor* p_cur ) override;
bool _cursor_drag_unit(       ptuiCursor* p_cur ) override;
bool _cursor_scroll_h (       ptuiCursor* p_cur ) override;

bool _keyctrl_move    ( int32_t idx, int32_t mv_x, int32_t mv_y ) override;
bool _keyctrl_tab     ( bool b_back                             ) override;

bool _try_update      (                         )       override;
void _put             ( const ptuiCursor* p_cur ) const override;

void _release();


void _put_time_value() const;

public:

ptuiEnvelope();
~ptuiEnvelope();

bool init( WoiceUnit* woice );
};

#endif

