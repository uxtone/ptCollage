
#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../../Generic/KeyControl.h"

#include "../Tune.h"

#include "SurfaceNo.h"
#include "Interface.h"
#include "if_Generic.h"
#include "if_Cursor.h"
#include "if_Player.h"

#define _FLIPWAIT 20

static bool              _b_init     = false;

static CRITICAL_SECTION  _critical;
static bool              _b_critical = false;

static void _GetMousePointer( HWND hWnd, float *px, float *py )
{
	POINT pt;
	GetCursorPos  (       &pt );
	ScreenToClient( hWnd, &pt );	
	*px = (float)pt.x / g_dxdraw->get_screen_mag();
	*py = (float)pt.y / g_dxdraw->get_screen_mag();
}

static int32_t _CountFramePerSecond()
{
	static bool      bFirst    = true;
	static uint32_t  wait      =    0;
	static uint32_t  count     =    0;
	static int32_t   max_count =    0;

	if( bFirst ){ wait = GetTickCount(); bFirst = false; }

	count++;

	if( wait + 1000 <= GetTickCount() )
	{
		wait     +=  1000;
		max_count = count;
		count     =     0;
	}
	return max_count;
}

void Interface_Process( HWND hWnd, bool bDraw )
{
	float cur_x  ;
	float cur_y  ;
	fRECT rc_view;
	PTP_SIGN ptp_sign = PTP_SIGN_None;


	if( !_b_init ) return;

	if( !TryEnterCriticalSection( &_critical ) ) return; if( !_b_critical ) return;

	KeyControl_UpdateTrigger();
	_GetMousePointer( hWnd, &cur_x, &cur_y );

	// fRECT ========================================================
	{
		RECT rc; GetClientRect( hWnd, &rc );
		rc_view.l = (float)rc.left;
		rc_view.t = (float)rc.top;
		rc_view.r = (float)rc.right / g_dxdraw->get_screen_mag();
		rc_view.b = (float)rc.bottom / g_dxdraw->get_screen_mag();
	}
	g_dxdraw->SetViewport( rc_view.l, rc_view.t, rc_view.r, rc_view.b, 0, 0 );
 
	// Make Rect ====================================================
	if_Player_SetRect( &rc_view );

	// Action ========================================================

	if( if_Cursor_Action( cur_x, cur_y, &ptp_sign ) ) bDraw = true;

	if( bDraw ){  }

	// 表示 ==========================================================
	if( bDraw )
	{
		bool b_tex_reloaded = false;
		g_dxdraw->Begin( &b_tex_reloaded );
		if( b_tex_reloaded )
		{
			if_Player_RedrawName( NULL );
		}

		if_Player_SetRect( &rc_view );
		if_Player_Put    ();
		if_gen_num6( rc_view.r - 16 - 4,  4, _CountFramePerSecond(), 2 );

		g_dxdraw->End( true );
	}

	LeaveCriticalSection( &_critical );

	switch( ptp_sign )
	{
	case PTP_SIGN_SelectFile:
		{
			Tune_SelectAndPlay( hWnd );
		}
		break;
	}
}

bool Interface_init( HWND hWnd, const pxPalette* palette, const pxFile2* file_profile )
{
	if_Cursor_Initialize();
	if_Player_Initialize( hWnd, palette );
	InitializeCriticalSection( &_critical ); _b_critical = true;
	_b_init     = true;
	return true;
}

void Interface_Release()
{
	_b_init = false;

	if( _b_critical )
	{
		EnterCriticalSection ( &_critical );
		_b_critical = false;
		DeleteCriticalSection( &_critical );
	}
}



