// UTF-8 strings. The code base keeps all text as UTF-8 and only converts at an OS boundary.
#pragma once

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <memory>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <type_traits>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#endif

#include <pxStdDef.h> // sds
#if __has_include(<pxMem.h>)
#include <pxMem.h>
#endif
bool pxMem_free( void** pp );
#if defined(_WIN32) && __has_include(<pxwUTF8.h>)
#include <pxwUTF8.h>
#define UX_HAS_PXW_UTF8 1
#endif
#if defined(__WINE__) && __has_include(<pxWinelibFile.h>)
#include <pxWinelibFile.h> // pxwl_fopen: DOS path -> Unix path through Wine
#endif

class uxDS;

// ---------------------------------------------------------------------------------------------------------------
// Type-safety check: prevent passing non-primitive types / classes (like uxDS, uxT) to printf varargs.
template <class T>
struct ux_va_ok
	: std::integral_constant<bool, !std::is_class<typename std::remove_cv<typename std::remove_reference<T>::type>::type>::value> {};

inline void ux_va_check() {}
template <class T, class ... R>
inline void ux_va_check( const T&, const R&... rest )
{
	static_assert( ux_va_ok<T>::value, "do not pass a uxDS (or any class) to a printf-style function: use *s or s.c_str()" );
	ux_va_check( rest ... );
}

// ---------------------------------------------------------------------------------------------------------------
// Abstract string class providing SDS-backed static utility and formatting methods.
class ux_S
{
public:
virtual ~ux_S() = default;

virtual const char* c_str() const = 0;
virtual size_t      size()  const = 0;
virtual bool        empty() const = 0;

    // String conversions
static double to_double( const char* s )
{
	return s ? atof( s ) : 0.0;
}

static int to_int( const char* s, int radix = 10 )
{
	return s ? (int)strtol( s, NULL, radix ) : 0;
}

    // Safe copy backed by sds
static int copy( char* dst, size_t cap, const char* src )
{
	if( !dst || !cap ) return -1;
	if( !src ){ dst[ 0 ] = '\0'; return 0; }
	sds s = sdsnew( src );
	if( !s ){ dst[ 0 ] = '\0'; return -1; }
	size_t len = sdslen( s );
	int ret = 0;
	if( len >= cap )
	{
		memcpy( dst, s, cap - 1 );
		dst[ cap - 1 ] = '\0';
		ret = -1;
	}
	else
	{
		memcpy( dst, s, len + 1 );
	}
	sdsfree( s );
	return ret;
}

template <size_t N>
static int copy( char ( &dst )[ N ], const char* src )
{
	return copy( dst, N, src );
}

    // vsprintf backed by SDS (sdscatvprintf)
static int vsprintf( char* buf, size_t cap, const char* fmt, va_list ap )
{
	if( !buf || !cap ) return -1;
	sds s = sdscatvprintf( sdsempty(), fmt, ap );
	if( !s ){ buf[ 0 ] = '\0'; return -1; }
	size_t len = sdslen( s );
	int ret = (int)len;
	if( len >= cap )
	{
		memcpy( buf, s, cap - 1 );
		buf[ cap - 1 ] = '\0';
		ret = -1;
	}
	else
	{
		memcpy( buf, s, len + 1 );
	}
	sdsfree( s );
	return ret;
}

template <size_t N>
static int vsprintf( char ( &buf )[ N ], const char* fmt, va_list ap )
{
	return vsprintf( buf, N, fmt, ap );
}

static int vsprintf( uxDS& dst, const char* fmt, va_list ap );

template <class ... A>
static int sprintf( uxDS& dst, const char* fmt, const A&... args );

template <class ... A>
static int sprintf( uxDS& dst, size_t cap, const char* fmt, const A&... args );

#if defined( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif
static sds format_sds( const char* fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	sds s = sdscatvprintf( sdsempty(), fmt, ap );
	va_end( ap );
	return s;
}

static int sprintf( char* buf, size_t cap, const char* text )
{
	return copy( buf, cap, text );
}

template <size_t N>
static int sprintf( char ( &buf )[ N ], const char* text )
{
	return copy( buf, N, text );
}

template <class ... A>
static int sprintf( char* buf, size_t cap, const char* fmt, const A&... args )
{
	ux_va_check( args ... );
	if( !buf || !cap ) return -1;
	sds s = format_sds( fmt, args ... );
	if( !s ){ buf[ 0 ] = '\0'; return -1; }
	size_t len = sdslen( s );
	int ret = (int)len;
	if( len >= cap )
	{
		memcpy( buf, s, cap - 1 );
		buf[ cap - 1 ] = '\0';
		ret = -1;
	}
	else
	{
		memcpy( buf, s, len + 1 );
	}
	sdsfree( s );
	return ret;
}

template <size_t N, class ... A>
static int sprintf( char ( &buf )[ N ], const char* fmt, const A&... args )
{
	return sprintf( (char*)buf, N, fmt, args ... );
}
#if defined( __GNUC__ )
#pragma GCC diagnostic pop
#endif

    // Methods returning uxDS:
static uxDS from_int( long long value );
static uxDS trim( const char* s, const char* cset );
static uxDS to_lower( const char* s );
static uxDS to_upper( const char* s );
static uxDS range( const char* s, ssize_t start, ssize_t end );

static int itoa( int value, char* buf, size_t cap, int radix = 10 )
{
	if( !buf || !cap ) return -1;
	if( radix == 10 )
	{
		sds s = sdsfromlonglong( value );
		if( !s ){ buf[ 0 ] = '\0'; return -1; }
		int ret = copy( buf, cap, s );
		sdsfree( s );
		return ret;
	}
	const char* fmt = ( radix == 16 ) ? "%x" : ( ( radix == 8 ) ? "%o" : "%d" );
	return sprintf( buf, cap, fmt, value );
}

template <size_t N>
static int itoa( int value, char ( &buf )[ N ], int radix = 10 )
{
	return itoa( value, buf, N, radix );
}

static int compare( const char* a, const char* b )
{
	if( !a || !b ) return ( a != NULL ) - ( b != NULL );
	sds sa = sdsnew( a );
	sds sb = sdsnew( b );
	int ret = sdscmp( sa, sb );
	sdsfree( sa );
	sdsfree( sb );
	return ret;
}

static int compare( const uxDS& a, const uxDS& b );

static int icompare( const char* a, const char* b )
{
	if( !a || !b ) return ( a != NULL ) - ( b != NULL );
	sds sa = sdsnew( a );
	sds sb = sdsnew( b );
	sdstolower( sa );
	sdstolower( sb );
	int ret = sdscmp( sa, sb );
	sdsfree( sa );
	sdsfree( sb );
	return ret;
}

static int icompare( const char* a, const char* b, size_t n )
{
	if( !a || !b ) return ( a != NULL ) - ( b != NULL );
	sds sa = sdsnewlen( a, strnlen( a, n ) );
	sds sb = sdsnewlen( b, strnlen( b, n ) );
	sdstolower( sa );
	sdstolower( sb );
	int ret = sdscmp( sa, sb );
	sdsfree( sa );
	sdsfree( sb );
	return ret;
}

static int icompare( const uxDS& a, const uxDS& b );
};

#include "uxDS.h"
#include "uxSS.h"

inline uxDS ux_S::from_int( long long value )
{
	return uxDS::adopt( sdsfromlonglong( value ) );
}

inline uxDS ux_S::trim( const char* s, const char* cset )
{
	if( !s ) return uxDS();
	sds d = sdsnew( s );
	d = sdstrim( d, cset );
	return uxDS::adopt( d );
}

inline uxDS ux_S::to_lower( const char* s )
{
	if( !s ) return uxDS();
	sds d = sdsnew( s );
	sdstolower( d );
	return uxDS::adopt( d );
}

inline uxDS ux_S::to_upper( const char* s )
{
	if( !s ) return uxDS();
	sds d = sdsnew( s );
	sdstoupper( d );
	return uxDS::adopt( d );
}

inline uxDS ux_S::range( const char* s, ssize_t start, ssize_t end )
{
	if( !s ) return uxDS();
	sds d = sdsnew( s );
	sdsrange( d, start, end );
	return uxDS::adopt( d );
}

inline int ux_S::vsprintf( uxDS& dst, const char* fmt, va_list ap )
{
	dst.vformat( fmt, ap );
	return (int)dst.size();
}

template <class ... A>
inline int ux_S::sprintf( uxDS& dst, const char* fmt, const A&... args )
{
	ux_va_check( args ... );
	dst.format( fmt, args ... );
	return (int)dst.size();
}

template <class ... A>
inline int ux_S::sprintf( uxDS& dst, size_t /*cap*/, const char* fmt, const A&... args )
{
	return ux_S::sprintf( dst, fmt, args ... );
}

inline int ux_S::compare( const uxDS& a, const uxDS& b )
{
	return a.cmp( b );
}

inline int ux_S::icompare( const uxDS& a, const uxDS& b )
{
	return a.icmp( b );
}

// sdssplitlen: split s at every occurrence of sep ("," or ", " ...). A uxDS per piece; empty if s is unset.
inline std::vector<uxDS> uxDS_split( const uxDS& s, const char* sep )
{
	std::vector<uxDS> out;
	if( !s || !sep || !*sep ) return out;
	int  count  = 0;
	sds* tokens = sdssplitlen( s.raw(), (ssize_t)s.size(), sep, (int)strlen( sep ), &count );
	if( !tokens ) return out;
	for( int i = 0; i < count; i++ ) out.push_back( uxDS( tokens[ i ], sdslen( tokens[ i ] ) ) );
	sdsfreesplitres( tokens, count );
	return out;
}

#if defined(UX_HAS_PXW_UTF8)

// OS text -> a new UTF-8 uxDS. Unset (NULL) on failure.
inline uxDS uxDS_from_t( const TCHAR* text_t )
{
	if( !text_t ) return uxDS();
#if defined UNICODE
	char* p_utf8 = NULL;
	if( !pxwUTF8_wide_to_utf8( text_t, &p_utf8, NULL ) ) return uxDS();
#elif defined _WIN32
	char* p_utf8 = NULL;
	if( !pxwUTF8_sjis_to_utf8( text_t, &p_utf8, NULL ) ) return uxDS();
#else
	return uxDS( text_t );
#endif
#ifdef _WIN32
	uxDS s( p_utf8 );
	pxMem_free( (void**)&p_utf8 );
	return s;
#endif
}

// OS text -> a UTF-8 buffer of "cap" bytes. Truncates on a character boundary, always terminates.
// Returns false (and leaves an empty string) when the conversion fails. Note that a UTF-8 buffer holds
// fewer characters than the same number of TCHARs did: non-ASCII text takes 2 to 4 bytes each.
inline bool uxCopy_from_t( char* dst, size_t cap, const TCHAR* text_t )
{
	if( !dst || !cap ) return false;
	dst[ 0 ] = '\0';

	uxDS s = uxDS_from_t( text_t );
	if( !s ) return false;

	size_t len = s.size();
	if( len >= cap )
	{
		len = cap - 1;
		while( len && ( (unsigned char)( *s )[ len ] & 0xC0 ) == 0x80 ) len--; // do not cut a multi-byte character
	}
	memcpy( dst, *s, len );
	dst[ len ] = '\0';
	return true;
}

template <size_t N>
inline bool uxCopy_from_t( char ( &dst )[ N ], const TCHAR* text_t ){ return uxCopy_from_t( dst, N, text_t ); }

// Shift-JIS <-> UTF-8, for the data pxtone stores in Shift-JIS (unit / voice / project names, comments).
// The result is an owning uxDS; unset on failure.
inline uxDS uxDS_from_sjis( const char* sjis )
{
	if( !sjis ) return uxDS();
#ifdef _WIN32
	char* p_utf8 = NULL;
	if( !pxwUTF8_sjis_to_utf8( sjis, &p_utf8, NULL ) ) return uxDS();
	uxDS s( p_utf8 );
	pxMem_free( (void**)&p_utf8 );
	return s;
#else
	return uxDS( sjis );
#endif
}

// UTF-8 -> Shift-JIS bytes (held in a uxDS: use .c_str()).
inline uxDS uxDS_to_sjis( const char* utf8 )
{
	if( !utf8 ) return uxDS();
#ifdef _WIN32
	char* p_sjis = NULL;
	if( !pxwUTF8_utf8_to_sjis( utf8, &p_sjis, NULL ) ) return uxDS();
	uxDS s( p_sjis );
	pxMem_free( (void**)&p_sjis );
	return s;
#else
	return uxDS( utf8 );
#endif
}

// UTF-8 -> OS text for a call into the OS / a TCHAR API. Converts to "const TCHAR*"; NULL stays NULL.
// The buffer lives until the end of the full expression: use it as an argument, never store the pointer.
//   MessageBox( hwnd, uxT( text ), uxT( title ), MB_OK );
class uxT
{
TCHAR* _p;

public:
explicit uxT( const char* utf8 ) : _p( NULL )
{
	if( !utf8 ) return;
#if defined UNICODE
	pxwUTF8_utf8_to_wide( utf8, (wchar_t**)&_p, NULL );
#elif defined _WIN32
	pxwUTF8_utf8_to_sjis( utf8, (char**)&_p, NULL );
#else
	_p = (TCHAR*)strdup( utf8 );
#endif
}
~uxT(){ pxMem_free( (void**)&_p ); }

uxT( const uxT& )            = delete;
uxT& operator=( const uxT& ) = delete;

operator const TCHAR*() const { return _p; }
};

// UTF-8 -> the LPARAM of a message that carries a string (CB_ADDSTRING, CB_FINDSTRING, WM_SETTEXT, ...).
// Same lifetime rule as uxT. It takes UTF-8 only: handing it OS text (wchar_t*) is a compile error, and so is
// forgetting it, because a bare (LPARAM)"text" cast is what silently sends narrow text to a wide control.
//   SendDlgItemMessage( hDlg, IDC_COMBO_BEAT, CB_ADDSTRING, 0, uxLP( "4" ) );
class uxLP
{
uxT _t;

public:
explicit uxLP( const char* utf8 ) : _t( utf8 ){}
uxLP( const wchar_t* )  = delete; // already OS text: pass (LPARAM)text
uxLP( const char16_t* ) = delete;

operator intptr_t() const { return (intptr_t)(const TCHAR*)_t; }
};

// OS writes text into a TCHAR buffer; this hands it a scratch one and, when the full expression ends, converts
// the result into the UTF-8 destination. "tchars" is how many TCHARs the OS may write (what you pass as the size).
//   GetDlgItemText( hDlg, IDC_PATH, uxTOut( path ), MAX_PATH );
//   GetModuleFileName( NULL, uxTOut( dir, MAX_PATH ), MAX_PATH );
class uxTOut
{
char*              _dst  ;
size_t             _cap  ;
uxDS*              _ds   ;
std::vector<TCHAR> _buf  ;

public:
uxTOut( char* dst, size_t dst_cap, size_t tchars ) : _dst( dst ), _cap( dst_cap ), _ds( NULL ), _buf( tchars + 1, 0 ){}
uxTOut( char* dst, size_t cap )                    : uxTOut( dst, cap, cap ){}

template <size_t N>
explicit uxTOut( char ( &dst )[ N ] )              : uxTOut( dst, N, N ){}

    // fill a uxDS (no length limit on the UTF-8 side)
explicit uxTOut( uxDS& dst, size_t tchars = 4096 ) : _dst( NULL ), _cap( 0 ), _ds( std::addressof( dst ) ), _buf( tchars + 1, 0 ){}

uxTOut( const uxTOut& )            = delete;
uxTOut& operator=( const uxTOut& ) = delete;

~uxTOut(){ if( _ds ) *_ds = uxDS_from_t( _buf.data() ); else if( _dst && _cap ) uxCopy_from_t( _dst, _cap, _buf.data() ); }

operator TCHAR*(){ return _buf.data(); }
};

#endif // UX_HAS_PXW_UTF8

#if defined( __GNUC__ )
#define UX_PRINTF( fmt_idx, args_idx ) __attribute__(( format( printf, fmt_idx, args_idx ) ))
#else
#define UX_PRINTF( fmt_idx, args_idx )
#endif

// Case-insensitive compare backed by SDS.
inline int ux_strnicmp( const char* a, const char* b, size_t n ){ return ux_S::icompare( a, b, n ); }
inline int ux_stricmp( const char* a, const char* b ){ return ux_S::icompare( a, b ); }
inline int ux_stricmp( const uxDS& a, const uxDS& b ){ return a.icmp( b ); }
inline int ux_stricmp( const uxDS& a, const char*  b ){ return a.icmp( uxDS( b ) ); }
inline int ux_stricmp( const char*  a, const uxDS& b ){ return uxDS( a ).icmp( b ); }

// Formatting functions forwarding to SDS-backed ux_S static methods.
inline int ux_vsprintf_s( char* buf, size_t cap, const char* fmt, va_list ap )
{
	return ux_S::vsprintf( buf, cap, fmt, ap );
}

template <size_t N>
inline int ux_vsprintf_s( char ( &buf )[ N ], const char* fmt, va_list ap )
{
	return ux_S::vsprintf( buf, fmt, ap );
}

template <class ... A>
inline int ux_sprintf_s( char* buf, size_t cap, const char* fmt, const A&... args )
{
	return ux_S::sprintf( buf, cap, fmt, args ... );
}

template <size_t N, class ... A>
inline int ux_sprintf_s( char ( &buf )[ N ], const char* fmt, const A&... args )
{
	return ux_S::sprintf( buf, fmt, args ... );
}

template <class ... A>
inline int ux_sprintf_s( uxDS& dst, const char* fmt, const A&... args )
{
	return ux_S::sprintf( dst, fmt, args ... );
}

template <class ... A>
inline int ux_sprintf_s( uxDS& dst, size_t /*cap, ignored*/, const char* fmt, const A&... args )
{
	return ux_S::sprintf( dst, fmt, args ... );
}

// fopen() for a UTF-8 path. NULL on failure.
//   Winelib: UTF-8 -> wide -> Wine's DOS-to-Unix path translation (glibc cannot open "Z:\\home\\...")
//   Windows: _wfopen, so any Unicode path works
//   other  : plain fopen
inline FILE* ux_fopen( const char* utf8_path, const char* mode )
{
	if( !utf8_path || !mode ) return NULL;
#if defined( __WINE__ ) && defined( UX_HAS_PXW_UTF8 )
	wchar_t* p_wide = NULL;
	if( !pxwUTF8_utf8_to_wide( utf8_path, &p_wide, NULL ) ) return NULL;
	FILE* fp = pxwl_fopen( (const WCHAR*)p_wide, mode );
	pxMem_free( (void**)&p_wide );
	return fp;
#elif defined( _WIN32 ) && defined( UX_HAS_PXW_UTF8 )
	wchar_t* p_wide = NULL;
	if( !pxwUTF8_utf8_to_wide( utf8_path, &p_wide, NULL ) ) return NULL;
	wchar_t mode_w[ 16 ] = {};
	for( size_t i = 0; mode[ i ] && i < 15; i++ ) mode_w[ i ] = (wchar_t)mode[ i ];
	FILE* fp = _wfopen( p_wide, mode_w );
	pxMem_free( (void**)&p_wide );
	return fp;
#else
	return fopen( utf8_path, mode );
#endif
}
