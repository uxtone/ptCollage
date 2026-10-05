// '17/02/15 class "ptui".
// '17/02/28 + virtual destructor.

#ifndef ptui_H
#define ptui_H

#include <pxStdDef.h>

#include <pxtnWoice.h>

#include "../ptVoice.h"

#include "./ptuiCursor.h"

#define ptuiANCHOR_x1_ofs        0x01
#define ptuiANCHOR_x1_w          0x04
#define ptuiANCHOR_x2_ofs        0x08
#define ptuiANCHOR_x2_w          0x02
#define ptuiANCHOR_y1_ofs        0x10
#define ptuiANCHOR_y1_h          0x20
#define ptuiANCHOR_y2_ofs        0x40
#define ptuiANCHOR_y2_h          0x80

typedef struct
{
	uint32_t flags;
	int32_t  x1   ;
	int32_t  x2   ;
	int32_t  y1   ;
	int32_t  y2   ;
}
ptuiANCHOR;

class ptui
{
private:

void operator = (const ptui& src){}
ptui            (const ptui& src){}

protected:

bool       _b_base_init  ;
bool       _b_show       ;

int32_t    _id           ;

ptuiANCHOR _anchor       ;
fRECT      _rect         ;

bool       _b_cur_on     ;

int32_t    _ani_no_num   ;
int32_t*   _ani_nos      ;

int32_t    _cur_last_x   ;
int32_t    _cur_last_y   ;
int32_t    _drag_start_x ;
int32_t    _drag_start_y ;


bool _unit_anime_set( int32_t idx, int32_t no );

    // override as needed
virtual bool _proc_begin_sub    ();
virtual bool _cursor_free       ( ptuiCursor* p_cur );
virtual bool _cursor_drag_unit  ( ptuiCursor* p_cur );
virtual bool _cursor_click_hold ( ptuiCursor* p_cur );
virtual bool _cursor_click_hold2( ptuiCursor* p_cur );
virtual bool _cursor_key_hold   ( ptuiCursor* p_cur );
virtual bool _cursor_keyboard   ( ptuiCursor* p_cur );
virtual bool _cursor_scroll_h   ( ptuiCursor* p_cur );
virtual bool _cursor_scroll_v   ( ptuiCursor* p_cur );

virtual void _put              ( const ptuiCursor* p_cur ) const = 0;

bool _base_init   ( const ptuiANCHOR* p_anchor, int32_t ani_no_num );
void _base_release();

public:

ptui();
virtual ~ptui();

bool check_on_map( const ptuiCursor* p_cur );
void proc_begin  ( const fRECT *rc_vw );

bool cursor_action(       ptuiCursor* p_cur );

void put          ( const ptuiCursor* p_cur ) const;

bool show_set     ( bool b_show );
};

#endif
