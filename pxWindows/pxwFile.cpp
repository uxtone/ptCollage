//  '11/08/12 pxwFile.cpp
//  '16/02/00 dir_base/dir_cmmn.

#include <pxStrT.h>
#include <pxMem.h>

#include "./pxwFilePath.h"

#include "./pxwFile.h"

#define MAX_PATH 260

static TCHAR *_path_dir_master_base = NULL;
static TCHAR *_path_dir_master_cmmn = NULL;
static TCHAR *_path_dir_trns        = NULL;

bool pxwFile_set_master_base_dir( const TCHAR *dir_name )
{
	if( !pxwFilePath_MakeFolderPath( &_path_dir_master_base, dir_name, false ) ) return false;
	return true;
}

bool pxwFile_set_master_cmmn_dir( const TCHAR *dir_name )
{
	if( !pxwFilePath_MakeFolderPath( &_path_dir_master_cmmn, dir_name, false ) ) return false;
	return true;
}

bool pxwFile_set_trns_dir       ( const TCHAR *dir_name )
{
	if( !pxwFilePath_MakeFolderPath( &_path_dir_trns, dir_name, true ) ) return false;
	return true;
}


bool pxwFile_cerate_trns_sub_dir( const TCHAR *dir_name )
{
	if( !_path_dir_trns ) return false;
	TCHAR path[ MAX_PATH ] = {0};
	_stprintf_s( path, MAX_PATH, _T("%s\\%s"), _path_dir_trns, dir_name );
	if( PathIsDirectory ( path       ) ) return true ;
	if( !CreateDirectory( path, NULL ) ) return false;
	return true;
}

const TCHAR *pxwFile_get_master_base_dir()
{
	return _path_dir_master_base;
}

const TCHAR *pxwFile_get_master_cmmn_dir()
{
	return _path_dir_master_cmmn;
}

const TCHAR *pxwFile_get_trns_dir()
{
	return _path_dir_trns;
}

void pxwFile_release()
{
	pxStrT_free( &_path_dir_master_base );
	pxStrT_free( &_path_dir_master_cmmn );
	pxStrT_free( &_path_dir_trns        );
}

bool pxwFile_trns_delete( const TCHAR *dir, const TCHAR *name )
{
	if( !_path_dir_trns ) return false;
	TCHAR path[ MAX_PATH ] = {0};
	_stprintf_s( path, MAX_PATH, _T("%s\\%s\\%s"), _path_dir_trns, dir, name );
	return DeleteFile( path ) ? true : false;
}

bool pxwFile_delete( const TCHAR *path )
{
	return DeleteFile( path ) ? true : false;
}

void *pxwFile_open_by_path( const TCHAR *path, const TCHAR *mode, int *p_size )
{
	FILE   *fp = NULL;

	if( !( fp = _tfopen( path, mode ) ) ) return NULL;

	if( mode[ 0 ] != 'w' && p_size )
	{
		fpos_t sz;
		fseek  ( fp, 0, SEEK_END );
		fgetpos( fp, &sz );
		if( p_size ) *p_size = (int)pxFPOS_OFFSET(sz);
		fseek  ( fp, 0, SEEK_SET );
	}
	return fp;
}

void *pxwFile_open( const TCHAR *dir_name, const TCHAR *name, const TCHAR *mode, int *p_size )
{
	void *ret = NULL;
	TCHAR path[ MAX_PATH ] = { 0 };

	if( !_path_dir_master_base )
	{
        if( dir_name ) _stprintf_s( path, MAX_PATH, _T("%s\\%s\\%s"), _path_dir_master_base, dir_name, name );
		else           _stprintf_s( path, MAX_PATH, _T("%s\\%s"    ), _path_dir_master_base,           name );
		if( ret = pxwFile_open_by_path( path, mode, p_size ) ) return ret;
	}
	if( !_path_dir_master_cmmn )
	{
		if( dir_name ) _stprintf_s( path, MAX_PATH, _T("%s\\%s\\%s"), _path_dir_master_cmmn, dir_name, name );
		else           _stprintf_s( path, MAX_PATH, _T("%s\\%s"    ), _path_dir_master_cmmn,           name );
		if( ret = pxwFile_open_by_path( path, mode, p_size ) ) return ret;
	}
	return NULL;
}

bool pxwFile_close( void **pfp )
{
	if( !pfp || !(*pfp) ) return false;
	if( *pfp ) fclose( (FILE*)*pfp );
	*pfp = NULL;
	return true;
}

bool pxwFile_make_real_path_master_base( TCHAR** p_real_path, const TCHAR *dir_name, const TCHAR *file_name )
{
	TCHAR real_path[ MAX_PATH ] = {};

	if( dir_name && dir_name[ 1 ] == ':' && dir_name[ 2 ] == '\\' )
	{
		if( file_name ) _stprintf_s( real_path, _T("%s\\%s"), dir_name, file_name );
		else            _stprintf_s( real_path, _T("%s"    ), dir_name            );
		if( !pxStrT_copy_allocate( p_real_path, real_path ) ) return false;
		return true;
	}

	if( !_path_dir_master_base ) return false;

	if( dir_name )
	{
		if( file_name ) _stprintf_s( real_path, _T("%s\\%s\\%s"), _path_dir_master_base, dir_name, file_name );
		else            _stprintf_s( real_path, _T("%s\\%s"    ), _path_dir_master_base, dir_name );
	}
	else
	{
		if( file_name ) _stprintf_s( real_path, _T("%s\\%s"    ), _path_dir_master_base, file_name );
		else            _stprintf_s( real_path, _T("%s"        ), _path_dir_master_base );
	}
	if( !pxStrT_copy_allocate( p_real_path, real_path ) ) return false;
	return true;
}

bool pxwFile_make_real_path_master_cmmn( TCHAR** p_real_path, const TCHAR *dir_name, const TCHAR *file_name )
{
	if( !_path_dir_master_cmmn ) return NULL;
	TCHAR real_path[ MAX_PATH ] = {};
	if( dir_name ) _stprintf_s( real_path, _T("%s\\%s\\%s"), _path_dir_master_cmmn, dir_name, file_name );
	else           _stprintf_s( real_path, _T("%s\\%s"    ), _path_dir_master_cmmn,           file_name );
	if( !pxStrT_copy_allocate( p_real_path, real_path ) ) return false;
	return true;
}

bool pxwFile_make_real_path_trns( TCHAR** p_real_path, const TCHAR *dir_name, const TCHAR *file_name )
{
	if( !_path_dir_trns ) return NULL;
	TCHAR real_path[ MAX_PATH ] = {};
	if( dir_name ) _stprintf_s( real_path, _T("%s\\%s\\%s"), _path_dir_trns, dir_name, file_name );
	else           _stprintf_s( real_path, _T("%s\\%s"    ), _path_dir_trns,           file_name );
	if( !pxStrT_copy_allocate( p_real_path, real_path ) ) return false;
	return true;
}
