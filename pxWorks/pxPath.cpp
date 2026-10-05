
#ifndef pxSTDAFX_H
#include <string.h> // strlen
#endif

#include "./pxShiftJIS.h"
#include "./pxUTF8.h"

#include "./pxPath.h"

static pxPathMode _mode = pxPathMode_unknown;

void  pxPath_setMode( pxPathMode mode   )
{
	if( mode == pxPathMode_auto )
	{
#ifdef _WIN32


#ifdef UNICODE
		_mode = pxPathMode_UTF16LE ;
#else
		_mode = pxPathMode_ShiftJIS;
#endif


#else
		_mode = pxPathMode_UTF8    ;
#endif

	}
	else
	{
		_mode = mode;
	}
}

static TCHAR *_find_last_c( const TCHAR *path, TCHAR c )
{
	TCHAR* p_last_period = NULL;

	switch( _mode )
	{
	case pxPathMode_UTF8:
	{
		const char* p     = (const char*)path;
		int32_t     bytes =    0;
		int32_t     size  =    0;

		uint32_t    code  =    0;

		if( !pxUTF8_check_size( p, &size, false ) ) return NULL;

		for( int s = 0; s < size; s += bytes, p+= bytes )
		{
			if( !pxUTF8_get_top_code( &code, p, &bytes ) ) return NULL;
			if( code == c ) p_last_period = (TCHAR*)p;
		}
	}
	break;
	case pxPathMode_ShiftJIS:
	{
		const char* p     = (const char*)path;
		int32_t     bytes =    0;
		int32_t     size  =    0;
		uint32_t    code  =    0;

		if( !pxShiftJIS_check_size( p, &size, false ) ) return NULL;

		for( int s = 0; s < size; s += bytes, p+= bytes )
		{
			if( !pxShiftJIS_get_top_code( &code, p, &bytes ) ) return NULL;
			if( code == c ) p_last_period = (TCHAR*)p;
		}
	}
	break;

#ifdef UNICODE
	case pxPathMode_UTF16LE:
	{
		const wchar_t* p   = (const wchar_t*)path;
		int32_t        len = (int32_t)_tcslen( path );
		for( int i = 0; i < len; i++, p++ )
		{
			if( *p == c ) p_last_period = (TCHAR*)p;
		}
	}
	break;
#endif

	default: return NULL;
	}

	return p_last_period;
}

const TCHAR *pxPath_find_ext( const TCHAR *path )
{
	TCHAR* p = _find_last_c( path, '.' );
	if( !p ) return NULL;
	return p + 1;
}

const TCHAR *pxPath_find_filename( const TCHAR *path )
{
	TCHAR* p = _find_last_c( path, '\\' );
	if( !p )
	{
		p = _find_last_c( path, '/' );
		if( !p ) return NULL;
	}
	return p + 1;
}

const TCHAR *pxPath_remove_filename(       TCHAR *path )
{
	const TCHAR *p = pxPath_find_filename( path );
	if( !p ) return NULL;
	p--;
	*((TCHAR*)p) = '\0';
	return path;
}
