// '17/03/07 ptuiEnve-SPS. copy from ptuiWave Tool.
// '17/03/19 ->ptuiEnve-Zoom.
// '17/03/20 ->ptuiEnveHead with close-button.

#ifndef ptuiEnveHead_H
#define ptuiEnveHead_H

#include <pxStdDef.h>

#include "../ptVoice.h"
#include "../WoiceUnit.h"

#include "./ptuiCursor.h"
#include "./ptuiSwitch.h"
#include "./ptui.h"

class ptuiEnveHead: public ptui
{
private:

enum _BTNID
{
	_BTNID_01 ,
	_BTNID_05 ,
	_BTNID_10 ,

	_BTNID_close,
	_BTNID_num,
};

bool         _b_init;
WoiceUnit*   _woice ;
ptuiSwitch** _btns  ;


bool _search_a_button  ( const ptuiCursor* p_cur, int32_t* p_idx );
bool _cursor_free      (       ptuiCursor* p_cur ) override;
bool _cursor_click_hold(       ptuiCursor* p_cur ) override;
void _put              ( const ptuiCursor *p_cur ) const override;

void _release();

public:
ptuiEnveHead();
~ptuiEnveHead();

bool init( WoiceUnit* woice );
};

#endif
