// uxtone dynamic string

// pass it by reference: "const uxDS& name" to read, "uxDS& name" to fill or change
// copying is a deep copy (sdsdup) and moving is free, so returning one by value is fine
// a default-constructed uxDS is "unset" (NULL), the same as the NULL pointers the old TCHAR code passed around:
// `if( s )` tests that, and it converts to NULL for const char* parameters

#pragma once

#ifdef __cplusplus

#include <stdarg.h>
#include <string.h>

class uxDS : public ux_S
{
sds _s;

public:
uxDS() : _s( NULL ){}
uxDS( const char* utf8 ) : _s( utf8 ? sdsnew( utf8 ) : NULL ){}                    // from a literal / uxSS<N> / any char*
uxDS( const char* utf8, size_t len ) : _s( utf8 ? sdsnewlen( utf8, len ) : NULL ){}
uxDS( const uxDS& o ) : _s( o._s ? sdsdup( o._s ) : NULL ){}
uxDS( uxDS&& o ) noexcept : _s( o._s ){ o._s = NULL; }
~uxDS() override { sdsfree( _s ); }                                                // sdsfree( NULL ) is a no-op

uxDS& operator=( const uxDS& o ){ uxDS t( o ); swap( t ); return *this; }
uxDS& operator=( uxDS&& o ) noexcept { swap( o ); return *this; }
uxDS& operator=( const char* utf8 ){ uxDS t( utf8 ); swap( t ); return *this; }

    // Taking the address of a uxDS is an error on purpose: the old (void**)&p / pxMem_free( &p ) idiom would free() an sds.
    // Pass a reference (uxDS&) to let a function fill it in.
uxDS* operator&() = delete;
const uxDS* operator&() const = delete;

void swap( uxDS& o ) noexcept { sds t = _s; _s = o._s; o._s = t; }

    // Take ownership of an sds made elsewhere / give it back (the caller sdsfree()s it).
static uxDS adopt( sds s ){ uxDS d; d._s = s; return d; }
sds         release()     { sds s = _s; _s = NULL; return s; }

    // access
explicit operator bool() const { return _s != NULL; }
operator const char*()   const { return _s; }                                      // NULL when unset, like the old pointers
const char* c_str()      const override { return _s ? _s : ""; }                            // never NULL
const char* operator*()  const { return c_str(); }                                  // printf( "%s", *s ): a class cannot go through "..."
sds         raw()        const { return _s; }                                      // for the sds*() functions
size_t      size()       const override { return _s ? sdslen( _s ) : 0; }
bool        empty()      const override { return !_s || !sdslen( _s ); }

    // change
void   clear(){ if( _s ) sdsclear( _s ); }
void   truncate( size_t len ){ if( !_s || len >= sdslen( _s ) ) return; if( len ) sdsrange( _s, 0, (ssize_t)len - 1 ); else sdsclear( _s ); }
uxDS&  operator+=( const char* utf8 ){ if( utf8 ){ if( !_s ) _s = sdsempty(); _s = sdscat( _s, utf8 ); } return *this; }
uxDS&  operator+=( const uxDS& o )  { return *this += (const char*)o; }

    // The sds string functions as methods. Each one makes the string first if it is unset, so none of them can fail on NULL.
uxDS&  cat( const char* utf8, size_t len ){ if( utf8 ){ if( !_s ) _s = sdsempty(); _s = sdscatlen( _s, utf8, len ); } return *this; }   // sdscatlen
uxDS&  cpy( const char* utf8 ){ if( !utf8 ) return *this = uxDS(); if( !_s ) _s = sdsempty(); _s = sdscpy( _s, utf8 ); return *this; }     // sdscpy (reuses the buffer)
uxDS&  trim( const char* cset ){ if( _s ) _s = sdstrim( _s, cset ); return *this; }                                                       // sdstrim: strip any of cset from both ends
uxDS&  range( ssize_t first, ssize_t last ){ if( _s ) sdsrange( _s, first, last ); return *this; }                                       // sdsrange: keep [first, last], negative counts from the end
uxDS&  to_lower(){ if( _s ) sdstolower( _s ); return *this; }                                                                             // sdstolower (ASCII)
uxDS&  to_upper(){ if( _s ) sdstoupper( _s ); return *this; }                                                                             // sdstoupper (ASCII)
uxDS&  map_chars( const char* from, const char* to ){ if( _s && from && to ) _s = sdsmapchars( _s, from, to, strlen( from ) ); return *this; } // sdsmapchars: from[i] -> to[i]
int    cmp( const uxDS& o ) const { if( !_s || !o._s ) return ( _s != NULL ) - ( o._s != NULL ); return sdscmp( _s, o._s ); }             // sdscmp (unset sorts first)
int    cmp( const char* utf8 ) const                                                                                                      // same algorithm as sdscmp, without making an sds of the argument
{
	if( !_s || !utf8 ) return ( _s != NULL ) - ( utf8 != NULL );
	size_t l1 = sdslen( _s ), l2 = strlen( utf8 );
	int    c  = memcmp( _s, utf8, l1 < l2 ? l1 : l2 );
	return c ? c : ( l1 > l2 ) - ( l1 < l2 );
}
int    icmp( const uxDS& o ) const { uxDS a( *this ), b( o ); a.to_lower(); b.to_lower(); return a.cmp( b ); }                            // ASCII case-insensitive cmp
static uxDS from_int( long long v ){ return adopt( sdsfromlonglong( v ) ); }                                                              // sdsfromlonglong

uxDS&  catprintf( const char* fmt, ... )
#if defined( __GNUC__ )
__attribute__(( format( printf, 2, 3 ) ))
#endif
;
uxDS&  format( const char* fmt, ... )   // replace the content with printf text (sdsclear + sdscatvprintf)
#if defined( __GNUC__ )
__attribute__(( format( printf, 2, 3 ) ))
#endif
;
uxDS&  vformat( const char* fmt, va_list ap );
};

// Compared by text, never by address (the implicit const char* conversion would otherwise compare pointers).
inline bool operator==( const uxDS& a, const uxDS& b ){ return a.c_str() == b.c_str() || ( a && b && !a.cmp( b ) ); }
inline bool operator==( const uxDS& a, const char* b ){ return a && b && !a.cmp( b ); }
inline bool operator==( const char* a, const uxDS& b ){ return b == a; }
inline bool operator!=( const uxDS& a, const uxDS& b ){ return !( a == b ); }
inline bool operator!=( const uxDS& a, const char* b ){ return !( a == b ); }
inline bool operator!=( const char* a, const uxDS& b ){ return !( b == a ); }

inline uxDS& uxDS::catprintf( const char* fmt, ... )
{
	va_list ap; va_start( ap, fmt );
	if( !_s ) _s = sdsempty();
	_s = sdscatvprintf( _s, fmt, ap );
	va_end( ap );
	return *this;
}

inline uxDS& uxDS::vformat( const char* fmt, va_list ap )
{
	if( !_s ) _s = sdsempty(); else sdsclear( _s );
	_s = sdscatvprintf( _s, fmt, ap );
	return *this;
}

inline uxDS& uxDS::format( const char* fmt, ... )
{
	va_list ap; va_start( ap, fmt );
	vformat( fmt, ap );
	va_end( ap );
	return *this;
}

#endif // __cplusplus

