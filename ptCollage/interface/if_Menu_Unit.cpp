

#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "Interface.h"
#include "SurfaceNo.h"
#include "if_BaseField.h"
#include "if_Cursor.h"
#include "if_Menu_Unit.h"


#define _MENU_WIDTH      128
#define _MENU_HEIGHT      48

#define _BUTTON_EDIT_X     8
#define _BUTTON_EDIT_Y     8
#define _BUTTON_REMOVE_X   8
#define _BUTTON_REMOVE_Y  24

#define _BUTTON_WIDTH    112
#define _BUTTON_HEIGHT    16


typedef struct
{
	bool          bActive;
	bool          bTop   ;
	int32_t       u      ;
	float         x, y   ;
	enum_UnitMenu menu   ;
}
_MENUSTRUCT;

static _MENUSTRUCT _menu    = {0};
static fRECT       _rcMenu  = {0};
static fRECT       _rc_view = {0};

void if_Menu_Unit_Initialize()
{
	_menu.bActive = false;
}

void if_Menu_Unit_SetRect( const fRECT *rc_view )
{
	_rc_view = *rc_view;

	if( !_menu.bActive )
	{
		memset( &_rcMenu, 0, sizeof(_rcMenu) );
		return;
	}

	if( _menu.x < 8 ) _menu.x = 8;
	if( _menu.y < 8 ) _menu.y = 8;
	if( _menu.x > rc_view->r - _MENU_WIDTH  - 8 )
		_menu.x = rc_view->r - _MENU_WIDTH  - 8;
	if( _menu.y > rc_view->b - _MENU_HEIGHT - 8 )
		_menu.y = rc_view->b - _MENU_HEIGHT - 8;

	_rcMenu.l = _menu.x;
	_rcMenu.t = _menu.y;
	_rcMenu.r = _menu.x + _MENU_WIDTH;
	_rcMenu.b = _menu.y + _MENU_HEIGHT;
}

void if_Menu_Unit_SetPosition( float x, float y, int32_t u )
{
	if( y + UNITNAME_HEIGHT + _MENU_HEIGHT < _rc_view.b - 8 ) _menu.bTop = false;
	else                                                      _menu.bTop = true;

	_menu.bActive = true;
	_menu.x       = x;
	_menu.u       = u;
	if( !_menu.bTop ) _menu.y = y + UNITNAME_HEIGHT;
	else              _menu.y = y - _MENU_HEIGHT;
	_menu.menu    = enum_UnitMenu_None;

	if( _menu.x < 8 ) _menu.x = 8;
	if( _menu.y < 8 ) _menu.y = 8;
	if( _menu.x > _rc_view.r - _MENU_WIDTH  - 8 )
		_menu.x = _rc_view.r - _MENU_WIDTH  - 8;
	if( _menu.y > _rc_view.b - _MENU_HEIGHT - 8 )
		_menu.y = _rc_view.b - _MENU_HEIGHT - 8;
}

void if_Menu_Unit_GetPosition( float *px, float *py ){ *px = _menu.x; *py = _menu.y; }

int32_t if_Menu_Unit_GetUnitIndex(){ return _menu.u; }



void if_Menu_Unit_Close(){ _menu.bActive = false; }
bool if_Menu_Unit_IsTop(){ return _menu.bTop; }

fRECT* if_Menu_Unit_GetRect(){ return &_rcMenu; }

bool if_Menu_Unit_IsFrame( float x, float y )
{
	if( !_menu.bActive ) return false;

	if( x >= _menu.x               &&
		x <  _menu.x + _MENU_WIDTH  &&
		y >= _menu.y               &&
		y <  _menu.y + _MENU_HEIGHT ) return true;
	return false;
}

enum_UnitMenu if_Menu_Unit_GetMenu( float x, float y )
{
	if( !_menu.bActive ) return enum_UnitMenu_None;
	if( x >= _menu.x + _BUTTON_EDIT_X                    &&
		x <  _menu.x + _BUTTON_EDIT_X + _BUTTON_WIDTH    &&
		y >= _menu.y + _BUTTON_EDIT_Y                    &&
		y <  _menu.y + _BUTTON_EDIT_Y + _BUTTON_HEIGHT   ){
		_menu.menu = enum_UnitMenu_Edit;

	}else if(
		x >= _menu.x + _BUTTON_REMOVE_X                  &&
		x <  _menu.x + _BUTTON_REMOVE_X + _BUTTON_WIDTH  &&
		y >= _menu.y + _BUTTON_REMOVE_Y                  &&
		y <  _menu.y + _BUTTON_REMOVE_Y + _BUTTON_HEIGHT ){
		_menu.menu = enum_UnitMenu_Remove;
	}else{
		_menu.menu = enum_UnitMenu_None;
	}
	return	_menu.menu;
}

void if_Menu_Unit_Put()
{
	fRECT rc[] =
	{
		{256,128,384,176},
	};

	fRECT rcActive[] =
	{
		{392,136,504,152},
		{392,152,504,168},
	};

	fRECT rcBox = {256,176,384,192};
	float y;

	if( !_menu.bActive ) return;

	if( _menu.bTop ) y = _menu.y + _MENU_HEIGHT;
	else             y = _menu.y - UNITNAME_HEIGHT;

	g_dxdraw->tex_Put_View( _menu.x, _menu.y, &rc[0], SURF_MENUS );
	g_dxdraw->tex_Put_View( _menu.x,       y, &rcBox, SURF_MENUS );

	switch( _menu.menu )
	{
	case enum_UnitMenu_Edit  : g_dxdraw->tex_Put_View( _menu.x + _BUTTON_EDIT_X  , _menu.y + _BUTTON_EDIT_Y  , &rcActive[ 0 ], SURF_MENUS ); break;
	case enum_UnitMenu_Remove: g_dxdraw->tex_Put_View( _menu.x + _BUTTON_REMOVE_X, _menu.y + _BUTTON_REMOVE_Y, &rcActive[ 1 ], SURF_MENUS ); break;
	}
}
