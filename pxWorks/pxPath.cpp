
#ifndef pxSTDAFX_H
#include <string.h> // strlen
#endif
#include <uxStr.h>

#include "./pxShiftJIS.h"
#include "./pxUTF8.h"

#include "./pxPath.h"

// uxDS paths are UTF-8, so that is the default: path helpers are used before any app calls setMode.
static pxPathMode _mode = pxPathMode_UTF8;

void  pxPath_setMode( pxPathMode mode   )
{
	// uxDS paths are UTF-8 on every platform, so "auto" is UTF-8.
	_mode = ( mode == pxPathMode_auto ) ? pxPathMode_UTF8 : mode;
}

static const char* _find_last_c( const char* path, char c )
{
	const char* p_last = NULL;
	if( !path ) return NULL;

	switch( _mode )
	{
	case pxPathMode_UTF8:
	{
		const char* p     = path;
		int32_t     bytes =    0;
		int32_t     size  =    0;
		uint32_t    code  =    0;

		if( !pxUTF8_check_size( p, &size, false ) ) return NULL;

		for( int s = 0; s < size; s += bytes, p += bytes )
		{
			if( !pxUTF8_get_top_code( &code, p, &bytes ) ) return NULL;
			if( code == (uint32_t)c ) p_last = p;
		}
	}
	break;
	case pxPathMode_ShiftJIS:
	{
		const char* p     = path;
		int32_t     bytes =    0;
		int32_t     size  =    0;
		uint32_t    code  =    0;

		if( !pxShiftJIS_check_size( p, &size, false ) ) return NULL;

		for( int s = 0; s < size; s += bytes, p += bytes )
		{
			if( !pxShiftJIS_get_top_code( &code, p, &bytes ) ) return NULL;
			if( code == (uint32_t)c ) p_last = p;
		}
	}
	break;

	default: return NULL;
	}

	return p_last;
}

const char* pxPath_find_ext( const uxDS& path )
{
	const char* p = _find_last_c( path, '.' );
	return p ? p + 1 : NULL;
}

const char* pxPath_find_filename( const uxDS& path )
{
	const char* p = _find_last_c( path, '\\' );
	if( !p ) p = _find_last_c( path, '/' );
	return p ? p + 1 : NULL;
}

bool pxPath_remove_filename( uxDS& path )
{
	const char* p = pxPath_find_filename( path );
	if( !p ) return false;
	path.truncate( (size_t)( p - 1 - *path ) );
	return true;
}

bool pxPath_remove_filename( char* path )
{
	if( !path ) return false;
	uxDS        tmp( path );
	const char* p = pxPath_find_filename( tmp );
	if( !p ) return false;
	path[ p - *tmp - 1 ] = '\0'; // the separator goes too, like the uxDS version
	return true;
}

bool pxPath_remove_ext( char* path )
{
	if( !path ) return false;
	uxDS        tmp( path );
	const char* e = pxPath_find_ext( tmp );
	if( !e ) return false;
	const char* f = pxPath_find_filename( tmp );
	if( f && e <= f ) return false; // a dot in a directory name is not an extension
	path[ e - *tmp - 1 ] = '\0'; // the dot goes too
	return true;
}

bool pxPath_remove_ext( uxDS& path )
{
	const char* e = pxPath_find_ext( path );
	if( !e ) return false;
	const char* f = pxPath_find_filename( path );
	if( f && e <= f ) return false;
	path.truncate( (size_t)( e - *path - 1 ) );
	return true;
}
