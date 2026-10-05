// '15/12/13 pxError.cpp

#define _CRT_SECURE_NO_WARNINGS

#ifndef pxSTDAFX_H
#include <stdlib.h> // malloc.
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#endif

// #include "./pxTText.h"
#include <uxStr.h>

#include "./pxMem.h"
#include "./pxError.h"

#define _BUFSIZE_ERROR 1024

static char*       _buf               = NULL      ;
static char        _str_date   [ 24 ] = "--/--/--";
static const char  _buf_default[ 24 ] = "no error";
static bool        _b_err             = false     ;

#ifdef _WIN32
static SYSTEMTIME _st;
#else
static struct tm  _t ;
#endif


bool pxError_init    ()
{
	_b_err = false;
	if( !pxMem_zero_alloc( (void**)&_buf, _BUFSIZE_ERROR*sizeof(char) ) ) return false;
	return true;
}

void pxError_reset   ()
{
	if( _buf )
	{
		memset( _buf, 0, sizeof(_BUFSIZE_ERROR) );
		memcpy( _buf, _buf_default, sizeof(_buf_default) );
	}
	_b_err = false;
}

void pxError_release ()
{
	pxMem_free( (void**)&_buf );
}

bool        pxError_is_error   (){ return _b_err   ; }
const char* pxError_get_message(){ return _buf     ; }
const char* pxError_get_date   (){ return _str_date; }


static void _set_date()
{
#ifdef _WIN32

	GetLocalTime( &_st );
	sprintf( _str_date, "%04d/%02d/%02d,%02d:%02d:%02d",
			 _st.wYear , _st.wMonth , _st.wDay,
			 _st.wHour , _st.wMinute, _st.wSecond );

#elif defined pxSCE

	time_t longtime;
	longtime = time(NULL);
	localtime_s( &longtime, &_t );
	sprintf( _str_date, "%04d/%02d/%02d,%02d:%02d:%02d",
			 _t.tm_year + 1900, _t.tm_mon + 1, _t.tm_mday,
			 _t.tm_hour       , _t.tm_min    , _t.tm_sec );

#else
	time_t    tt;
	time(    &tt );
	localtime_r( &tt, &_t );
	sprintf( _str_date, "%04d/%02d/%02d,%02d:%02d:%02d",
			 _t.tm_year + 1900, _t.tm_mon + 1, _t.tm_mday,
			 _t.tm_hour       , _t.tm_min    , _t.tm_sec );
#endif
}


bool pxerr  ()
{
	return pxerr( "fatal" );
}

bool pxerr( const char *fmt, ... )
{
	if( !_buf  ) return false;
	if( _b_err ) return false;

	_set_date();

	va_list ap; va_start( ap, fmt ); vsprintf    ( _buf,                 fmt, ap ); va_end( ap );

	_b_err = true;
	return true;
}

bool pxerr_t( const char* text1, const uxDS& text_t )
{
	if( !_buf  ) return false;
	if( _b_err ) return false;

	_set_date();

	strcpy( _buf, text1 );

	// pxTText tt;
	uxDS tt;
	// if( !tt.set_TCHAR_to_sjis( text_t ) ) return false;
	if( !( tt = uxDS_to_sjis( text_t ) ) ) return false;
	// strcat( _buf, tt.sjis() );
	strcat( _buf, *tt );

	_b_err = true;
	return true;
}
