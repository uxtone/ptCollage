// '15/12/17 pxwrMouse.h

#ifndef pxwrMouse_H
#define pxwrMouse_H

#include <pxStdDef.h>

class pxwrMouse
{
private:

void*   _hwnd     ;
bool    _b_capture;

int     _x;
int     _y;

uint8_t _flags_now ;
uint8_t _flags_old ;
uint8_t _flags_trg ;
uint8_t _flags_ntrg;

public:
pxwrMouse( void *hwnd );

bool set_position();
void set_click_l ( bool b );
void set_click_r ( bool b );

void    trigger_update();

int     get_x   () const;
int     get_y   () const;
uint8_t get_now () const;
uint8_t get_trg () const;
uint8_t get_ntrg() const;
};

#endif
