// '17/03/20 ptuiRackHalf copy from ptuiRack.

#ifndef ptuiRackHalf_H
#define ptuiRackHalf_H

#include "../WoiceUnit.h"

#include "./ptuiCursor.h"
#include "./ptui.h"
#include "./ptuiKnob.h"

class ptuiRackHalf: public ptui
{
private:

enum _KNOB_ID
{
	_KNOB_volume = 0,
	_KNOB_num,
};

bool       _b_init;
WoiceUnit* _woice ;
int32_t    _start_volume;

ptuiKnob*  _knob_volume;

bool _cursor_free     (       ptuiCursor* p_cur )       override;
bool _cursor_drag_unit(       ptuiCursor* p_cur )       override;
void _put             ( const ptuiCursor *p_cur ) const override;

bool _search_button   ( const ptuiCursor* p_cur, int32_t* p_tgt_id ) const;

bool _volume_get      ( _KNOB_ID id, int32_t* p_volume ) const;
bool _volume_set      ( _KNOB_ID id, int32_t volume    );
bool _volume_default  ( _KNOB_ID id                    );

void _release();

public:

ptuiRackHalf();
~ptuiRackHalf();

bool init( WoiceUnit* woice );
};

#endif
