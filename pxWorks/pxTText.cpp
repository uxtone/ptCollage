
#include "./pxError.h"
#include "./pxMem.h"

#ifdef _WIN32
#include <pxwUTF8.h>
#endif

#include "pxStr.h"
#include "./pxTText.h"

pxTText::pxTText()
{
	_p_text_t = NULL;
	_p_wide   = NULL;
	_p_sjis   = NULL;
}

pxTText::~pxTText()
{
	_clear();
}

void pxTText::_clear()
{
	pxMem_free( (void**)&_p_wide );
	pxMem_free( (void**)&_p_sjis );
}


bool pxTText::set_sjis_to_t( const char* p_text )
{
	if( !p_text ) return false;

	_clear();

#ifdef UNICODE
	if( !pxwUTF8_sjis_to_wide( p_text, &_p_wide, NULL ) ){ pxerr( "tt-set:%s", p_text ); return false; }
	_p_text_t = _p_wide;
#else
	if( !pxStr_copy_allocate( &_p_sjis, p_text ) ) return false;
	_p_text_t = _p_sjis;
#endif
	return true;
}

bool pxTText::set_UTF8_to_t( const char* p_text )
{
	if( !p_text ) return false;

	_clear();

#ifdef UNICODE
	if( !pxwUTF8_utf8_to_wide( p_text, &_p_wide, NULL ) ) return false;
	_p_text_t = _p_wide;
#elif defined _WIN32
	if( !pxwUTF8_utf8_to_sjis( p_text, &_p_sjis, NULL ) ) return false;
	_p_text_t = _p_sjis;
#else
	if( !pxStr_copy_allocate( &_p_sjis, p_text ) ) return false;
	_p_text_t = _p_sjis;
#endif
	return true;
}

bool pxTText::set_TCHAR_to_sjis( const TCHAR* text_t )
{
	if( !text_t ) return false;

	_clear();

#ifdef UNICODE
	if( !pxwUTF8_wide_to_sjis( text_t, &_p_sjis, NULL ) ) return false;
#else
	if( !pxStr_copy_allocate( &_p_sjis, text_t ) ) return false;
#endif
	return true;
}

const TCHAR* pxTText::tchr() const
{
	return _p_text_t;
}

const char* pxTText::sjis() const
{
	return _p_sjis;
}
