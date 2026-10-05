// .
// '16/10/17 pxtoneToollib -> pxtonewinWoice.


#ifndef pxtonewinWoice_H
#define pxtonewinWoice_H

#include "./pxtonewinXA2.h"

#include <tchar.h>

class pxtonewinWoice
{
private:

bool                    _b_init  ;

int32_t                 _wave_id ;
TCHAR                   _status_text[ MAX_PATH ];
int32_t                 _sps     ;

pxtnPulse_NoiseBuilder* _ptn_bldr;
pxtnPulse_Frequency*    _freq    ;
pxtnWoice*              _woice   ;
pxtonewinXA2*           _strm_xa2;

void _release  ();

bool _load_and_play_PCM ( const TCHAR* path, bool b_loop, int key, bool* pb_timer );
bool _load_and_play_PTV ( const TCHAR* path,              int key, bool* pb_timer );
bool _load_and_play_PTN ( const TCHAR* path, bool b_loop, int key, bool* pb_timer );
bool _load_and_play_OGGV( const TCHAR* path, bool b_loop, int key, bool* pb_timer );

public:
pxtonewinWoice();
~pxtonewinWoice();
bool init         ( pxtonewinXA2 *strm_xa2 );
bool load_and_play( const TCHAR* path, bool b_loop, int key, bool* pb_timer );
void stop         ( bool b_force );
bool get_text     ( TCHAR* txt_info, bool b_jp ) const;
void set_sps      ( int sps );
};

#endif
