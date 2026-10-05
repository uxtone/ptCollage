// '17/02/21 ptuiWave_mini from ptuiWave.

#ifndef ptuiWave_mini_H
#define ptuiWave_mini_H

#include <pxStdDef.h>

#include <pxtnWoice.h>

#include "../ptVoice.h"
#include "../WoiceUnit.h"

#include "./ptuiCursor.h"

#include "./ptuiMap.h"

class ptuiWave_mini: public ptuiMap
{
private:

bool       _b_init  ;
WoiceUnit* _woice   ;
int32_t    _unit_idx;

bool _try_update(                         )       override;
void _put       ( const ptuiCursor* p_cur ) const override;

void _release   ();

public:

ptuiWave_mini();
~ptuiWave_mini();

bool init( WoiceUnit* woice, int32_t unit_idx );
};

#endif
