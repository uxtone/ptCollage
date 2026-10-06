
#include <pxStdDef.h>



#include "./pxStrT.h"

bool pxStrT_copy_allocate( uxDS& dst, const uxDS& src )
{
	return pxStrT_copy( dst, src );
}

bool pxStrT_copy_allocate( uxDS& dst, const uxDS& src, int32_t extra )
{
	(void)extra;
	return pxStrT_copy( dst, src );
}

bool pxStrT_copy( uxDS& dst, const uxDS& src )
{
	dst = src; // deep copy; an unset source leaves dst unset
	return true;
}

void pxStrT_free( uxDS& s )
{
	s = uxDS();
}

int32_t pxStrT_size( const uxDS& str )
{
	return (int32_t)str.size();
}

bool pxStrT_compare( const uxDS& str1, const uxDS& str2, int32_t num, int32_t* p_res )
{
	if( !str1 || !str2 || !p_res ) return false;

	if( num )
	{
		int32_t len1 = (int32_t)str1.size();
		int32_t len2 = (int32_t)str2.size(); // was str1: num was never clamped to the second string.
		if( num > len1 ) num = len1;
		if( num > len2 ) num = len2;
		*p_res = memcmp( *str1, *str2, num );
	}
	else
	{
		*p_res = str1.cmp( str2 ); // sdscmp
	}
	return true;
}

bool pxStrT_is_different( const uxDS& a, const uxDS& b )
{
	if(  a && !b ) return true ;
	if( !a &&  b ) return true ;
	if( a ) return a.cmp( b ) != 0;
	return false;
}
