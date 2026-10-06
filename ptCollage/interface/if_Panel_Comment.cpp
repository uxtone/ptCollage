


#include <pxtnService.h>
extern pxtnService *g_pxtn;


#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../../Generic/KeyControl.h"
#include "../../Generic/if_Generic.h"

#include "SurfaceNo.h"
#include "Interface.h"
#include "if_Panel_Comment.h"

#define _BUTTON_X 16
#define _BUTTON_Y  4
#define _BUTTON_W 96
#define _BUTTON_H 16


static fRECT _rcPanel = {0};


void if_Panel_Comment_SetRect( const fRECT *rc_view )
{
	float x = rc_view->r - PANEL_RACK_W;

	if( x < 0 ) x = 0;

	_rcPanel.l = x                           ;
	_rcPanel.t = rc_view->t + PANEL_RACK_H   ;
	_rcPanel.r = _rcPanel.l + PANEL_SCALE_W  ;
	_rcPanel.b = _rcPanel.t + PANEL_SCALE_H  ;
}

fRECT* if_Panel_Comment_GetRect()
{
	return &_rcPanel;
}

void if_Panel_Comment_Put()
{
	fRECT rcPanel = {128, 48,264, 72};
	fRECT rcOn    = {264, 48,272, 72};

	g_dxdraw->tex_Put_View( _rcPanel.l, _rcPanel.t, &rcPanel, SURF_TABLES );

	if( g_pxtn->text->is_comment_buf() )
		g_dxdraw->tex_Put_View( _rcPanel.l + 24, _rcPanel.t, &rcOn, SURF_TABLES );
}

#include <pxwAlteration.h>
extern pxwAlteration* g_alte;

INT_PTR CALLBACK
dlg_Comment( HWND hWnd, UINT msg, WPARAM w, LPARAM l );
extern HINSTANCE g_hInst    ;
extern HWND      g_hWnd_Main;

bool if_Panel_Comment_HitButton( float cur_x, float cur_y )
{
	const fRECT* p_rc = if_Panel_Comment_GetRect();

	// 表示倍率変更
	if( cur_x > p_rc->l + _BUTTON_X && cur_x < p_rc->l + _BUTTON_X + _BUTTON_W &&
		cur_y > p_rc->t + _BUTTON_Y && cur_y < p_rc->t + _BUTTON_Y + _BUTTON_H )
	{
		if( DialogBox( g_hInst, uxT( "DLG_COMMENT" ), g_hWnd_Main, dlg_Comment ) ) g_alte->set();
		KeyControl_Clear();
		return true;
	}
	return false;
}
