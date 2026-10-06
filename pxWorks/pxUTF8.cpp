
#define _CRT_SECURE_NO_WARNINGS

#include "./pxMem.h"



#include "./pxUTF8.h"

typedef struct
{
	uint8_t bytes[ 4 ][ 2 ];
	int32_t byte_num  ;
}
pxUTF8CHECK;

static const pxUTF8CHECK _utf8_checks[] =
{
	// 1byte
	{ 0x01,0x7F,  0x00,0x00,  0x00,0x00,  0x00,0x00, 1 }, // ASCII

	// 2byte
	{ 0xC2,0xDF,  0x80,0xBF,  0x00,0x00,  0x00,0x00, 2 }, // Europe, Arabic, Hebrew

	// 3byte
	{ 0xE0,0xE0,  0xA0,0xBF,  0x80,0xBF,  0x00,0x00, 3 }, // Indic, Thai, symbol, Chinese, Japanese, Korean
	{ 0xE1,0xEC,  0x80,0xBF,  0x80,0xBF,  0x00,0x00, 3 },
	{ 0xED,0xED,  0x80,0x9F,  0x80,0xBF,  0x00,0x00, 3 },
	{ 0xEE,0xEE,  0x80,0xBF,  0x80,0xBF,  0x00,0x00, 3 }, // private use area
	{ 0xEF,0xEF,  0xA4,0xBF,  0x80,0xBF,  0x00,0x00, 3 },
	{ 0xEF,0xEF,  0x80,0xA3,  0x80,0xBF,  0x00,0x00, 3 }, // private use area

	// 4byte
	{ 0xF0,0xF0,  0x90,0xBF,  0x80,0xBF,  0x80,0xBF, 4 },
	{ 0xF1,0xF2,  0x80,0xBF,  0x80,0xBF,  0x80,0xBF, 4 },
	{ 0xF3,0xF3,  0x80,0xAF,  0x80,0xBF,  0x80,0xBF, 4 },
	{ 0xF3,0xF3,  0xB0,0xBF,  0x80,0xBF,  0x80,0xBF, 4 }, // private use area
	{ 0xF4,0xF4,  0x80,0x8F,  0x80,0xBF,  0x80,0xBF, 4 }, // private use area
};

bool pxUTF8_get_top_code( uint32_t* p_code, const char* str_src, int32_t* p_byte )
{
	if( !p_byte || !str_src ) return false;

	const uint8_t* p       = (uint8_t*)str_src;
	uint32_t       code    = 0;
	int32_t        byte    = 0;
	int32_t        chk_num = sizeof(_utf8_checks) / sizeof(_utf8_checks[0]);

	if( !(*p) )
	{
		if( p_code ) *p_code = 0;
		if( p_byte ) *p_byte = 1;
		return true;
	}

	const pxUTF8CHECK* p_chk = _utf8_checks;

	for( int ch = 0; ch < chk_num && !byte; ch++, p_chk++ )
	{
		p = (uint8_t*)str_src;
		for( int i = 0; i < p_chk->byte_num && !byte; i++, p++ )
		{
			if( *p < p_chk->bytes[ i ][ 0 ] || *p > p_chk->bytes[ i ][ 1 ] ) break;
			if( i == p_chk->byte_num - 1 ) byte = p_chk->byte_num;
		}
	}
	if( !byte ) return false;

	p = (uint8_t*)str_src;

	for( int i = 0; i < byte; i++, p++ ) code = ( code << 8 ) + *p;

	if( p_code ) *p_code = code;
	if( p_byte ) *p_byte = byte;

	return true;
}

bool pxUTF8_check_code_byte( const uint32_t code, int32_t* p_byte )
{
	const uint8_t*     p       = NULL;
	int32_t            chk_num = sizeof(_utf8_checks) / sizeof(_utf8_checks[0]);
	int32_t            byte    =    0;
	const pxUTF8CHECK* p_chk = _utf8_checks;

	int32_t r = 0;

	if     ( code & 0xff000000 ) r = 3;
	else if( code & 0x00ff0000 ) r = 2;
	else if( code & 0x0000ff00 ) r = 1;
	else if( code & 0x000000ff ) r = 0;
	else
	{
		if( p_byte ) *p_byte = 1;
		return true;
	}

	for( int ch = 0; ch < chk_num && !byte; ch++, p_chk++ )
	{
		p = ((uint8_t*)&code) + r;
		for( int i = 0; i <= r; i++, p-- )
		{
			if( *p < p_chk->bytes[ i ][ 0 ] || *p > p_chk->bytes[ i ][ 1 ] ) break;
			if( i == r ) byte = p_chk->byte_num;
		}
	}
	if( !byte  ) return false;
	if( p_byte ) *p_byte = byte;
	return true;
}

bool pxUTF8_append_code( char* str_dst, uint32_t code )
{
	if( !str_dst ) return false;

	int32_t byte =       0;
	char*   pd   = str_dst;
	char*   ps   = NULL   ;

	if( !pxUTF8_check_code_byte( code, &byte ) ) return false;

	ps = ((char*)&code) + (byte-1);
	for( int i = 0; i < byte; i++, pd++, ps-- ) *pd = *ps;
	return true;
}

bool pxUTF8_check_size( const char* str_src, int32_t* p_size , bool b_permit_illegal )
{
	if( !str_src ) return false;

	const char* p     = str_src;
	int32_t     byte  =       0;
	uint32_t    code  =       0;
	int32_t     count =       0;

	for( int s = 0; true; s += byte, p += byte )
	{
		if( !pxUTF8_get_top_code( &code, p, &byte ) )
		{
			if( b_permit_illegal ){ *p_size = count; return true ; }
			else                                     return false;
		}
		if( !code ) break;
		count += byte;
	}
	*p_size = count;
	return true;
}

bool pxUTF8_count_codes( const char* str_src, int32_t* p_count, int32_t data_size )
{
	if( !str_src ) return false;

	const char* p     = str_src;
	int32_t     byte  =       0;
	uint32_t    code  =       0;
	int32_t     count =       0;

	for( int s = 0; s < data_size; s += byte, p += byte )
	{
		if( !pxUTF8_get_top_code( &code, p, &byte ) ) return false;
		if( !code ) break;
		count++;
	}
	*p_count = count;
	return true;
}

bool pxUTF8_compare( int32_t* p_res, const char* str1, const char* str2 )
{
	if( !str1 || !str2 ) return false;

	int32_t     size1    = 0;
	int32_t     size2    = 0;
	int32_t     size_max = 0;
	uint32_t    code1    = 0;
	uint32_t    code2    = 0;
	int32_t     byte1    = 0;
	int32_t     byte2    = 0;
	const char* p1       = str1;
	const char* p2       = str2;

	if( !pxUTF8_check_size( str1, &size1, false ) ) return false;
	if( !pxUTF8_check_size( str2, &size2, false ) ) return false;
	size_max = size1; if( size2 > size1 ) size_max = size2;

	for( int s = 0; s < size_max; s += byte1, p1 += byte1, p2 += byte2 )
	{
		if( !pxUTF8_get_top_code( &code1, p1, &byte1 ) ) return false;
		if( !pxUTF8_get_top_code( &code2, p2, &byte2 ) ) return false;
		if( !byte1 && !byte2 ){ *p_res =  0; return true; }
		if( code1 < code2    ){ *p_res =  1; return true; }
		if( code1 > code2    ){ *p_res = -1; return true; }
	}
	*p_res = 0;
	return true;
}


bool pxUTF8_free( char** pp )
{
	return pxMem_free( (void**)pp );
}

bool pxUTF8_copy_allocate( char** pp, const char* str_src )
{
	if( !str_src ) return false;
	char*   p    = NULL;
	int32_t size = 0;

	if( !pxUTF8_check_size( str_src, &size, false ) ) return false;
	if( !pxMem_zero_alloc( (void**)&p, size + 1 ) ) return false;
	memcpy( p, str_src, size );
	*pp = p;
	return true;
}


// test ----------------------------------------------

#include "./pxDescriptor.h"

bool pxUTF8_test_reverse   ( const uxDS& path_src, const uxDS& path_dst )
{
	bool      b_ret     = false;
	int32_t   file_size =     0;
	int32_t   buf_size  =     0;
	int32_t   dst_size  =     0;
	char*     p_src     = NULL ;
	char*     p_dst     = NULL ;
	uint32_t* codes     = NULL ;
	int32_t   code_num  =     0;

	// read.
	{
		FILE*        fp = NULL; if( !( fp = ux_fopen( path_src, "rb" ) ) ) return false;
		pxDescriptor desc     ; desc.set_file_r( fp );

		if( !desc.get_size_bytes( &file_size ) ) return false;
		buf_size = file_size + 1;
		if( !pxMem_zero_alloc( (void**)&p_src, buf_size  ) ) goto term;
		if( !pxMem_zero_alloc( (void**)&p_dst, buf_size  ) ) goto term;
		if( !desc.r( p_src, 1,                 file_size ) ) goto term;
	}

	// count.
	if( !pxUTF8_count_codes( p_src, &code_num, buf_size ) || code_num == 0 ) goto term;

	// allocate.
	if( !pxMem_zero_alloc( (void**)&codes, sizeof(int32_t) * code_num ) ) goto term;

	// get..
	{
		char*   p    = p_src;
		int32_t byte =     0;
		int32_t code =     0;

		for( int c = 0; c < code_num; c++, p += byte )
		{
			if( !pxUTF8_get_top_code( &codes[ c ], p, &byte ) ) goto term;
			if( !codes[ c ] ) break;
		}
	}

	// set (reverse)
	{
		char*           pd   = p_dst;
		const uint32_t* ps   = &codes[ code_num - 1 ];
		int32_t         byte =     0;

		for( int c = 0; c < code_num; c++, ps--, pd += byte )
		{
			if( !pxUTF8_check_code_byte(      *ps, &byte ) ) goto term;
			if( !pxUTF8_append_code    (  pd, *ps        ) ) goto term;
			dst_size += byte;
		}
	}

	// write.
	{
		FILE*        fp = NULL; if( !( fp = ux_fopen( path_src, "wb" ) ) ) return false;
		pxDescriptor desc     ; desc.set_file_w( fp );
		if( !desc.w_asfile ( p_dst, 1, dst_size ) ) goto term;
	}

	b_ret = true;
term:

	pxMem_free( (void**)&p_src );
	pxMem_free( (void**)&p_dst );
	pxMem_free( (void**)&codes );
	return b_ret;
}
