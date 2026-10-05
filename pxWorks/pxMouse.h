// '15/12/17 pxMouse.h

#ifndef pxMouse_H
#define pxMouse_H

#include <pxStdDef.h>

class pxMouse
{
private:

void*   _mouse    ;
int32_t _mode_idx ;

int32_t _wheel    ;
int32_t _wheel_old;
int32_t _wheel_trg;


public:
pxMouse( void* hwnd );
~pxMouse();

void    set_click_l ( bool b    );
void    set_click_r ( bool b    );
void    set_position(           );
void    wheel       ( int32_t v );
int32_t wheel_get   () const;

void    trigger_update();

int     get_x       () const;
int     get_y       () const;
uint8_t get_now     () const;
uint8_t get_trg     () const;
uint8_t get_ntrg    () const;

void    set_mode_idx();
int32_t get_mode_idx() const;
};

#endif
