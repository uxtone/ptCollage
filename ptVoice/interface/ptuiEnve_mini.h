// '17/03/21 ptuiEnve_mini from ptuiEnvelope.

#ifndef ptuiEnve_mini_H
#define ptuiEnve_mini_H

#include <pxStdDef.h>

#include "../../Generic/if_gen_Scroll.h"

#include "../ptVoice.h"
#include "../WoiceUnit.h"

#include "./ptuiMap.h"

class ptuiEnve_mini: public ptuiMap
{
private:

bool           _b_init  ;
WoiceUnit*     _woice   ;
int32_t        _unit_idx;

bool _try_update      (                         )       override;
void _put             ( const ptuiCursor* p_cur ) const override;

void _release();

public:

ptuiEnve_mini();
~ptuiEnve_mini();

bool init( WoiceUnit* woice, int32_t unit_idx );
};

#endif

