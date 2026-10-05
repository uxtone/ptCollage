

#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "Interface.h"
#include "SurfaceNo.h"
#include "if_BaseField.h"
#include "if_Cursor.h"
#include "if_Menu_Play.h"
#include "if_PlayField.h"


#define MENU_WIDTH   88
#define MENU_HEIGHT  80

#define BUTTON_SETREPEAT_X  8
#define BUTTON_SETREPEAT_Y  8
#define BUTTON_SETLAST_X    8
#define BUTTON_SETLAST_Y   24
#define BUTTON_CUTREPEAT_X  8
#define BUTTON_CUTREPEAT_Y 40
#define BUTTON_CUTLAST_X    8
#define BUTTON_CUTLAST_Y   56

#define BUTTON_WIDTH    72
#define BUTTON_HEIGHT   16


typedef struct
{
	bool          bActive;
	float         x, y;
	enum_PlayMenu menu;
}
_MENUSTRUCT;

static _MENUSTRUCT _menu   ;
static fRECT       _rcMenu ;
static fRECT       _rc_view;

void if_Menu_Play_Initialize()
{
	_menu.bActive = false;
}

// 座標
void if_Menu_Play_SetPosition( float x, float y, bool bCenter )
{
	_menu.bActive = true;
	_menu.x       = x;
	_menu.y       = y;
	_menu.menu    = enum_PlayMenu_None;

	if( bCenter ) _menu.x -= MENU_WIDTH / 2;
	if( _menu.x < 8 ) _menu.x = 8;
	if( _menu.y < 8 ) _menu.y = 8;
	if( _menu.x > _rc_view.r - MENU_WIDTH  - 8 )
		_menu.x = _rc_view.r - MENU_WIDTH  - 8;
	if( _menu.y > _rc_view.b - MENU_HEIGHT - 8 )
		_menu.y = _rc_view.b - MENU_HEIGHT - 8;
}

void if_Menu_Play_GetPosition( int32_t *px, int32_t *py ){ *px = _menu.x; *py = _menu.y; }

// 領域
void if_Menu_Play_SetRect( const fRECT *rc_view )
{
	_rc_view = *rc_view;

	if( !_menu.bActive )
	{
		memset( &_rcMenu, 0, sizeof(fRECT) );
		return;
	}

	if( _menu.x < 8 ) _menu.x = 8;
	if( _menu.y < 8 ) _menu.y = 8;
	if( _menu.x > _rc_view.r - MENU_WIDTH  - 8 )
		_menu.x = _rc_view.r - MENU_WIDTH  - 8;
	if( _menu.y > _rc_view.b - MENU_HEIGHT - 8 )
		_menu.y = _rc_view.b - MENU_HEIGHT - 8;

	_rcMenu.l = _menu.x;
	_rcMenu.t = _menu.y;
	_rcMenu.r = _menu.x + MENU_WIDTH ;
	_rcMenu.b = _menu.y + MENU_HEIGHT;
}

void if_Menu_Play_SetActive( bool bActive )
{
	_menu.bActive = bActive;
}

fRECT* if_Menu_Play_GetRect()
{
	return &_rcMenu;
}

bool if_Menu_Play_IsFrame( int32_t x, int32_t y )
{
	if( !_menu.bActive ) return false;

	if( x >= _menu.x               &&
		x <  _menu.x + MENU_WIDTH  &&
		y >= _menu.y               &&
		y <  _menu.y + MENU_HEIGHT ) return true;
	return false;
}

enum_PlayMenu if_Menu_Play_GetMenu( float x, float y )
{
	if( !_menu.bActive ) return enum_PlayMenu_None;
	if( x >= _menu.x + BUTTON_SETREPEAT_X                 &&
		x <  _menu.x + BUTTON_SETREPEAT_X + BUTTON_WIDTH  &&
		y >= _menu.y + BUTTON_SETREPEAT_Y                 &&
		y <  _menu.y + BUTTON_SETREPEAT_Y + BUTTON_HEIGHT )
	{
		_menu.menu = enum_PlayMenu_SetRepeat;
	}
	else if(
		x >= _menu.x + BUTTON_SETLAST_X                 &&
		x <  _menu.x + BUTTON_SETLAST_X + BUTTON_WIDTH  &&
		y >= _menu.y + BUTTON_SETLAST_Y                 &&
		y <  _menu.y + BUTTON_SETLAST_Y + BUTTON_HEIGHT )
	{
		_menu.menu = enum_PlayMenu_SetLast;
	}
	else if(
		x >= _menu.x + BUTTON_CUTREPEAT_X                 &&
		x <  _menu.x + BUTTON_CUTREPEAT_X + BUTTON_WIDTH  &&
		y >= _menu.y + BUTTON_CUTREPEAT_Y                 &&
		y <  _menu.y + BUTTON_CUTREPEAT_Y + BUTTON_HEIGHT )
	{
		_menu.menu = enum_PlayMenu_CutRepeat;
	}
	else if(
		x >= _menu.x + BUTTON_CUTLAST_X                 &&
		x <  _menu.x + BUTTON_CUTLAST_X + BUTTON_WIDTH  &&
		y >= _menu.y + BUTTON_CUTLAST_Y                 &&
		y <  _menu.y + BUTTON_CUTLAST_Y + BUTTON_HEIGHT )
	{
		_menu.menu = enum_PlayMenu_CutLast;
	}
	else
	{
		_menu.menu = enum_PlayMenu_None;
	}
	return	_menu.menu;
}

void if_Menu_Play_Put()
{
	fRECT rc[] =
	{
		{  0,256, 88,336},
	};

	fRECT rcActive[] =
	{
		{ 96,264,168,280},
		{ 96,280,168,296},
		{ 96,296,168,312},
		{ 96,312,168,328},
	};

	if( !_menu.bActive ) return;

	g_dxdraw->tex_Put_View( _menu.x, _menu.y, &rc[0], SURF_MENUS );

	switch( _menu.menu )
	{
	case enum_PlayMenu_SetRepeat: g_dxdraw->tex_Put_View( _menu.x + BUTTON_SETREPEAT_X, _menu.y + BUTTON_SETREPEAT_Y, &rcActive[ 0 ], SURF_MENUS ); break;
	case enum_PlayMenu_SetLast  : g_dxdraw->tex_Put_View( _menu.x + BUTTON_SETLAST_X  , _menu.y + BUTTON_SETLAST_Y  , &rcActive[ 1 ], SURF_MENUS ); break;
	case enum_PlayMenu_CutRepeat: g_dxdraw->tex_Put_View( _menu.x + BUTTON_CUTREPEAT_X, _menu.y + BUTTON_CUTREPEAT_Y, &rcActive[ 2 ], SURF_MENUS ); break;
	case enum_PlayMenu_CutLast  : g_dxdraw->tex_Put_View( _menu.x + BUTTON_CUTLAST_X  , _menu.y + BUTTON_CUTLAST_Y  , &rcActive[ 3 ], SURF_MENUS ); break;
	}
}
