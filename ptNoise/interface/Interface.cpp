

#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../../Generic/KeyControl.h"
#include "../../Generic/if_Generic.h"

#include "../PcmTable.h"

#include "SurfaceNo.h"
#include "Interface.h"
#include "if_Cursor.h"
#include "if_PcmTable.h"
#include "if_Panel_Scale.h"
#include "if_Panel_Logo.h"
#include "if_Free.h"
#include "if_Menu_Zoom.h"

// 変数群 =======================================-

static bool   _bInit = false;

if_gen_Scroll g_ScrlPCM_H;

// 関数群 =======================================-

// マウスカーソルの座標を取得
static void _GetMousePointer( HWND hWnd, float* p_cur_x, float* p_cur_y )
{
	POINT pt;

	GetCursorPos  ( &pt );
	ScreenToClient( hWnd, &pt );
	
	*p_cur_x = (float)pt.x / g_dxdraw->get_screen_mag();
	*p_cur_y = (float)pt.y / g_dxdraw->get_screen_mag();
}

static int32_t _CountFramePerSecond()
{
	static bool     bFirst    = true;
	static uint32_t wait      = 0;
	static uint32_t count     = 0;
	static int32_t  max_count = 0;

	if( bFirst ){ wait   = GetTickCount(); bFirst = false; }

	count++;
	if( wait + 1000 <= GetTickCount() )
	{
		wait     += 1000;
		max_count = count;
		count     = 0;
	}

	return max_count;
}

#define _FLIPWAIT     20

void Interface_Process( HWND hWnd, bool bDraw )
{
	float cur_x;
	float cur_y;
	fRECT rc_view;

	if( _bInit )
	{
		KeyControl_UpdateTrigger();
		_GetMousePointer( hWnd, &cur_x, &cur_y );

		// sRECT ========================================================
		{
			RECT rc; GetClientRect( hWnd, &rc );
			rc_view.l = (float)rc.left  ;
			rc_view.t = (float)rc.top   ;
			rc_view.r = (float)rc.right  / g_dxdraw->get_screen_mag();
			rc_view.b = (float)rc.bottom / g_dxdraw->get_screen_mag();
		}
		g_dxdraw->SetViewport( rc_view.l, rc_view.t, rc_view.r, rc_view.b, 0, 0 );


		// Make Rect ====================================================
		if_PcmTable_SetRect   ( &rc_view );
		if_Panel_Scale_SetRect( &rc_view );
		if_Panel_Logo_SetRect ( &rc_view );
		if_Free_SetRect       (          );

		// Action ========================================================
		if( if_Cursor_Action( cur_x, cur_y ) ) bDraw = true;

		if_PcmTable_SetRect   ( &rc_view );
		if_Panel_Scale_SetRect( &rc_view );
		if_Panel_Logo_SetRect ( &rc_view );
		if_Free_SetRect       (          );

		if( bDraw )
		{
			bool b_tex_reloaded = false;
			g_dxdraw->Begin( &b_tex_reloaded );
			if( b_tex_reloaded )
			{
			}

			if_Free_Put(       );
			if_Panel_Scale_Put();
			if_Panel_Logo_Put( );
			if_PcmTable_Put(  &rc_view );

			if_PcmTable_Put_CursorPosition( cur_x, cur_y );

			g_ScrlPCM_H.Put();

			if_Menu_Zoom_Put();

			// フレーム数
			uint32_t  fps;
			fRECT rcFps = {112, 96,136,104};
			fps = _CountFramePerSecond();
			g_dxdraw->tex_Put_View( rc_view.r - 40 - 4,  2, &rcFps, SURF_PARTS );
			if_gen_num6( rc_view.r - 16 - 4,  2, fps, 2 );

			g_dxdraw->End( true );
		}
	}
}

bool Interface_init( HWND hWnd, const pxFile2* file_profile )
{
	g_ScrlPCM_H.Initialize( g_dxdraw, SURF_PARTS, false ); 

	if_Cursor_Initialize  ( );
	if_PcmTable_Initialize( );
	if_Panel_Scale_init   ( file_profile );

	_bInit = true;

	return true;
}




