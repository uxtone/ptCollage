#ifndef if_gen_Scroll_H
#define if_gen_Scroll_H

#include <pxStdDef.h>

#include "../pxWindows/pxwDx09Draw.h"

#define SCROLLBUTTONSIZE 16

enum
{
	SCROLLACTION_GRAY,
	SCROLLACTION_ENABLE,
	SCROLLACTION_DRAG,
	SCROLLACTION_LEFT,
	SCROLLACTION_RIGHT,
	SCROLLACTION_body ,
};

class if_gen_Scroll
{

private:

pxwDx09Draw *_dxdraw;

int32_t  _surf;
bool     _bVertical;

int32_t  _act_no;
int32_t  _ani_no;

int32_t  _size        ;
int32_t  _offset      ;

int32_t  _start_cur   ;
int32_t  _start_offset;
int32_t  _cursor_move ;
int32_t  _body_move   ;
int32_t  _post_move   ;

fRECT    _rcBtn1;
fRECT    _rcBtn2;
fRECT    _rcKnob;

fRECT    _vw_rect;

uint32_t _counter;

bool     _b_frame_update;

if_gen_Scroll(              const if_gen_Scroll &src  ){               } // copy
if_gen_Scroll & operator = (const if_gen_Scroll &right){ return *this; } // substitution

public:
if_gen_Scroll();
~if_gen_Scroll();

void    Initialize( pxwDx09Draw *dxdraw, int32_t surf, bool bVertical );
void    SetSize   ( int32_t size   );
void    SetOffset ( int32_t offset );
int32_t GetOffset ();
int32_t GetSize   ();
void    SetMove   ( int32_t cursor_move, int32_t body_move );
void    SetRect   ( fRECT*  vw_rect );
bool    Action    ( int32_t cur_x, int32_t cur_y, bool b_l_clk, bool b_l_trg );
void    Put       ();
void    Scroll    ( int32_t view_width, int32_t move );
void    PostMove  ( int32_t move );

bool    is_frame_update() const;
};

#endif
