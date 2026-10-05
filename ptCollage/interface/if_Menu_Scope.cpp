
#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "Interface.h"
#include "SurfaceNo.h"
#include "if_BaseField.h"
#include "if_Cursor.h"
#include "if_Menu_Scope.h"
#include "if_ScopeField.h"

#define SCOPEMENU_POINT_WIDTH   88
#define SCOPEMENU_POINT_HEIGHT  64
#define SCOPEMENU_SCOPE_WIDTH   88
#define SCOPEMENU_SCOPE_HEIGHT 112

#define BUTTON_COPY_X       8
#define BUTTON_COPY_Y       8
#define BUTTON_CUT_X        8
#define BUTTON_CUT_Y       24
#define BUTTON_CLEAR_X      8
#define BUTTON_CLEAR_Y     40
#define BUTTON_DELETE_X     8
#define BUTTON_DELETE_Y    56
#define BUTTON_TRANSPOSE_X  8
#define BUTTON_TRANSPOSE_Y 72
#define BUTTON_SCOPE_X      8
#define BUTTON_SCOPE_Y     88

#define BUTTON_WIDTH    72
#define BUTTON_HEIGHT   16


typedef struct
{
	bool           bActive;
	bool           bPoint;
	float          x, y;
	enum_ScopeMenu menu;
}
SCOPEMENUSTRUCT;

static SCOPEMENUSTRUCT _scope_menu = {0};
static fRECT           _rc_view    = {0};
static fRECT           _rcPanel    = {0};

void if_Menu_Scope_Initialize()
{
	_scope_menu.bActive = false;
}

void if_Menu_Scope_SetRect( const fRECT *rc_view )
{
	float width, height;

	_rc_view = *rc_view;

	if( !_scope_menu.bActive )
	{
		memset( &_rcPanel, 0, sizeof(_rcPanel) );
		return;
	}
	if( _scope_menu.bPoint )
	{
		width  = SCOPEMENU_POINT_WIDTH;
		height = SCOPEMENU_POINT_HEIGHT;
	}else{
		width  = SCOPEMENU_SCOPE_WIDTH;
		height = SCOPEMENU_SCOPE_HEIGHT;
	}

	if( _scope_menu.x < 8 ) _scope_menu.x = 8;
	if( _scope_menu.y < 8 ) _scope_menu.y = 8;
	if( _scope_menu.x > rc_view->r - width  - 8 )
		_scope_menu.x = rc_view->r - width  - 8;
	if( _scope_menu.y > rc_view->b - height - 8 )
		_scope_menu.y = rc_view->b - height - 8;

	_rcPanel.l = _scope_menu.x;
	_rcPanel.t = _scope_menu.y;
	_rcPanel.r = _scope_menu.x + width;
	_rcPanel.b = _scope_menu.y + height;
}

void if_Menu_Scope_SetPosition( float x, float y, bool bCenter )
{
	int32_t width, height;
	int32_t clock1, clock2;

	_scope_menu.bActive = true;
	_scope_menu.x       = x;
	_scope_menu.y       = y;
	_scope_menu.menu    = enum_ScopeMenu_None;

	if_ScopeField_GetSelected( &clock1, &clock2 );
	if( clock1 == clock2 ) _scope_menu.bPoint = true ;
	else                   _scope_menu.bPoint = false;

	if( _scope_menu.bPoint )
	{
		width  = SCOPEMENU_POINT_WIDTH;
		height = SCOPEMENU_POINT_HEIGHT;
	}else{
		width  = SCOPEMENU_SCOPE_WIDTH;
		height = SCOPEMENU_SCOPE_HEIGHT;
	}

	if( bCenter ) _scope_menu.x -= width / 2;
	if( _scope_menu.x < 8 ) _scope_menu.x = 8;
	if( _scope_menu.y < 8 ) _scope_menu.y = 8;
	if( _scope_menu.x > _rc_view.r - width  - 8 )
		_scope_menu.x = _rc_view.r - width  - 8;
	if( _scope_menu.y > _rc_view.b - height - 8 )
		_scope_menu.y = _rc_view.b - height - 8;
}

void if_Menu_Scope_GetPosition( int32_t *px, int32_t *py ){ *px = _scope_menu.x; *py = _scope_menu.y; }

void if_Menu_Scope_SetActive( bool bActive )
{
	_scope_menu.bActive = bActive;
}

fRECT* if_Menu_Scope_GetRect()
{
	return &_rcPanel;
}

bool if_Menu_Scope_IsPanel( float x, float y )
{
	if( !_scope_menu.bActive ) return false;

	if( _scope_menu.bPoint ){
		if( x >= _scope_menu.x                    &&
			x <  _scope_menu.x + SCOPEMENU_POINT_WIDTH  &&
			y >= _scope_menu.y                    &&
			y <  _scope_menu.y + SCOPEMENU_POINT_HEIGHT ) return true;
	}else{
		if( x >= _scope_menu.x                    &&
			x <  _scope_menu.x + SCOPEMENU_SCOPE_WIDTH  &&
			y >= _scope_menu.y                    &&
			y <  _scope_menu.y + SCOPEMENU_SCOPE_HEIGHT ) return true;
	}
	return false;
}

enum_ScopeMenu if_Menu_Scope_GetMenu( float x, float y )
{
	if( !_scope_menu.bActive ) return enum_ScopeMenu_None;
	if(  _scope_menu.bPoint  )
	{
		if( x >= _scope_menu.x + BUTTON_COPY_X                 &&
			x <  _scope_menu.x + BUTTON_COPY_X + BUTTON_WIDTH  &&
			y >= _scope_menu.y + BUTTON_COPY_Y                 &&
			y <  _scope_menu.y + BUTTON_COPY_Y + BUTTON_HEIGHT ){
			_scope_menu.menu = enum_ScopeMenu_Paste;
		}else if(
			x >= _scope_menu.x + BUTTON_CUT_X                  &&
			x <  _scope_menu.x + BUTTON_CUT_X  + BUTTON_WIDTH  &&
			y >= _scope_menu.y + BUTTON_CUT_Y                  &&
			y <  _scope_menu.y + BUTTON_CUT_Y  + BUTTON_HEIGHT ){
			_scope_menu.menu = enum_ScopeMenu_Insert;
		}else if(
			x >= _scope_menu.x + BUTTON_CLEAR_X                 &&
			x <  _scope_menu.x + BUTTON_CLEAR_X + BUTTON_WIDTH  &&
			y >= _scope_menu.y + BUTTON_CLEAR_Y                 &&
			y <  _scope_menu.y + BUTTON_CLEAR_Y + BUTTON_HEIGHT ){
			_scope_menu.menu = enum_ScopeMenu_Scope;
		}else{
			_scope_menu.menu = enum_ScopeMenu_None;
		}
	}
	else
	{
		if( x >= _scope_menu.x + BUTTON_COPY_X                 &&
			x <  _scope_menu.x + BUTTON_COPY_X + BUTTON_WIDTH  &&
			y >= _scope_menu.y + BUTTON_COPY_Y                 &&
			y <  _scope_menu.y + BUTTON_COPY_Y + BUTTON_HEIGHT ){
			_scope_menu.menu = enum_ScopeMenu_Copy;
		}else if(
			x >= _scope_menu.x + BUTTON_CUT_X                  &&
			x <  _scope_menu.x + BUTTON_CUT_X  + BUTTON_WIDTH  &&
			y >= _scope_menu.y + BUTTON_CUT_Y                  &&
			y <  _scope_menu.y + BUTTON_CUT_Y  + BUTTON_HEIGHT ){
			_scope_menu.menu = enum_ScopeMenu_Cut;
		}else if(
			x >= _scope_menu.x + BUTTON_CLEAR_X                 &&
			x <  _scope_menu.x + BUTTON_CLEAR_X + BUTTON_WIDTH  &&
			y >= _scope_menu.y + BUTTON_CLEAR_Y                 &&
			y <  _scope_menu.y + BUTTON_CLEAR_Y + BUTTON_HEIGHT ){
			_scope_menu.menu = enum_ScopeMenu_Clear;
		}else if(
			x >= _scope_menu.x + BUTTON_DELETE_X                 &&
			x <  _scope_menu.x + BUTTON_DELETE_X + BUTTON_WIDTH  &&
			y >= _scope_menu.y + BUTTON_DELETE_Y                 &&
			y <  _scope_menu.y + BUTTON_DELETE_Y + BUTTON_HEIGHT ){
			_scope_menu.menu = enum_ScopeMenu_Delete;
		}else if(
			x >= _scope_menu.x + BUTTON_TRANSPOSE_X                 &&
			x <  _scope_menu.x + BUTTON_TRANSPOSE_X + BUTTON_WIDTH  &&
			y >= _scope_menu.y + BUTTON_TRANSPOSE_Y                 &&
			y <  _scope_menu.y + BUTTON_TRANSPOSE_Y + BUTTON_HEIGHT ){
			_scope_menu.menu = enum_ScopeMenu_Transpose;
		}else if(
			x >= _scope_menu.x + BUTTON_SCOPE_X                 &&
			x <  _scope_menu.x + BUTTON_SCOPE_X + BUTTON_WIDTH  &&
			y >= _scope_menu.y + BUTTON_SCOPE_Y                 &&
			y <  _scope_menu.y + BUTTON_SCOPE_Y + BUTTON_HEIGHT ){
			_scope_menu.menu = enum_ScopeMenu_Scope;
		}else{
			_scope_menu.menu = enum_ScopeMenu_None;
		}
	}
	return	_scope_menu.menu;
}

void if_Menu_Scope_Put()
{
	fRECT rc[] =
	{
		{  0,  0, 88,112},
		{  0,112, 88,176},
	};

	fRECT rcActive[] =
	{
		{ 96,  8,168, 24},// copy
		{ 96, 24,168, 40},// cut
		{ 96, 40,168, 56},// clear
		{ 96, 56,168, 72},// delete
		{ 96, 72,168, 88},// transpose
		{ 96, 88,168,104},// scope
		{ 96,120,168,136},// paste
		{ 96,136,168,152},// insert
	};

	if( !_scope_menu.bActive ) return;

	if( !_scope_menu.bPoint )
	{
		g_dxdraw->tex_Put_View( _scope_menu.x, _scope_menu.y, &rc[0], SURF_MENUS );
		switch( _scope_menu.menu )
		{
		case enum_ScopeMenu_Copy     : g_dxdraw->tex_Put_View( _scope_menu.x + BUTTON_COPY_X,      _scope_menu.y + BUTTON_COPY_Y     , &rcActive[ 0 ], SURF_MENUS ); break;
		case enum_ScopeMenu_Cut      : g_dxdraw->tex_Put_View( _scope_menu.x + BUTTON_CLEAR_X,     _scope_menu.y + BUTTON_CUT_Y      , &rcActive[ 1 ], SURF_MENUS ); break;
		case enum_ScopeMenu_Clear    : g_dxdraw->tex_Put_View( _scope_menu.x + BUTTON_CLEAR_X,     _scope_menu.y + BUTTON_CLEAR_Y    , &rcActive[ 2 ], SURF_MENUS ); break;
		case enum_ScopeMenu_Delete   : g_dxdraw->tex_Put_View( _scope_menu.x + BUTTON_DELETE_X,    _scope_menu.y + BUTTON_DELETE_Y   , &rcActive[ 3 ], SURF_MENUS ); break;
		case enum_ScopeMenu_Transpose: g_dxdraw->tex_Put_View( _scope_menu.x + BUTTON_TRANSPOSE_X, _scope_menu.y + BUTTON_TRANSPOSE_Y, &rcActive[ 4 ], SURF_MENUS ); break;
		case enum_ScopeMenu_Scope    : g_dxdraw->tex_Put_View( _scope_menu.x + BUTTON_SCOPE_X,     _scope_menu.y + BUTTON_SCOPE_Y    , &rcActive[ 5 ], SURF_MENUS ); break;
		}
	}
	else
	{
		g_dxdraw->tex_Put_View( _scope_menu.x, _scope_menu.y, &rc[1], SURF_MENUS );
		switch( _scope_menu.menu )
		{
		case enum_ScopeMenu_Paste :    g_dxdraw->tex_Put_View( _scope_menu.x + BUTTON_COPY_X,      _scope_menu.y + BUTTON_COPY_Y     , &rcActive[ 6 ], SURF_MENUS ); break;
		case enum_ScopeMenu_Insert:    g_dxdraw->tex_Put_View( _scope_menu.x + BUTTON_CLEAR_X,     _scope_menu.y + BUTTON_CUT_Y      , &rcActive[ 7 ], SURF_MENUS ); break;
		case enum_ScopeMenu_Scope :    g_dxdraw->tex_Put_View( _scope_menu.x + BUTTON_DELETE_X,    _scope_menu.y + BUTTON_CLEAR_Y    , &rcActive[ 5 ], SURF_MENUS ); break;
		}
	}
}
