// '16/01/25 pxDebugLog.cpp

#ifndef pxSTDAFX_H
#include <stdio.h>  // NULL
#include <stdarg.h> // va_list
#include <string.h> // strlen
#endif

#include "./pxDirectory.h"
#include "./pxFile2.h"
#include "./pxStrT.h"
#include "./pxPath.h"
#include "./pxDateTime.h"

#include "./pxMutex.h"

#include <iostream>

static bool        _b_init     = false;
static TCHAR       *_log_dir_name  = NULL ;
static const TCHAR *_ext_      = _T("dbg");
static pxMutex     *_mtx       = NULL ;

static pxFile2* _file = NULL;

void pxDebugLog_release()
{
//	cout << "dbglog release01" << endl;

	_b_init = false;

//	cout << "dbglog release02" << endl;

	SAFE_DELETE( _mtx );

//	cout << "dbglog release03" << endl;

//	pxStrT_free( &_dir_name );

//	cout << "dbglog release04" << endl;
	SAFE_DELETE( _file );

	pxStrT_free( &_log_dir_name );
}

bool pxDebugLog_init( const TCHAR *base_dir_name, const TCHAR* log_dir_name )
{
	if( _b_init ) return false;

	_file = new pxFile2();
	if( !_file->init_base( base_dir_name, true ) ) goto term;
	if( !pxStrT_copy_allocate( &_log_dir_name, log_dir_name )   ) goto term;
	if( !( _mtx = new pxMutex() ) || !_mtx->init()      ) goto term;

	_b_init = true;
term:
	if( !_b_init ) pxDebugLog_release();
	return _b_init;
}

void dlog_c( const char *fmt, ... )
{
	if( !_b_init || !_file || !_mtx ) return;

	_mtx->lock();
	{
		pxDATETIME dt; pxDateTime_get_now( &dt );
		TCHAR      name[  32 ] = {0};
		char       str [ 512 ] = {0};

#ifdef UNICODE
		_stprintf_s( name, 32, _T("%04d%02d%02d"), dt.Y, dt.M, dt.D );
#else
		sprintf    ( name,        "%04d%02d%02d" , dt.Y, dt.M, dt.D );
#endif
		pxDescriptor* desc = NULL;
		if( _file->open_a( &desc, _log_dir_name, name, _ext_ ) )
		{

//		pxFile file( false );
//		if( file.open( _dir_name, name, _T("a+t") ) )
//		{
			va_list ap; va_start( ap, fmt ); vsprintf( str, fmt, ap ); va_end( ap );
			desc->w_arg_asfile( "%02d%02d%02d,%s\n", dt.h, dt.m, dt.s, str );
#ifdef pxPLATFORM_windows
			OutputDebugStringA( str );
#endif
			SAFE_DELETE( desc );
//			file.close();
		}
	}
	_mtx->unlock();
}

void dlog_t( const char* text1, const TCHAR* text_t )
{
	if( !_b_init || !_file || !_mtx ) return;

	_mtx->lock();
	{
		pxDATETIME dt; pxDateTime_get_now( &dt );
		TCHAR      name    [  16 ] = {0};
		char       str_utf8[ 512 ] = {0};
		const char* str_c = NULL;
		int32_t    size_utf8 = 0;

#ifdef UNICODE
		_stprintf_s( name, 16, _T("%04d%02d%02d"), dt.Y, dt.M, dt.D );
#else
		sprintf    ( name,        "%04d%02d%02d" , dt.Y, dt.M, dt.D );
#endif


		pxDescriptor* desc = NULL;
		if( _file->open_a( &desc, _log_dir_name, name, _ext_ ) )
		{
#ifdef UNICODE
 			if( !( size_utf8 =      WideCharToMultiByte( CP_UTF8, 0, text_t, -1, NULL    ,         0, NULL, NULL ) ) ||
				size_utf8 >= 512|| !WideCharToMultiByte( CP_UTF8, 0, text_t, -1, str_utf8, size_utf8, NULL, NULL ) )
			{
				SAFE_DELETE( desc );
				_mtx->unlock(); return;
			}
			str_c = str_utf8;
#else
			str_c = text_t  ;
#endif


			desc->w_arg_asfile( "%02d%02d%02d,%s%s\n", dt.h, dt.m, dt.s, text1, str_c );
#ifdef pxPLATFORM_windows
			OutputDebugStringA( text1  );
			//OutputDebugStringW( text_t );
			OutputDebugString( text_t );
#endif
			SAFE_DELETE( desc );
		}
	}
	_mtx->unlock();
}

bool pxDebugLog_GetCurrentPath( TCHAR** p_path )
{
	if( !_b_init || !_file || !_mtx ) return false;
	pxDATETIME dt; pxDateTime_get_now( &dt );
	TCHAR name[ 16 ] = {};

#ifdef UNICODE
//	_stprintf_s( path, MAX_PATH, _T("%s/%s/%04d%02d%02d.%s"), pxFile::sttc_get_master_base_dir(), _dir_name, dt.Y, dt.M, dt.D, _ext_ );
	_stprintf_s( name,           _T(      "%04d%02d%02d"   ), dt.Y, dt.M, dt.D );
#else
//	sprintf    ( path,              "%s/%s/%04d%02d%02d.%s" , pxFile::sttc_get_master_base_dir(), _dir_name, dt.Y, dt.M, dt.D, _ext_ );
	sprintf    ( name,                    "%04d%02d%02d"    , dt.Y, dt.M, dt.D );
#endif

	return _file->make_real_path( p_path, _log_dir_name, name, _ext_ );
}

static int _GetPassedDays( const TCHAR* name )
{
#ifdef UNICODE
	if( _tcslen( name ) - _tcslen( _ext_ ) != 9 ) return -1;
#else
	if( strlen ( name ) - strlen ( _ext_ ) != 9 ) return -1;
#endif

	for( int i = 0; i < 8; i++ )
	{
		if( name[ i ] < '0' || name[ i ] > '9'  ) return -1;
	}

	pxDATETIME time_now ;
	pxDATETIME time_name; memset( &time_name, 0, sizeof(time_name) );

	pxDateTime_get_now( &time_now );

	time_name.Y = (uint16_t)( name[ 0 ] - '0' ) * 1000 +
				  (uint16_t)( name[ 1 ] - '0' ) *  100 +
				  (uint16_t)( name[ 2 ] - '0' ) *   10 +
				  (uint16_t)( name[ 3 ] - '0' ) *    1 ;
	time_name.M = (uint8_t )( name[ 4 ] - '0' ) *   10 +
				  (uint8_t )( name[ 5 ] - '0' ) *    1 ;
	time_name.D = (uint8_t )( name[ 6 ] - '0' ) *   10 +
				  (uint8_t )( name[ 7 ] - '0' ) *    1 ;

	return (int)pxDateTime_compare_days( &time_now, &time_name );
}


typedef struct
{
	int limit_day;
	int count    ;
}
_DELETEANDCOUNT;

bool _delete_func( void *user, const TCHAR *path )
{
	_DELETEANDCOUNT *p = (_DELETEANDCOUNT*)user;

	if( _GetPassedDays( pxPath_find_filename( path ) ) >= p->limit_day )
	{
		if( pxFile2_delete( path ) ) p->count++;
	}
	return true;
}

int pxDebugLog_DeleteOlds( int days )
{
	_DELETEANDCOUNT dac = {}; dac.limit_day = days;
	/*
//	TCHAR           path     [ pxBUFSIZE_PATH ];
//	TCHAR           path_find[ pxBUFSIZE_PATH ];

#ifdef UNICODE
	_stprintf_s( path_find, pxBUFSIZE_PATH, _T("%s/%s"  ), pxFile::sttc_get_trns_dir(), _dir_name );
	_stprintf_s( path     , pxBUFSIZE_PATH, _T("%s/*.%s"), path_find, _ext_ );
#else
	sprintf    ( path_find,                    "%s/%s"   , pxFile::sttc_get_trns_dir(), _dir_name );
	sprintf    ( path     ,                    "%s/*.%s" , path_find, _ext_ );
#endif
	*/
	TCHAR* path_find = NULL;
	if( !_file->make_real_path( &path_find, _log_dir_name, NULL, _ext_ ) ) return -1;
	pxDirectory_file_find( path_find, _ext_, false, _delete_func, &dac );

	return dac.count;
}
