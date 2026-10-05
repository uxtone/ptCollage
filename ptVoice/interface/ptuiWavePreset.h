// '17/01/14 if_Presetクラス化.
// '17/02/18 ptuiクラスを継承 ptuiPre set.
// '17/03/07 -> ptuiWavePreset.

#ifndef ptuiWavePreset_H
#define ptuiWavePreset_H

#include "../WoiceUnit.h"
#include "./ptuiCursor.h"
#include "./ptui.h"
#include "./ptuiSwitch.h"

enum ptvWAVEPRESET
{
	ptvWAVEPRESET_Sine =  0,
	ptvWAVEPRESET_Triangle ,
	ptvWAVEPRESET_Sow      ,
	ptvWAVEPRESET_Rectangle,
	ptvWAVEPRESET_Random   ,
	ptvWAVEPRESET_Reverse  ,
	ptvWAVEPRESET_num      ,
};

class ptuiWavePreset: public ptui
{
private:

bool         _b_init;
WoiceUnit*   _woice ;
ptuiSwitch** _btns  ;

bool _cursor_free      (       ptuiCursor* p_cur )       override;
bool _cursor_click_hold(       ptuiCursor* p_cur )       override;
void _put              ( const ptuiCursor *p_cur ) const override;

bool _search_button( const ptuiCursor* p_cur, int32_t* p_tgt_id ) const;

void _release();

bool _module_overtone ( pxtnVOICEUNIT *p_vc, ptvWAVEPRESET index );
bool _module_coodinate( pxtnVOICEUNIT *p_vc, ptvWAVEPRESET index );
void _do_preset       (                      ptvWAVEPRESET index );

public:

ptuiWavePreset();
~ptuiWavePreset();

bool init( WoiceUnit* woice );

};

#endif
