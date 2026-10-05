#include <uxStr.h>
#include <commctrl.h>
#include <pxStdDef.h>
#include "pxwRuntime.h"

bool pxwRuntime::init( const uxDS& app_name, int* p_exit_code )
{
	// we magnify by an integer factor ourselves (pxwDx09Draw_system_mag);
	// without this the system bitmap-stretches the whole window instead.
	SetProcessDPIAware();

	InitCommonControls();

	if( FAILED( CoInitializeEx( NULL, COINIT_MULTITHREADED ) ) ){ *p_exit_code = 0; return false; }
	_com = true;

	// for xaudio2_7.dll_unloaded bug.
	_xa2_keep = new pxwXAudio2Keep_loadlib();
#ifdef _DEBUG
	const bool b_debug = true;
#else
	const bool b_debug = false;
#endif
	if( !_xa2_keep->invoke( b_debug ) )
	{
		MessageBox( NULL, uxT( "keep XAudio2 Error" ), uxT( app_name ), MB_OK | MB_ICONERROR );
		*p_exit_code = -1;
		return false;
	}
	return true;
}

pxwRuntime::~pxwRuntime()
{
	SAFE_DELETE( _xa2_keep );
	if( _com ) CoUninitialize();
}
