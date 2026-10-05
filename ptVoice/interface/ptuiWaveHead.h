// '17/02/24 if_Voice Type -> ptuiVoice Type.
// '17/03/04 -> ptuiWave-Tool.
// '17/03/21 -> ptuiWaveHead.

#ifndef ptuiWaveHead_H
#define ptuiWaveHead_H

#include <pxStdDef.h>

#include "../ptVoice.h"
#include "../WoiceUnit.h"

#include "./ptuiCursor.h"
#include "./ptuiSwitch.h"
#include "./ptui.h"


class ptuiWaveHead: public ptui
{
private:

enum _UNIT_ID
{
	_UNIT_cood     ,
	_UNIT_osci     ,
	_UNIT_btn_close,
	_UNIT_num      ,
};


bool         _b_init;
WoiceUnit*   _woice ;
ptuiSwitch** _btns  ;

bool _search_a_button   ( const ptuiCursor* p_cur, int32_t* p_idx );
bool _cursor_free       (       ptuiCursor* p_cur ) override;
bool _cursor_click_hold (       ptuiCursor* p_cur ) override;
void _put               ( const ptuiCursor *p_cur ) const override;

bool _set_wave_tool( pxtnVOICETYPE type );

void _release();

public:
ptuiWaveHead();
~ptuiWaveHead();

bool init( WoiceUnit* woice );
};

#endif
