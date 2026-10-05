// '17/02/27 ptuiRack
// '17/03/03 connote ptuiTuning/ptuiPanpot. and ptuiVolume.
// '17/03/20 -> ptuiRackFull

#ifndef ptuiRackFull_H
#define ptuiRackFull_H

#include "../WoiceUnit.h"

#include "./ptuiCursor.h"
#include "./ptui.h"
#include "./ptuiKnob.h"
#include "./ptuiSwitch.h"

class ptuiRackFull: public ptui
{
private:

enum _UNIT_ID
{
	_UNIT_KNOB_volume = 0,
	_UNIT_KNOB_pan    ,
	_UNIT_KNOB_tuning ,

	_UNIT_BTN_mute    ,
	_UNIT_BTN_wave    ,
	_UNIT_BTN_enve    ,

	_UNIT_BTN_add_unit,

	_UNIT_num,
};

bool        _b_init      ;
WoiceUnit*  _woice       ;
int32_t     _start_volume;
int32_t     _unit_idx    ;

ptuiKnob*   _knob_volume ;
ptuiKnob*   _knob_pan    ;
ptuiKnob*   _knob_tuning ;

ptuiSwitch* _btn_mute    ;
ptuiSwitch* _btn_wave    ;
ptuiSwitch* _btn_enve    ;
ptuiSwitch* _btn_add_unit;

bool _cursor_free      (       ptuiCursor* p_cur )       override;
bool _cursor_drag_unit (       ptuiCursor* p_cur )       override;
bool _cursor_click_hold(       ptuiCursor* p_cur )       override;
void _put              ( const ptuiCursor* p_cur ) const override;

bool _search_a_knob    ( const ptuiCursor* p_cur, int32_t* p_tgt_id ) const;

bool _volume_get       ( _UNIT_ID id, int32_t* p_volume ) const;
bool _volume_set       ( _UNIT_ID id, int32_t volume    );
bool _volume_default   ( _UNIT_ID id                    );


void _release();

public:

ptuiRackFull();
~ptuiRackFull();

bool init( WoiceUnit* woice, int32_t unit_idx );
bool is_enable() const;
void mute_button_update();

};

#endif
