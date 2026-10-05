#pragma once

#include <windows.h>
#include <shellapi.h>   // CommandLineToArgvW

// Defines the program entry point with a TCHAR-correct command line
//
//   pxwENTRY_POINT( hInst, hPrevInst, lpCmd, nCmd )
//   {
//       ...   // lpCmd is LPTSTR: wide in UNICODE builds, narrow otherwise
//   }

// with UNICODE, wWinMain is the real entry point. A WinMain wrapper is also
// defined for toolchains linked without `-municode` which forwards the wide
// command line, matching lpCmdLine's typical contents

// without UNICODE, WinMain is the entry point directly.

#ifdef UNICODE

// Returns the wide command line without the program name.
static inline LPWSTR pxwEntryPoint_wide_args( void )
{
	WCHAR* p    = GetCommandLineW();
	int    argc = 0;
	LPWSTR* argv = CommandLineToArgvW( p, &argc );
	if( argv ) LocalFree( argv );

	if( argc <= 1 ) // avoids GetCommandLineW bug that does not always quote the program name if no arguments
	{
		while( *p ) ++p;
		return p;
	}

	BOOL quoted = ( p[ 0 ] == L'"' );
	++p; // skips the " or the first letter (all paths are at least 1 letter)
	while( *p )
	{
		if     (  quoted && *p == L'"' ) { quoted = FALSE; } // found end quote
		else if( !quoted && *p == L' ' )
		{
			// found an unquoted space, now skip all spaces
			do { ++p; } while( *p == L' ' );
			break;
		}
		++p;
	}
	return p;
}

#define pxwENTRY_POINT( h_inst, h_prev, cmd, n_show )                                  \
		int WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int );                          \
		int WINAPI WinMain( HINSTANCE px_h, HINSTANCE px_p, LPSTR, int px_n )              \
		{ return wWinMain( px_h, px_p, pxwEntryPoint_wide_args(), px_n ); }               \
		int WINAPI wWinMain( HINSTANCE h_inst, HINSTANCE h_prev, LPWSTR cmd, int n_show )

#else

#define pxwENTRY_POINT( h_inst, h_prev, cmd, n_show )                                  \
		int WINAPI WinMain( HINSTANCE h_inst, HINSTANCE h_prev, LPSTR cmd, int n_show )

#endif
