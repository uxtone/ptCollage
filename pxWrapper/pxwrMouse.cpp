// '15/12/17 pxwrMouse.cpp

#include "./pxwrMouse.h"

pxwrMouse::pxwrMouse( void* hwnd )
{
	_hwnd       = hwnd ;
	_x          =     0;
	_y          =     0;
	_flags_now  =     0;
	_flags_old  =     0;
	_flags_trg  =     0;
	_flags_ntrg =     0;

	_b_capture  = false;
}

void pxwrMouse::trigger_update()
{
	_flags_trg  =  _flags_now & ~_flags_old;
	_flags_ntrg = ~_flags_now &  _flags_old;
	_flags_old  =  _flags_now;
}

// action..

bool pxwrMouse::set_position()
{
#ifdef _WIN32
	if( !_hwnd ) return false;
	POINT pt;
	GetCursorPos( &pt );
	ScreenToClient( (HWND)_hwnd, &pt );
	_x = pt.x;
	_y = pt.y;
#else
#endif
	return true;
}

void pxwrMouse::set_click_l( bool b )
{
#ifdef _WIN32
	if( b ){ _flags_now |=  pxMOUSEBIT_L; if( !_b_capture ){ SetCapture( (HWND)_hwnd ); _b_capture = true ; } }
	else   { _flags_now &= ~pxMOUSEBIT_L; if(  _b_capture ){ ReleaseCapture();          _b_capture = false; } }
#else
	yet.
#endif
}

void pxwrMouse::set_click_r( bool b )
{
#ifdef _WIN32
	if( b ){ _flags_now |=  pxMOUSEBIT_R; if( !_b_capture ){ SetCapture( (HWND)_hwnd ); _b_capture = true ; } }
	else   { _flags_now &= ~pxMOUSEBIT_R; if(  _b_capture ){ ReleaseCapture();          _b_capture = false; } }
#else
	yet.
#endif
}

// const..

int  pxwrMouse::get_x() const
{
	return _x;
}
int  pxwrMouse::get_y() const
{
	return _y;
}

uint8_t  pxwrMouse::get_now () const{ return _flags_now ; }
uint8_t  pxwrMouse::get_trg () const{ return _flags_trg ; }
uint8_t  pxwrMouse::get_ntrg() const{ return _flags_ntrg; }
