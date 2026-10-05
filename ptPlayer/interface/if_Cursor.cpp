
#include <pxwDx09Draw.h>
extern pxwDx09Draw*  g_dxdraw  ;

#include <pxtonewinXA2.h>
extern pxtonewinXA2* g_strm_xa2;

#include "../../Generic/KeyControl.h"

#include "Interface.h"
#include "SurfaceNo.h"
#include "if_Cursor.h"

bool if_Cursor_Action_Player( float cur_x, float cur_y, PTP_SIGN *p_ptp_sign );

ACTIONCURSOR g_cursor = {0};


///////////////////////////////////////
// 以下はグローバル ///////////////////
///////////////////////////////////////

void if_Cursor_Initialize()
{
	g_cursor.action  = 0;
}

bool if_Cursor_Action( float cur_x, float cur_y, PTP_SIGN *p_ptp_sign )
{
	// フリー ========================================
	if( !g_cursor.action )
	{
		g_cursor.focus = ifCurFocus_None;
		while( 1 )
		{
			if( if_Cursor_Action_Player( cur_x, cur_y, p_ptp_sign ) ) break;
			if( g_strm_xa2->tune_is_sampling() ) break;
			break;
		}

		// ビジー ========================================
	}else{
		bool bDrag = false;

		switch( g_cursor.focus )
		{
		case ifCurFocus_Player: bDrag = if_Cursor_Action_Player( cur_x, cur_y, p_ptp_sign ); break;
		}
		return true;
	}

	if( g_cursor.focus == ifCurFocus_None ) return false;
	return true;
}


