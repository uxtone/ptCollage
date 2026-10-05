
#include <pxtnService.h>
extern pxtnService*  g_pxtn    ;

#include <pxtonewinXA2.h>
extern pxtonewinXA2* g_strm_xa2;

#include <pxwDx09Draw.h>
extern pxwDx09Draw*  g_dxdraw  ;

#include "../../Generic/KeyControl.h"


#include "../UnitFocus.h"
#include "../Unit.h"
#include "../Woice.h"

#include "if_WoiceTray.h"
#include "if_UnitTray.h"
#include "if_KeyField.h"
#include "if_ToolPanel.h"
#include "Interface.h"
#include "SurfaceNo.h"

#define _BUTTON_Y     24

#define _BUTTON_X0    16
#define _BUTTON_X1    40
#define _BUTTON_X2    64
#define _BUTTON_X3    88
#define _BUTTON_X4   112

extern HINSTANCE     g_hInst;
extern HWND          g_hWnd_Main;

static fRECT         _rcPanel  = {0};
static enum_ToolMode _mode     = enum_ToolMode_W;
static bool          _bKeyOnly = false;

enum_ToolMode if_ToolPanel_GetMode()
{
	return _mode;
}

bool if_ToolPanel_IsUseKeyOnly()
{
	return _bKeyOnly;
}

static void _SetMode( enum_ToolMode mode, bool bKeyOnly )
{
	_mode      = mode;
	_bKeyOnly  = bKeyOnly;
}

void if_ToolPanel_SetRect( const fRECT *rc_view )
{
	_rcPanel.l = rc_view->l;
	_rcPanel.r = rc_view->l + NAMETRAY_WIDTH;
	_rcPanel.t = rc_view->t + HEADER_H;
	_rcPanel.b = rc_view->b + HEADER_H + TOOLPANEL_HEIGHT;
}


void if_ToolPanel_Put()
{
	static const fRECT rc[] =
	{
		{  0, 88,144,136},
		{  0,136,144,184},
		{  0,184,144,232},
	};

	fRECT rcEdit     = { 16,232, 56,248};

	g_dxdraw->tex_Put_View( _rcPanel.l, _rcPanel.t, &rc[ _mode ], SURF_TABLES );

	if( _mode == enum_ToolMode_K && _bKeyOnly )
	{
		g_dxdraw->tex_Put_View( _rcPanel.l + _BUTTON_X0, _rcPanel.t + _BUTTON_Y, &rcEdit, SURF_TABLES );
	}
}

enum_ToolButton if_ToolPanel_GetHitButton( float cur_x, float cur_y )
{

	switch( _mode )
	{
	// Woice
	case enum_ToolMode_W:
		if( cur_y >= _rcPanel.t  + _BUTTON_Y  && cur_y < _rcPanel.t + _BUTTON_Y  + 16 )
		{
			if(      cur_x >= _rcPanel.l + _BUTTON_X0  && cur_x < _rcPanel.l + _BUTTON_X0  + 16 ) return enum_ToolButton_W_add;
		}
		else if( cur_y >= _rcPanel.t +  0 && cur_y < _rcPanel.t + 16 )
		{
			if(      cur_x >= _rcPanel.l + 44 && cur_x < _rcPanel.l +  83 ) return enum_ToolButton_Mode_U;
			else if( cur_x >= _rcPanel.l + 83 && cur_x < _rcPanel.l + 144 ) return enum_ToolButton_Mode_K;
		}
		break;

	// Unit
	case enum_ToolMode_U:
		if( cur_y >= _rcPanel.t + _BUTTON_Y  && cur_y < _rcPanel.t + _BUTTON_Y  + 16 )
		{
			if(      cur_x >= _rcPanel.l + _BUTTON_X0  && cur_x < _rcPanel.l + _BUTTON_X0  + 16 ) return enum_ToolButton_U_add ;
			else if( cur_x >= _rcPanel.l + _BUTTON_X3  && cur_x < _rcPanel.l + _BUTTON_X3  + 16 ) return enum_ToolButton_U_solo;
			else if( cur_x >= _rcPanel.l + _BUTTON_X4  && cur_x < _rcPanel.l + _BUTTON_X4  + 16 ) return enum_ToolButton_U_all ;
		}
		else if( cur_y >= _rcPanel.t  +  0 && cur_y < _rcPanel.t + 16 )
		{
			if(      cur_x >= _rcPanel.l +  0 && cur_x < _rcPanel.l +  44 ) return enum_ToolButton_Mode_W;
			else if( cur_x >= _rcPanel.l + 83 && cur_x < _rcPanel.l + 144 ) return enum_ToolButton_Mode_K;
		}
		break;

	// keyboard
	case enum_ToolMode_K:
		if( cur_y >= _rcPanel.t  + _BUTTON_Y  && cur_y < _rcPanel.t  + _BUTTON_Y  + 16 )
		{
			if(      cur_x >= _rcPanel.l + _BUTTON_X0 && cur_x < _rcPanel.l + _BUTTON_X0 + 16 ) return enum_ToolButton_K_pen;
			else if( cur_x >= _rcPanel.l + _BUTTON_X1 && cur_x < _rcPanel.l + _BUTTON_X1 + 16 ) return enum_ToolButton_K_hand;
			else if( cur_x >= _rcPanel.l + _BUTTON_X3 && cur_x < _rcPanel.l + _BUTTON_X3 + 16 )  return enum_ToolButton_K_up;
			else if( cur_x >= _rcPanel.l + _BUTTON_X4 && cur_x < _rcPanel.l + _BUTTON_X4 + 16 ) return enum_ToolButton_K_down;
		}
		else if( cur_y >= _rcPanel.t  +  0 && cur_y < _rcPanel.t  +  16 )
		{
			if(      cur_x >= _rcPanel.l +  0 && cur_x < _rcPanel.l +  44 ) return enum_ToolButton_Mode_W;
			else if( cur_x >= _rcPanel.l + 44 && cur_x < _rcPanel.l +  83 ) return enum_ToolButton_Mode_U;
		}

		break;
	default:
		break;
	}

	return enum_ToolButton_unknown;
}

void if_ToolPanel_SetMode_W(){ _mode = enum_ToolMode_W; _bKeyOnly = false; if_WoiceTray_JustScroll();                           }
void if_ToolPanel_SetMode_U(){ _mode = enum_ToolMode_U; _bKeyOnly = false; if_UnitTray_JustScroll( );                           }
void if_ToolPanel_SetMode_K(){ _mode = enum_ToolMode_K; _bKeyOnly = false; if_UnitTray_JustScroll( ); if_KeyField_JustScroll(); }

bool if_ToolPanel_SetHitButton( enum_ToolButton button )
{
	bool bDraw = false;

	switch( button )
	{
	case enum_ToolButton_Mode_W : if_ToolPanel_SetMode_W(); bDraw = true; break;
	case enum_ToolButton_Mode_U : if_ToolPanel_SetMode_U(); bDraw = true; break;
	case enum_ToolButton_Mode_K : if_ToolPanel_SetMode_K(); bDraw = true; break;

	case enum_ToolButton_W_add  :

		if( g_strm_xa2->tune_is_sampling() ) break;
		Woice_Dialog_Add();
		KeyControl_Clear();
		bDraw = true;
		break;

	case enum_ToolButton_U_add  :

		if( g_strm_xa2->tune_is_sampling() ) break;
		Unit_Dialog_Add();
		KeyControl_Clear();
		bDraw = true;
		break;

	case enum_ToolButton_U_all  :
		g_pxtn->Unit_SetOpratedAll( true );
		bDraw = true;
		break;

	case enum_ToolButton_U_solo:
		g_pxtn->Unit_Solo( UnitFocus_Get() );
		bDraw = true;
		break;

	case enum_ToolButton_K_pen  : _bKeyOnly = false; bDraw = true; break;
	case enum_ToolButton_K_hand :
		_bKeyOnly = true ; bDraw = true; break;

	case enum_ToolButton_K_up   : UnitFocus_Shift( false ); if_UnitTray_JustScroll(); bDraw = true; break;
	case enum_ToolButton_K_down : UnitFocus_Shift( true  ); if_UnitTray_JustScroll(); bDraw = true; break;
	default: break;
	}

	return bDraw;
}
