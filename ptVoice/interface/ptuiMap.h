// '17/02/18 ptuiMap.

#ifndef ptuiMap_H
#define ptuiMap_H

#include <pxStdDef.h>

#include "./ptui.h"

class ptuiMap: public ptui
{
private:
void operator = (const ptuiMap& src){}
ptuiMap         (const ptuiMap& src){}

protected:

bool      _b_map_init  ;

uint32_t* _p_map_view  ;
uint32_t* _p_map_posi  ;
uint32_t* _p_map_nega  ;
bool      _b_map_redraw;

int32_t   _surf_screen ;

virtual bool _try_update  (                                         ) = 0;
virtual bool _keyctrl_move( int32_t idx, int32_t mv_x, int32_t mv_y );
virtual bool _keyctrl_tab ( bool b_back                             );

void _map_release();
bool _map_init( int32_t map_w, int32_t map_h, int32_t surf_screen );

public:
ptuiMap();
~ptuiMap();

void redraw_set   ();
void redraw_clear ();

bool try_update  (                                         );
bool keyctrl_move( int32_t idx, int32_t mv_x, int32_t mv_y );
bool keyctrl_tab ( bool b_back                             );

bool Xxx ();
};

#endif