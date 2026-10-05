// '17/01/15 pxwAlteration.

#ifndef pxwAlteration_H
#define pxwAlteration_H

#include <pxStdDef.h>

class pxwAlteration
{
private:
void operator = (const pxwAlteration& src){}
pxwAlteration   (const pxwAlteration& src){}

HWND _hwnd  ;
bool _b_alte;

public:
pxwAlteration();

void set_window( HWND hwnd );

void off();
void set();
bool is () const;
};

#endif