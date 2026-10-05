
#include <pxtonewinXA2.h>
extern pxtonewinXA2* g_strm_xa2;

#include "../../Generic/KeyControl.h"
#include "../../Generic/if_gen_Scroll.h"

#include "../ScopeEvent.h"

#include "if_Menu_Scope.h"
#include "if_ScopeField.h"
#include "if_BaseField.h"

#include "if_Panel_Scale.h"

#include "if_ToolPanel.h"
#include "if_KeyField.h"
#include "if_Copier.h"

extern if_gen_Scroll  g_ScrlEventH;
extern if_gen_Scroll  g_ScrlWoiceV;
extern if_gen_Scroll  g_ScrlUnitV ;
extern if_gen_Scroll  g_ScrlKeyV  ;

bool Function_IDM_UNDOEVENT();
bool Function_IDM_REDOEVENT();

bool pxtoneProject_IDM_SAVEPROJECT( HWND hWnd, bool b_as );

bool if_ShortcutKey_Action( HWND hWnd )
{
	int32_t clock1, clock2;
	fRECT*  p_rect;
	bool b_l_clk = KeyControl_IsClickLeft       ();
	bool b_l_trg = KeyControl_IsClickLeftTrigger();

	p_rect  = if_BaseField_GetRect();

	// ショートカット
	if( !g_strm_xa2->tune_is_sampling() && KeyControl_IsKey( VK_CONTROL ) )
	{
		if(      KeyControl_IsKeyTrigger( 'Z' ) ){ Function_IDM_UNDOEVENT(); return true; }// undo.
		else if( KeyControl_IsKeyTrigger( 'Y' ) ){ Function_IDM_REDOEVENT(); return true; }// redo.
		else if( KeyControl_IsKeyTrigger( 'S' ) ){ pxtoneProject_IDM_SAVEPROJECT( hWnd, false ); KeyControl_Clear(); }// save.
		else if( KeyControl_IsKeyTrigger( 'A' ) ){ if_Menu_Scope_SetActive( false ); if_ScopeField_SetScopeAll(); return true; }// select all.
		else if( KeyControl_IsKeyTrigger( 'C' ) ){ if_Menu_Scope_SetActive( false ); if_ScopeField_GetSelected( &clock1, &clock2 ); ScopeEvent_Copy( clock1, clock2, if_Copier_GetEventKinds() ); } // copy.
		else if( KeyControl_IsKeyTrigger( 'V' ) ){ if_Menu_Scope_SetActive( false ); if_ScopeField_GetSelected( &clock1, &clock2 ); ScopeEvent_Paste( clock1,     1, if_Copier_GetEventKinds() ); return true; }// paste.
		else if( KeyControl_IsKeyTrigger( 'X' ) )
		{
			if_Copier_AllCheck();
			if_Menu_Scope_SetActive( false );
			if_ScopeField_GetSelected( &clock1, &clock2 );
			ScopeEvent_Cut( clock1, clock2 );
			return true;
		}// cut.
	}

	if( KeyControl_IsKeyTrigger( VK_LEFT  ) ){ g_ScrlEventH.Scroll( p_rect->r - p_rect->l, -1 ); return true; }
	if( KeyControl_IsKeyTrigger( VK_RIGHT ) ){ g_ScrlEventH.Scroll( p_rect->r - p_rect->l,  1 ); return true; }
	if( KeyControl_IsKeyTrigger( VK_F1    ) ){ if_ToolPanel_SetMode_W();                         return true; }
	if( KeyControl_IsKeyTrigger( VK_F2    ) ){ if_ToolPanel_SetMode_U();                         return true; }
	if( KeyControl_IsKeyTrigger( VK_F3    ) ){ if_ToolPanel_SetMode_K();                         return true; }

	if( if_ToolPanel_GetMode() == enum_ToolMode_K )
	{
		fRECT* p_rc = if_KeyField_GetRect();
		if( KeyControl_IsKeyTrigger( VK_UP    ) ){ g_ScrlKeyV.Scroll( p_rc->b - p_rc->t, -1 ); return true; }
		if( KeyControl_IsKeyTrigger( VK_DOWN  ) ){ g_ScrlKeyV.Scroll( p_rc->b - p_rc->t,  1 ); return true; }
	}

	if( KeyControl_IsKey( VK_CONTROL ) )
	{
		if( if_ToolPanel_GetMode() != enum_ToolMode_W )
		{
			if( KeyControl_GetWheel() > 0 ){ if_Panel_Scale_ShiftZoom( false ); return true; }
			if( KeyControl_GetWheel() < 0 ){ if_Panel_Scale_ShiftZoom( true  ); return true; }
		}
	}
	else if( KeyControl_IsKey( VK_SHIFT ) )
	{
		if( KeyControl_GetWheel() > 0 ){ g_ScrlEventH.PostMove(  -1.5 ); g_ScrlEventH.Action( 0, 0, b_l_clk, b_l_trg ); return true; }
		if( KeyControl_GetWheel() < 0 ){ g_ScrlEventH.PostMove(  +1.5 ); g_ScrlEventH.Action( 0, 0, b_l_clk, b_l_trg ); return true; }
	}
	else
	{
		if( KeyControl_GetWheel() > 0 )
		{
			g_ScrlKeyV  .PostMove( -1.5 ); g_ScrlKeyV  .Action( 0, 0, b_l_clk, b_l_trg );
			g_ScrlUnitV .PostMove( -1.5 ); g_ScrlUnitV .Action( 0, 0, b_l_clk, b_l_trg );
			g_ScrlWoiceV.PostMove( -1.5 ); g_ScrlWoiceV.Action( 0, 0, b_l_clk, b_l_trg );
			return true;
		}
		if( KeyControl_GetWheel() < 0 )
		{
			g_ScrlKeyV  .PostMove( +1.5 ); g_ScrlKeyV  .Action( 0, 0, b_l_clk, b_l_trg );
			g_ScrlUnitV .PostMove( +1.5 ); g_ScrlUnitV .Action( 0, 0, b_l_clk, b_l_trg );
			g_ScrlWoiceV.PostMove( +1.5 ); g_ScrlWoiceV.Action( 0, 0, b_l_clk, b_l_trg );
			return true;
		}
	}

	return false;
}