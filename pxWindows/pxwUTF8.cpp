
#include <pxMem.h>

// "sjis" here is the pxtone file encoding. UNICODE builds only pass real Shift-JIS
// through these, so don't depend on the system code page. ANSI builds also pass their
// own TCHAR text through, which is in the system code page.
#ifdef UNICODE
#define _CP_SJIS 932
#else
#define _CP_SJIS CP_ACP
#endif

bool pxwUTF8_sjis_to_utf8( const char* p_src, char** pp_dst, int32_t* p_dst_size )
{
	bool     b_ret     = false;
	wchar_t* p_wide    = NULL ;
	char*    p_utf8    = NULL ;
	int      num_wide  =     0;
	int      size_utf8 =     0;

	if( !p_src     ) return false;
	if( p_dst_size ) *p_dst_size = 0;
	*pp_dst = NULL;

	// to UTF-16
	if( !(num_wide = ::MultiByteToWideChar( _CP_SJIS, 0, p_src, -1, NULL, 0 ) )                ) goto term; 
	if( !pxMem_zero_alloc( (void**)&p_wide, num_wide * sizeof(wchar_t) )                     ) goto term;
	if( !MultiByteToWideChar( _CP_SJIS, 0, p_src, -1, p_wide, num_wide )                       ) goto term;

	// to UTF-8
 	if( !(size_utf8 = ::WideCharToMultiByte( CP_UTF8, 0, p_wide, -1, NULL, 0, NULL, NULL ) ) ) goto term;
	if( !pxMem_zero_alloc( (void**)&p_utf8, size_utf8 )                                      ) goto term;
	if( !WideCharToMultiByte( CP_UTF8, 0, p_wide, -1, p_utf8, size_utf8, NULL, NULL )        ) goto term;

	if( p_dst_size ) *p_dst_size = size_utf8 - 1; // remove last ' '
	*pp_dst = p_utf8;

	b_ret = true;
term:
	pxMem_free( (void**)&p_wide );
	if( !b_ret ) pxMem_free( (void**)&p_utf8 );

	return b_ret;
}

bool pxwUTF8_sjis_to_wide( const char*    p_src, wchar_t** pp_dst, int32_t* p_dst_num  )
{
	bool     b_ret     = false;
	wchar_t* p_wide    = NULL ;
	int      num_wide  =     0;

	if( !p_src    ) return false;
	if( p_dst_num ) *p_dst_num = 0;
	*pp_dst = NULL;

	// to UTF-16
	if( !(num_wide = ::MultiByteToWideChar( _CP_SJIS, 0, p_src, -1, NULL, 0 ) ) ) goto term; 
	if( !pxMem_zero_alloc( (void**)&p_wide, num_wide * sizeof(wchar_t) )      ) goto term;
	if( !MultiByteToWideChar( _CP_SJIS, 0, p_src, -1, p_wide, num_wide )        ) goto term;

	if( p_dst_num ) *p_dst_num = num_wide - 1; // remove last ' '
	*pp_dst = p_wide;

	b_ret = true;
term:
	if( !b_ret ) pxMem_free( (void**)&p_wide );

	return b_ret;
}

bool pxwUTF8_utf8_to_wide( const char* p_src, wchar_t** pp_dst, int32_t* p_dst_num )
{
	bool     b_ret     = false;
	wchar_t* p_wide    = NULL ;
	int      num_wide  =     0;

	if( !p_src    ) return false;
	if( p_dst_num ) *p_dst_num = 0;
	*pp_dst = NULL;

	// UTF-8 to UTF-16
	if( !( num_wide = ::MultiByteToWideChar( CP_UTF8, 0, p_src, -1, NULL, 0 ) )              ) goto term; 
	if( !pxMem_zero_alloc( (void**)&p_wide, num_wide * sizeof(wchar_t) )                     ) goto term;
	if( !MultiByteToWideChar( CP_UTF8, 0, p_src, -1, p_wide, num_wide )                      ) goto term;

	if( p_dst_num ) *p_dst_num = num_wide - 1; // remove last ' '
	*pp_dst = p_wide;

	b_ret = true;
term:
	if( !b_ret ) pxMem_free( (void**)&p_wide );

	return b_ret;
}

bool pxwUTF8_code_utf8_to_wide( uint32_t code_src, wchar_t* p_code_dst )
{
	if( !code_src || !p_code_dst ) return false;

	char    str_utf8[8] = { 0 };
	wchar_t str_wide[8] = { 0 };
	int32_t i           =   0  ;

	if( i || code_src & 0xff000000 ){ str_utf8[i] = (char)( (code_src & 0xff000000) >> 24 ); i++; }
	if( i || code_src & 0x00ff0000 ){ str_utf8[i] = (char)( (code_src & 0x00ff0000) >> 16 ); i++; }
	if( i || code_src & 0x0000ff00 ){ str_utf8[i] = (char)( (code_src & 0x0000ff00) >>  8 ); i++; }
	if( i || code_src & 0x000000ff ){ str_utf8[i] = (char)( (code_src & 0x000000ff) >>  0 ); i++; }
	if( !MultiByteToWideChar( CP_UTF8, 0, str_utf8, -1, str_wide, 8 ) )
	{
		DWORD err = GetLastError();
		return false;
	}
	*p_code_dst = str_wide[ 0 ];
	return true;
}

bool pxwUTF8_utf8_to_sjis( const char* p_src, char** pp_dst, int32_t* p_dst_size )
{
	bool     b_ret     = false;
	wchar_t* p_wide    = NULL ;
	char*    p_sjis    = NULL ;
	int      num_wide  =     0;
	int      size_sjis =     0;
 
	if( !p_src     ) return false;
	if( p_dst_size ) *p_dst_size = 0;
	*pp_dst = NULL;

	// UTF-8 to UTF-16
	if( !( num_wide = ::MultiByteToWideChar( CP_UTF8, 0, p_src, -1, NULL, 0 ) )              ) goto term; 
	if( !pxMem_zero_alloc( (void**)&p_wide, num_wide * sizeof(wchar_t) )                     ) goto term;
	if( !MultiByteToWideChar( CP_UTF8, 0, p_src, -1, p_wide, num_wide )                      ) goto term;
 
	// UTF16 to Shift-JIS
	if( !( size_sjis = ::WideCharToMultiByte( _CP_SJIS, 0, p_wide, -1, NULL, 0, NULL, NULL ) ) ) goto term;
	if( !pxMem_zero_alloc( (void**)&p_sjis, size_sjis )                                      ) goto term;
	if( !WideCharToMultiByte( _CP_SJIS, 0, p_wide, -1, p_sjis, size_sjis, NULL, NULL )         ) goto term;
  
	if( p_dst_size ) *p_dst_size = size_sjis - 1; // remove last ' '
	*pp_dst = p_sjis;

	b_ret = true;
term:
	pxMem_free( (void**)&p_wide );
	if( !b_ret ) pxMem_free( (void**)&p_sjis );

	return b_ret;
}

bool pxwUTF8_wide_to_utf8( const wchar_t* p_src, char** pp_dst, int32_t* p_dst_size )
{
	bool     b_ret     = false;
	char*    p_utf8    = NULL ;
	int      size_utf8 =     0;

	if( !p_src     ) return false;
	if( p_dst_size ) *p_dst_size = 0;
	*pp_dst = NULL;

	// to UTF-8
 	if( !(size_utf8 = ::WideCharToMultiByte( CP_UTF8, 0, p_src, -1, NULL, 0, NULL, NULL ) ) ) goto term;
	if( !pxMem_zero_alloc( (void**)&p_utf8, size_utf8 )                                     ) goto term;
	if( !WideCharToMultiByte( CP_UTF8, 0, p_src, -1, p_utf8, size_utf8, NULL, NULL )        ) goto term;

	if( p_dst_size ) *p_dst_size = size_utf8 - 1; // remove last ' '
	*pp_dst = p_utf8;

	b_ret = true;
term:
	if( !b_ret ) pxMem_free( (void**)&p_utf8 );

	return b_ret;
}

bool pxwUTF8_wide_to_sjis( const wchar_t* p_src, char** pp_dst, int32_t* p_dst_size )
{
	bool     b_ret     = false;
	char*    p_sjis    = NULL ;
	int      size_sjis =     0;
 
	if( !p_src     ) return false;
	if( p_dst_size ) *p_dst_size = 0;
	*pp_dst = NULL;
 
	// UTF16 to Shift-JIS
	if( !( size_sjis = ::WideCharToMultiByte( _CP_SJIS, 0, p_src, -1, NULL, 0, NULL, NULL ) ) ) goto term;
	if( !pxMem_zero_alloc( (void**)&p_sjis, size_sjis )                                     ) goto term;
	if( !WideCharToMultiByte( _CP_SJIS, 0, p_src, -1, p_sjis, size_sjis, NULL, NULL )         ) goto term;
  
	if( p_dst_size ) *p_dst_size = size_sjis - 1; // remove last ' '
	*pp_dst = p_sjis;

	b_ret = true;
term:
	if( !b_ret ) pxMem_free( (void**)&p_sjis );

	return b_ret;
}
