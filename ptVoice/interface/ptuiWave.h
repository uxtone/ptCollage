// '17/02/12 if_WaveTable.cpp/h -> ptv-WaveTable class.
// '17/02/15 -> ptuiWave.
// '17/02/18 ptuiクラスを継承

#ifndef ptuiWave_H
#define ptuiWave_H

#include <pxStdDef.h>

#include <pxtnWoice.h>

#include "../ptVoice.h"
#include "../WoiceUnit.h"

#include "./ptuiCursor.h"

#include "./ptuiMap.h"

class ptuiWave: public ptuiMap
{
private:

bool       _b_init;
WoiceUnit* _woice ;

void _pos_make        ( int32_t *px, int32_t *py, int32_t cur_x, int32_t cur_y ) const;

bool _point_search    ( const ptuiCursor* p_cur, int32_t* p_idx ) const;
bool _point_add       ( const ptuiCursor* p_cur, int32_t* p_idx );
bool _point_drag      ( const ptuiCursor* p_cur );
bool _point_set       ( int32_t idx, int32_t x, int32_t y );
bool _point_get       ( int32_t idx, int32_t* px, int32_t* py ) const;
bool _point_delete    ( int32_t idx );

    // override..
bool _cursor_free     ( ptuiCursor* p_cur ) override;
bool _cursor_drag_unit( ptuiCursor* p_cur ) override;

bool _keyctrl_move    ( int32_t idx, int32_t mv_x, int32_t mv_y ) override;
bool _keyctrl_tab     ( bool b_back ) override;

bool _try_update      (                         )       override;
void _put             ( const ptuiCursor* p_cur ) const override;

void _release();

public:

ptuiWave();
~ptuiWave();

bool init         ( WoiceUnit* woice );
};

#endif
