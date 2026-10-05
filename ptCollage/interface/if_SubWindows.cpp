
#include "../../Generic/KeyControl.h"

#include "if_Copier.h"
#include "if_Effector.h"
#include "if_Player.h"
#include "if_Projector.h"

#include "if_SubWindows.h"

#define _SUBWINDOWSNUM 4


static SUBWINDOWSTRUCT* _handles[ _SUBWINDOWSNUM ];

void if_SubWindows_Initialize()
{
	memset( _handles, 0, sizeof(SUBWINDOWSTRUCT*) * _SUBWINDOWSNUM );
}

void if_SubWindows_Release()
{
	for( int i = 0; i < _SUBWINDOWSNUM; i++ )
	{
		if( _handles[ i ] )
		{
			free( _handles[ i ] );
			_handles[ i ] = NULL;
		}
	}
}

SUBWINDOWSTRUCT* if_SubWindows_Create( PUTFUNCTION func_put, CURSORFUNCTION func_cur )
{
	for( int i = 0; i < _SUBWINDOWSNUM; i++ )
	{
		if( !_handles[ i ] )
		{
			_handles[ i ] = (SUBWINDOWSTRUCT*)malloc( sizeof(SUBWINDOWSTRUCT) );
			_handles[ i ]->func_put = func_put;
			_handles[ i ]->func_cur = func_cur;
			return _handles[ i ];
		}
	}
	return NULL;
}


void if_SubWindows_Put()
{
	for( int i = _SUBWINDOWSNUM - 1; i >= 0; i-- )
	{
		_handles[ i ]->func_put();
	}
}

bool if_Cursor_Action_SubWindow( float cur_x, float cur_y )
{
	for( int i = 0; i < _SUBWINDOWSNUM; i++ )
	{
		if( _handles[ i ] && _handles[ i ]->func_cur( cur_x, cur_y ) ) return true;
	}
	return false;
}

void if_SubWindows_SetFocus( SUBWINDOWSTRUCT* handle )
{
	for( int i = 0; i < _SUBWINDOWSNUM; i++ )
	{
		if( _handles[ i ] == handle )
		{
			for( i = i; i > 0; i-- )
			{
				SUBWINDOWSTRUCT* work = _handles[ i - 1 ];
				_handles[ i - 1 ]     = _handles[ i ];
				_handles[ i ]         = work;
			}
			return;
		}
	}
}
