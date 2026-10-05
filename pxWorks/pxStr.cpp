
#include "pxError.h"

#ifndef pxSTDAFX_H
#include <stdlib.h>
#include <string.h>
#endif

#include "./pxStr.h"


bool pxStr_sjis_is_2byte( char c )
{
	unsigned char u = (unsigned char)c;
	if( u >= 0x81 && u <= 0x9F ) return true;
	if( u >= 0xE0 && u <= 0xFC ) return true;
	return false;
}

bool pxStr_copy_without_ext( char* p_dst, const char* p_src, int buf_size )
{
	char*       pd = p_dst;
	const char* ps = p_src;

	for( int i = 0 ;i < buf_size; i++, ps++, pd++ )
	{
		if( *ps == '.' || *ps == '\0' ){ *pd = '\0'; if( !strlen( p_dst ) ) return false; return true; }
		*pd = *ps;
	}
	return false;
}


void pxStr_free( char **pp )
{
	if( *pp ){ free( *pp ); *pp = NULL; }
}


void pxStr_sjis_RemoveLastBlank( char *buf )
{
	for( int a = strlen( buf ) - 1; a >= 0; a-- )
	{
		if( a > 0 && pxStr_sjis_is_2byte( buf[ a - 1 ] ) ) break;
		if( buf[ a ] == '\r' || buf[ a ] == '\n' ) buf[ a ] = '\0';
	}
}

int pxStr_sjis_CopyWithLength( char *p_dst, const char *p_src, int len )
{
	int i = 0;

	for( ; i < len-1; i++ )
	{
		char a = p_src[ i ];
		if( !a ) break;
		if( pxStr_sjis_is_2byte( a ) )
		{
			if( i < len-2 ){ p_dst[ i ] = p_src[ i ]; i++;  p_dst[ i ] = p_src[ i ]; }
			else break;
		}
		else
		{
			p_dst[ i ] = p_src[ i ];
		}
	}
	p_dst[ i ] = '\0';
	return i;
}


int pxStr_csv_ReadAlloc( const char *p_src, char **pp_dst, int size_dst, unsigned char sep )
{
	const char *p  = p_src;
	int        len =     0;

	while( len < size_dst )
	{
		if( pxStr_sjis_is_2byte( *p ) ){ len++; p++; }
		else if( !( *p ) || *p == sep || *p == 0x0d || *p == 0x0a ) break;
		len++; p++;
	}
	if( len >= size_dst || !len ) return 0;
	if( !(  *pp_dst = (char*)malloc( len + 1 ) ) ) return 0;
	memcpy( *pp_dst, p_src, len );
	*( *pp_dst + len ) = 0;

	return len;
}

int pxStr_csv_Read( const char *p_src, char *p_dst, int size_dst, unsigned char sep )
{
	const char *p  = p_src;
	int        len =     0;

	while( len < size_dst )
	{
		if( pxStr_sjis_is_2byte( *p ) ){ len++; p++; }
		else if( !( *p ) || *p == sep || *p == 0x0d || *p == 0x0a ) break;
		len++; p++;
	}
	if( len >= size_dst || !len ) return 0;
	memcpy( p_dst, p_src, len );
	*( p_dst + len ) = 0;
	return len;
}

bool pxStr_copy_allocate( char** p_dst, const char *str )
{
	if( !str ) {
		pxerr( "** pxSTR-z **" ); return false;
	}
	int  buf_size = strlen( str ) + 1;
	if( !(*p_dst = (char*)malloc( buf_size ) ) ){
		pxerr( "** pxSTR **" ); return false;
	}
	if( buf_size > 1 ) memcpy( *p_dst, str, buf_size );
	else               *p_dst = 0;
	return true;
}

bool pxStr_is_different( const char* a, const char* b )
{
	if(  a && !b ) return true ;
	if( !a &&  b ) return true ;
	if( a )
	{
		if( a == b         ) return false;
		if( strcmp( a, b ) ) return true ;
	}
	return false;
}
