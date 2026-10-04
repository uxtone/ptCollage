// '16/03/02 FILE *fp -> pxFile file.

#include <pxFile2.h>
#include <pxStrT.h>
#include <pxDebugLog.h>

#include "./pxwFilePath.h"

static TCHAR*         _dir_name         = _T("window-rect");
static const pxFile2* _ref_file_profile = NULL;

void pxwWindowRect_init( const pxFile2* ref_file_profile )
{
	_ref_file_profile = ref_file_profile;
}

void pxwWindowRect_center( HWND hWnd )
{
	HWND hParent;
	RECT rcParent, rcMe, rcDesk;
	long x, y;

	SystemParametersInfo( SPI_GETWORKAREA, 0, &rcDesk, 0 );
	GetWindowRect( hWnd, &rcMe );

	if( hParent = GetParent( hWnd ) )
	{
		WINDOWPLACEMENT place;
		place.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement( hParent, &place );
		switch( place.showCmd )
		{
		case SW_SHOWMINIMIZED:
		case SW_HIDE         :
			SystemParametersInfo( SPI_GETWORKAREA, 0, &rcParent, 0 );
			break;
		default:
			GetWindowRect( hParent, &rcParent );
		}
	}
	else
	{
		SystemParametersInfo( SPI_GETWORKAREA, 0, &rcParent, 0 );
	}

	x = rcParent.left + ( (rcParent.right  - rcParent.left) - (rcMe.right  - rcMe.left) ) /2;
	y = rcParent.top  + ( (rcParent.bottom - rcParent.top ) - (rcMe.bottom - rcMe.top ) ) /2;

	if( x                          < rcDesk.left   ) x = rcDesk.left;
	if( y                          < rcDesk.top    ) y = rcDesk.top;
	if( x + (rcMe.right-rcMe.left) > rcDesk.right  ) x = rcDesk.right  - (rcMe.right-rcMe.left);
	if( y + (rcMe.bottom-rcMe.top) > rcDesk.bottom ) y = rcDesk.bottom - (rcMe.bottom-rcMe.top);
	
	SetWindowPos( hWnd, NULL, x, y, 0, 0, SWP_NOSIZE|SWP_SHOWWINDOW|SWP_DRAWFRAME );

	PostMessage( hWnd, WM_SIZE, SIZE_RESTORED, 0 );
}

bool pxwWindowRect_load( HWND hWnd, const TCHAR* name, BOOL bSize, bool b_default_center )
{
	bool b_ret   = false;
	RECT rect    = {   };
	RECT rcDesk  = {   };
	LONG showCmd = SW_NORMAL;

	RECT rc_now  = {   };
	int  w;
	int  h;

	pxDescriptor* desc = NULL;
	if( !_ref_file_profile->open_r( &desc, _dir_name, name, NULL ) ) goto term;
	if( !desc->r( &rect   , sizeof(RECT), 1 ) ) goto term;
	if( !desc->r( &showCmd, sizeof(LONG), 1 ) ) goto term;
	desc->clear();

	if( bSize )
	{
		w = rect.right    - rect.left  ;
		h = rect.bottom   - rect.top   ;
	}
	else
	{
		GetClientRect( hWnd, &rc_now );
		w = rc_now.right  - rc_now.left;
		h = rc_now.bottom - rc_now.top ;
	}

	SystemParametersInfo( SPI_GETWORKAREA, 0, &rcDesk, 0 );

	{
		int max_w, max_h, min_w, min_h;
		max_w = GetSystemMetrics( SM_CXMAXIMIZED );
		max_h = GetSystemMetrics( SM_CYMAXIMIZED );
		min_w = GetSystemMetrics( SM_CXMIN       );
		min_h = GetSystemMetrics( SM_CYMIN       );

		if( w > max_w ) w = max_w;
		if( h > max_h ) h = max_h;
	}

	if( rect.left < rcDesk.left       ) rect.left = rcDesk.left      ;
	if( rect.left > rcDesk.right  - w ) rect.left = rcDesk.right  - w;
	if( rect.top  < rcDesk.top        ) rect.top  = rcDesk.top       ;
	if( rect.top  > rcDesk.bottom - h ) rect.top  = rcDesk.bottom - h;

	rect.right  = rect.left + w;
	rect.bottom = rect.top  + h;

	if( bSize ) MoveWindow(   hWnd, rect.left, rect.top, w, h, FALSE );
	else        SetWindowPos( hWnd, NULL, rect.left, rect.top, 0,0, SWP_NOSIZE );

	dlog_c( "show window: start");
	if( showCmd == SW_SHOWMAXIMIZED ) ShowWindow( hWnd, SW_SHOWMAXIMIZED );
	else                              ShowWindow( hWnd, SW_NORMAL        );
	dlog_c( "show window: end");

	b_ret = true;
term:
	SAFE_DELETE( desc );

	if( !b_ret && b_default_center ) pxwWindowRect_center( hWnd );

	return TRUE;
}
 
bool pxwWindowRect_save( HWND hWnd, const TCHAR* name )
{
	WINDOWPLACEMENT place = {   };
	RECT            rc    = {   };
	bool            b_ret = false;
	pxDescriptor*   desc  = NULL ;

	if( !_ref_file_profile->open_w( &desc, _dir_name, name, NULL ) ) goto term;

	if( !GetWindowPlacement( hWnd, &place ) ) return false;

	if( place.showCmd == SW_NORMAL )
	{
		if( !GetWindowRect( hWnd, &rc ) ) return false;
		place.rcNormalPosition = rc;
	}
    static int32_t w;
    w = place.rcNormalPosition.right - place.rcNormalPosition.left;
    static int32_t h;
    h = place.rcNormalPosition.bottom - place.rcNormalPosition.top;

	if( !desc->w_asfile( &place.rcNormalPosition, sizeof(RECT), 1 ) ) return false;
	if( !desc->w_asfile( &place.showCmd,          sizeof(LONG), 1 ) ) return false;

	b_ret = true;
term:

	SAFE_DELETE( desc );

	return b_ret;
}
