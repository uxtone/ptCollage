

#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "Interface.h"
#include "SurfaceNo.h"

#define _FRAME_W          88
#define _FRAME_L          96
#define _BUFFER_NUM        6

enum enum_TenkeyButton
{
	enum_TenkeyButton_0 = 0,
	enum_TenkeyButton_1,
	enum_TenkeyButton_2,
	enum_TenkeyButton_3,
	enum_TenkeyButton_4,
	enum_TenkeyButton_5,
	enum_TenkeyButton_6,
	enum_TenkeyButton_7,
	enum_TenkeyButton_8,
	enum_TenkeyButton_9,
	enum_TenkeyButton_c,
	enum_TenkeyButton_ok,
	enum_TenkeyButton_none,
};

typedef struct
{
	bool              bActive;
	float             x, y;
	enum_TenkeyButton button;
	int32_t           buf[ _BUFFER_NUM ];
	int32_t           buf_index;
	int32_t           u;
	int32_t           clock;
}
_TENKEYSTRUCT;

static _TENKEYSTRUCT _tenkey  = {0};
static fRECT         _rcFrame = {0};
static fRECT         _rc_view = {0};

void if_Tenkey_Tuning_Initialize()
{
	_tenkey.bActive   = false;
	_tenkey.buf_index = 0;
}

void if_Tenkey_Tuning_SetRect( const fRECT *rc_view )
{
	_rc_view = *rc_view;

	if( !_tenkey.bActive ){ memset( &_rcFrame, 0, sizeof(_rcFrame) ); return; }

	if( _tenkey.x > rc_view->r - _FRAME_W )
		_tenkey.x = rc_view->r - _FRAME_W;
	if( _tenkey.y > rc_view->b - _FRAME_L - SCROLL_HEIGHT )
		_tenkey.y = rc_view->b - _FRAME_L - SCROLL_HEIGHT;

	_rcFrame.l  = _tenkey.x;
	_rcFrame.t  = _tenkey.y;
	_rcFrame.r  = _tenkey.x + _FRAME_W;
	_rcFrame.b  = _tenkey.y + _FRAME_L;
}

void if_Tenkey_Tuning_SetActive( float x, float y, int32_t u, int32_t clock )
{
	_tenkey.bActive   = true;
	_tenkey.buf_index = 0;
	_tenkey.button    = enum_TenkeyButton_none;
	_tenkey.x         = x - _FRAME_W / 2;
	_tenkey.y         = y - _FRAME_L / 2;
	_tenkey.u         = u;
	_tenkey.clock     = clock;

	if( _tenkey.x > _rc_view.r - _FRAME_W )
		_tenkey.x = _rc_view.r - _FRAME_W;
	if( _tenkey.y > _rc_view.b - _FRAME_L - SCROLL_HEIGHT )
		_tenkey.y = _rc_view.b - _FRAME_L - SCROLL_HEIGHT;
}


void if_Tenkey_Tuning_Close()
{
	_tenkey.bActive = false;
}

fRECT* if_Tenkey_Tuning_GetRect()
{
	return &_rcFrame;
}

bool if_Tenkey_Tuning_IsFrame( float x, float y )
{
	if( !_tenkey.bActive ) return false;

	if( x >= _tenkey.x             &&
		x <  _tenkey.x + _FRAME_W  &&
		y >= _tenkey.y             &&
		y <  _tenkey.y + _FRAME_L ) return true;
	return false;
}

bool if_Tenkey_Tuning_ButtonDown( float x, float y )
{
	if( !_tenkey.bActive ) return false;

	_tenkey.button = enum_TenkeyButton_none;

	if(      x >= _tenkey.x + 10 && x <  _tenkey.x + 30 )
	{
		if(      y >= _tenkey.y + 26 && y <  _tenkey.y + 38 ) _tenkey.button = enum_TenkeyButton_7;
		else if( y >= _tenkey.y + 42 && y <  _tenkey.y + 54 ) _tenkey.button = enum_TenkeyButton_4;
		else if( y >= _tenkey.y + 58 && y <  _tenkey.y + 70 ) _tenkey.button = enum_TenkeyButton_1;
		else if( y >= _tenkey.y + 74 && y <  _tenkey.y + 86 ) _tenkey.button = enum_TenkeyButton_0;
	}
	else if( x >= _tenkey.x + 34 && x <  _tenkey.x + 54 )
	{
		if(      y >= _tenkey.y + 26 && y <  _tenkey.y + 38 ) _tenkey.button = enum_TenkeyButton_8;
		else if( y >= _tenkey.y + 42 && y <  _tenkey.y + 54 ) _tenkey.button = enum_TenkeyButton_5;
		else if( y >= _tenkey.y + 58 && y <  _tenkey.y + 70 ) _tenkey.button = enum_TenkeyButton_2;
		else if( y >= _tenkey.y + 74 && y <  _tenkey.y + 86 ) _tenkey.button = enum_TenkeyButton_c;
	}
	else if( x >= _tenkey.x + 58 && x <  _tenkey.x + 78 )
	{
		if(      y >= _tenkey.y + 26 && y <  _tenkey.y + 38 ) _tenkey.button = enum_TenkeyButton_9;
		else if( y >= _tenkey.y + 42 && y <  _tenkey.y + 54 ) _tenkey.button = enum_TenkeyButton_6;
		else if( y >= _tenkey.y + 58 && y <  _tenkey.y + 70 ) _tenkey.button = enum_TenkeyButton_3;
		else if( y >= _tenkey.y + 74 && y <  _tenkey.y + 86 ) _tenkey.button = enum_TenkeyButton_ok;
	}

	if( _tenkey.button == enum_TenkeyButton_none ) return false;

	return true;
}

bool if_Tenkey_Tuning_ButtonUp()
{
	if( _tenkey.button < enum_TenkeyButton_c )
	{
		if( _tenkey.buf_index < _BUFFER_NUM )
		{
			_tenkey.buf[ _tenkey.buf_index ] = _tenkey.button;
			_tenkey.buf_index++;
		}
	}
	else if( _tenkey.button == enum_TenkeyButton_c )
	{
		_tenkey.buf_index = 0;
	}
	else if( _tenkey.button == enum_TenkeyButton_ok )
	{
		return true;
	}
	_tenkey.button = enum_TenkeyButton_none;
	return false;
}

void if_Tenkey_Tuning_ButtonCancel()
{
	_tenkey.button = enum_TenkeyButton_none;
}

void if_Tenkey_Tuning_Put()
{
	fRECT rcFrame= {  0,  0,88, 96};

	if( !_tenkey.bActive ) return;

	g_dxdraw->tex_Put_View( _tenkey.x, _tenkey.y, &rcFrame, SURF_DIALOGS );

	{
		fRECT rcButton[] =
		{
			{ 88,80,112,96},
			{ 88,64,112,80},
			{ 88,48,112,64},
			{ 88,32,112,48},
			{ 88,16,112,32},
			{ 88, 0,112,16},
			{112,80,136,96},
			{112,64,136,80},
			{112,48,136,64},
			{112,32,136,48},
			{112,16,136,32},
			{112, 0,136,16},
		};

		POINT pt[] = {
			{ 8,72},

			{ 8,56}, {32,56}, {56,56},
			{ 8,40}, {32,40}, {56,40},
			{ 8,24}, {32,24}, {56,24},
			{32,72}, {56,72},
		};

		g_dxdraw->tex_Put_View(
			_tenkey.x + pt[ _tenkey.button ].x,
			_tenkey.y + pt[ _tenkey.button ].y,
			&rcButton[      _tenkey.button ], SURF_DIALOGS );
	}
	{
		fRECT rcNum[] = {
			{  0, 96,  8,104},
			{  8, 96, 16,104},
			{ 16, 96, 24,104},
			{ 24, 96, 32,104},
			{ 32, 96, 40,104},
			{ 40, 96, 48,104},
			{ 48, 96, 56,104},
			{ 56, 96, 64,104},
			{ 64, 96, 72,104},
			{ 72, 96, 80,104},
		};
		fRECT rcDot  = { 88, 96, 96,104};

		float x, y;
		x = _tenkey.x + 11;
		y = _tenkey.y + 10;
		for( int32_t i = 0; i < _tenkey.buf_index; i++ )
		{
			g_dxdraw->tex_Put_View( x, y, &rcNum[ _tenkey.buf[ i ] ], SURF_DIALOGS );
			if( !i ){ x += 8; g_dxdraw->tex_Put_View( x, y, &rcDot, SURF_DIALOGS ); }
			x += 8;
		}
	}
}

bool if_Tenkey_Tuning_GetResult( float* p_f, int32_t* p_u, int32_t* p_clock )
{
	float f = 0.0f;
	float w = 1.0f;
	if( _tenkey.bActive )
	{
		for( int32_t i = 0; i < _tenkey.buf_index; i++ )
		{
			f += w * _tenkey.buf[ i ];
			w = w / 10.0f;
		}
	}
	if( f > 9.99999f ) f = 9.99999f;
	if( f <= 0.0f    ) return false;

	*p_f     = f;
	*p_u     = _tenkey.u;
	*p_clock = _tenkey.clock;

	return true;
}