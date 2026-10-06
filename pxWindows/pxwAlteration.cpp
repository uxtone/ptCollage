



#include "./pxwAlteration.h"

pxwAlteration:: pxwAlteration()
{
	_hwnd   = NULL ;
	_b_alte = false;
}

void pxwAlteration::set_window( HWND hwnd )
{
	_hwnd   = hwnd;
}

void pxwAlteration::off()
{
	if( _hwnd )
	{
		uxSS<MAX_PATH> str = { 0 };
		GetWindowText( _hwnd, uxTOut( str ), MAX_PATH );
		int a = strlen( str );
		if( a && str[a-1] == '*' ){ str[a-1] = '\0'; SetWindowText( _hwnd, uxT( str ) ); }
	}

	_b_alte = false;
}

void pxwAlteration::set  ()
{
	if( _hwnd )
	{
		uxSS<MAX_PATH> str = { 0 };
		GetWindowText( _hwnd, uxTOut( str ), MAX_PATH );
		int a = strlen( str );
		if( a && str[a-1] != '*' ){ strcat( str, "*" ); SetWindowText( _hwnd, uxT( str ) ); }
	}
	_b_alte = true ;
}

bool pxwAlteration::is   () const
{
	return _b_alte;
}

