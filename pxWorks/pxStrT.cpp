
#include <pxStdDef.h>

#include "./pxError.h"

#include "./pxStrT.h"

bool pxStrT_copy_allocate( TCHAR** pp_dst, const TCHAR *str )
{
	return pxStrT_copy_allocate( pp_dst, str, 0 );
}

bool pxStrT_copy_allocate( TCHAR** pp_dst, const TCHAR *str, int32_t extra )
{
#ifdef UNICODE
	int len = _tcslen( str );
#else
	int len = strlen ( str );
#endif

	*pp_dst = (TCHAR*)malloc( (len + 1 + extra )* sizeof(TCHAR) );
	if( !(*pp_dst) ){ pxerr( "strt-alc" ); return false; }
	memset( *pp_dst,   0,     (len + 1 + extra )* sizeof(TCHAR) );
	memcpy( *pp_dst, str,     (len + 1         )* sizeof(TCHAR) );
	return true;
}

bool    pxStrT_copy         ( TCHAR* dst, const TCHAR* src )
{
#ifdef UNICODE
	int len = _tcslen( src );
#else
	int len = strlen ( src );
#endif
	// was "len * sizeof(TCHAR) + 1", which is only half of a terminator. it is a whole TCHAR.
	memcpy( dst, src, (len + 1) * sizeof(TCHAR) );
	return true;
}


void pxStrT_free( TCHAR **pp )
{
	if( *pp ){ free( *pp ); *pp = NULL; }
}

int32_t pxStrT_size( const TCHAR* str )
{
	if( !str ) return 0;
#ifdef UNICODE
	int len = _tcslen( str );
#else
	int len = strlen ( str );
#endif

	return len * sizeof(TCHAR);
}

bool pxStrT_compare( const TCHAR *str1, const TCHAR *str2, int32_t num, int32_t* p_res )
{
	if( !str1 || !str2 || !p_res ) return false;

#ifdef UNICODE
	int len1 = _tcslen( str1 );
	int len2 = _tcslen( str2 ); // was str1: num was never clamped to the second string.
#else
	int len1 = strlen ( str1 );
	int len2 = strlen ( str2 );
#endif

	if( num )
	{
		if( num > len1 ) num = len1;
		if( num > len2 ) num = len2;
		*p_res = memcmp( str1, str2, num * sizeof(TCHAR) );
	}
	else
	{
#ifdef UNICODE
		*p_res = _tcscmp( str1, str2 );
#else
		*p_res = strcmp ( str1, str2 );
#endif
	}
	return true;
}

bool pxStrT_is_different( const TCHAR* a, const TCHAR* b )
{
	if(  a && !b ) return true ;
	if( !a &&  b ) return true ;
	if( a )
	{
		if( a == b         ) return false;
#ifdef UNICODE
		if( _tcscmp( a, b ) ) return true ;
#else
		if(  strcmp( a, b ) ) return true ;
#endif
	}
	return false;
}
