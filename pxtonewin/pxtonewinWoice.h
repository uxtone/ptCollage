// .
// '16/10/17 pxtoneToollib -> pxtonewinWoice.


#ifndef pxtonewinWoice_H
#define pxtonewinWoice_H


#include <pxStdDef.h>

#include "./pxtonewinXA2.h"

#include <tchar.h>

class pxtonewinWoice
{
private:

bool                    _b_init  ;

int32_t                 _wave_id ;
uxSS<MAX_PATH>                   _status_text;
int32_t                 _sps     ;

pxtnPulse_NoiseBuilder* _ptn_bldr;
pxtnPulse_Frequency*    _freq    ;
pxtnWoice*              _woice   ;
pxtonewinXA2*           _strm_xa2;

void _release  ();

bool _load_and_play_PCM ( const uxDS& path, bool b_loop, int key, bool* pb_timer );
bool _load_and_play_PTV ( const uxDS& path,              int key, bool* pb_timer );
bool _load_and_play_PTN ( const uxDS& path, bool b_loop, int key, bool* pb_timer );
bool _load_and_play_OGGV( const uxDS& path, bool b_loop, int key, bool* pb_timer );

public:
pxtonewinWoice();
~pxtonewinWoice();
bool init         ( pxtonewinXA2 *strm_xa2 );
bool load_and_play( const uxDS& path, bool b_loop, int key, bool* pb_timer );
void stop         ( bool b_force );
bool get_text     ( uxDS& txt_info, bool b_jp ) const;
void set_sps      ( int sps );
};

#endif
