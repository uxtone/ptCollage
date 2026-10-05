
#include "./pxMem.h"

#include "./pxShiftJIS.h"

static bool _is_shift_jis_2byte( char c )
{
	unsigned char u = (unsigned char)c;
	if( u >= 0x81 && u <= 0x9F ) return true;
	if( u >= 0xE0 && u <= 0xFC ) return true;
	return false;
}


bool    pxShiftJIS_check_code_byte( uint32_t code,    int32_t* p_byte )
{
	if( !p_byte ) return false;

	if( code & 0xffff0000 ) return false;
	if( code & 0x0000ff00 )
	{
		if( !_is_shift_jis_2byte( (char)( (code>>8) & 0xff ) ) ) return false;
		*p_byte = 2;
	}
	else
	{
		*p_byte = 1;
	}
	return true;
}

bool    pxShiftJIS_append_code    ( char* str_dst, uint32_t code )
{
	if( !str_dst ) return false;

	int32_t byte =       0;
	char*   pd   = str_dst;
	char*   ps   = NULL   ;

	if( !pxShiftJIS_check_code_byte( code, &byte ) ) return false;

	ps = ((char*)&code) + (byte-1);
	for( int i = 0; i < byte; i++, pd++, ps-- ) *pd = *ps;
	return true;
}


bool pxShiftJIS_get_top_code( uint32_t* p_code, const char* str_src, int32_t* p_byte )
{
	if( !str_src ) return false;

	int32_t  bytes = 0;
	uint32_t code  = 0;

	if( _is_shift_jis_2byte( *str_src ) ) bytes = 2;
	else                                  bytes = 1;

	const uint8_t* p = (uint8_t*)str_src;

	for( int i = 0; i < bytes; i++, p++ ) code = ( code << 8 ) + *p;

	if( p_code ) *p_code = code ;
	if( p_byte ) *p_byte = bytes;
	return true;
}

bool pxShiftJIS_check_size( const char* str_src, int32_t* p_size, bool b_permit_illegal )
{
	if( !str_src ) return false;

	const char* p     = str_src;
	int32_t     byte  =       0;
	uint32_t    code  =       0;
	int32_t     count =       0;

	for( int s = 0; true; s += byte, p += byte )
	{
		if( !pxShiftJIS_get_top_code( &code, p, &byte ) )
		{
			if( b_permit_illegal ){ *p_size = count; return true; }
			else                   return false;
		}
		if( !code ) break;
		count += byte;
	}
	*p_size = count;
	return true;
}


bool pxShiftJIS_free( char** pp )
{
	return pxMem_free( (void**)pp );
}

bool pxShiftJIS_copy_allocate( char** pp, const char* str_src )
{
	if( !str_src ) return false;
	char*   p    = NULL;
	int32_t size = 0;

	if( !pxShiftJIS_check_size( str_src, &size, false ) ) return false;
	if( !pxMem_zero_alloc( (void**)&p, size + 1 ) ) return false;
	memcpy( p, str_src, size );
	*pp = p;
	return true;
}
