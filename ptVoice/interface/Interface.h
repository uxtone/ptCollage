
// '17/02/28 Interfac e_Xxxx をクラス化。


#ifndef Interface_H
#define Interface_H

#include <pxStdDef.h>

#include <pxMouse.h>

#include <pxtnWoice.h>

#include "../ptVoice.h"

#include "./ptuiCursor.h"
#include "./ptui.h"

#include "./ptuiHeader.h"
#include "./ptuiRackFull.h"
#include "./ptuiWave_mini.h"
#include "./ptuiEnve_mini.h"
#include "./ptuiRackHalf.h"
#include "./ptuiWavePreset.h"
#include "./ptuiWorkTemp.h"
#include "./ptuiWaveHead.h"
#include "./ptuiEnveHead.h"
#include "./ptuiVelocity.h"
#include "./ptuiSines.h"
#include "./ptuiReleaseTime.h"
#include "./ptuiOrganKey.h"
#include "./ptuiWave.h"
#include "./ptuiEnvelope.h"


enum PTUI_INDEX
{
	PTUI_Header =  0,
	PTUI_WorkTemp   ,

	PTUI_Rack0      ,
	PTUI_Rack0_wave ,
	PTUI_Rack0_enve ,
	PTUI_Rack1      ,
	PTUI_Rack1_wave ,
	PTUI_Rack1_enve ,

	PTUI_RackHalf   ,
	PTUI_WaveScreen ,
	PTUI_WaveHead   ,
	PTUI_Sines      ,
	PTUI_WavePreset ,
	PTUI_EnveScreen ,
	PTUI_EnveHead    ,
	PTUI_ReleaseTime,
	PTUI_OrganKey   ,
	PTUI_Velocity   ,

	PTUI_num,
};

class Interface
{
private:

void operator = (const Interface& src){}
Interface       (const Interface& src){}

bool _b_init;

ptuiCursor*      _cursor   ;

ptuiHeader*      _header   ;
ptuiRackFull*    _rack_0   ;
ptuiWave_mini*   _rack_0_wa;
ptuiEnve_mini*   _rack_0_en;
ptuiRackFull*    _rack_1   ;
ptuiWave_mini*   _rack_1_wa;
ptuiEnve_mini*   _rack_1_en;
ptuiRackHalf*    _rack_half;

ptuiWavePreset*  _wavepre  ;
ptuiWorkTemp*    _worktemp ;
ptuiWaveHead*    _wavehead ;
ptuiEnveHead*    _envehead ;
ptuiVelocity*    _velocity ;
ptuiSines*       _sines    ;
ptuiReleaseTime* _rlstime  ;
ptuiOrganKey*    _organkey ;
ptuiWave*        _wavetbl  ;
ptuiEnvelope*    _envetbl  ;


ptui**           _ptuis   ;
int32_t          _ptui_num;
int32_t          _ptui_max;

void _release();
bool _key_control();
bool _mode_update();

public:

Interface();
~Interface();

bool init();

bool proc( const fRECT* rc_view, const pxMouse* mouse, bool b_draw );

bool organkey_reset();
bool map_redraw_set();
};

#endif
