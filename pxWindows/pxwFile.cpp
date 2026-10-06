//  '11/08/12 pxwFile.cpp
//  '16/02/00 dir_base/dir_cmmn.

#include <pxStrT.h>


#include <pxMem.h>

#include "./pxwFilePath.h"

#include "./pxwFile.h"

#define MAX_PATH 260

static uxDS _path_dir_master_base = NULL;
static uxDS _path_dir_master_cmmn = NULL;
static uxDS _path_dir_trns        = NULL;

bool pxwFile_set_master_base_dir( const uxDS& dir_name )
{
	if( !pxwFilePath_MakeFolderPath( _path_dir_master_base, dir_name, false ) ) return false;
	return true;
}

bool pxwFile_set_master_cmmn_dir( const uxDS& dir_name )
{
	if( !pxwFilePath_MakeFolderPath( _path_dir_master_cmmn, dir_name, false ) ) return false;
	return true;
}

bool pxwFile_set_trns_dir       ( const uxDS& dir_name )
{
	if( !pxwFilePath_MakeFolderPath( _path_dir_trns, dir_name, true ) ) return false;
	return true;
}


bool pxwFile_cerate_trns_sub_dir( const uxDS& dir_name )
{
	if( !_path_dir_trns ) return false;
	uxSS<MAX_PATH> path = {0};
	ux_sprintf_s( path, MAX_PATH, "%s\\%s", *_path_dir_trns, *dir_name );
	if( PathIsDirectory ( uxT( path )       ) ) return true ;
	if( !CreateDirectory( uxT( path ), NULL ) ) return false;
	return true;
}

const uxDS pxwFile_get_master_base_dir()
{
	return _path_dir_master_base;
}

const uxDS pxwFile_get_master_cmmn_dir()
{
	return _path_dir_master_cmmn;
}

const uxDS pxwFile_get_trns_dir()
{
	return _path_dir_trns;
}

void pxwFile_release()
{
	pxStrT_free( _path_dir_master_base );
	pxStrT_free( _path_dir_master_cmmn );
	pxStrT_free( _path_dir_trns        );
}

bool pxwFile_trns_delete( const uxDS& dir, const uxDS& name )
{
	if( !_path_dir_trns ) return false;
	uxSS<MAX_PATH> path = {0};
	ux_sprintf_s( path, MAX_PATH, "%s\\%s\\%s", *_path_dir_trns, *dir, *name );
	return DeleteFile( uxT( path ) ) ? true : false;
}

bool pxwFile_delete( const uxDS& path )
{
	return DeleteFile( uxT( path ) ) ? true : false;
}

void *pxwFile_open_by_path( const uxDS& path, const uxDS& mode, int *p_size )
{
	FILE   *fp = NULL;

	if( !( fp = ux_fopen( path, mode ) ) ) return NULL;

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

void *pxwFile_open( const uxDS& dir_name, const uxDS& name, const uxDS& mode, int *p_size )
{
	void *ret = NULL;
	uxSS<MAX_PATH> path = { 0 };

	if( !_path_dir_master_base )
	{
		if( dir_name ) ux_sprintf_s( path, MAX_PATH, "%s\\%s\\%s", *_path_dir_master_base, *dir_name, *name );
		else           ux_sprintf_s( path, MAX_PATH, "%s\\%s", *_path_dir_master_base,           *name );
		if( ret = pxwFile_open_by_path( path, mode, p_size ) ) return ret;
	}
	if( !_path_dir_master_cmmn )
	{
		if( dir_name ) ux_sprintf_s( path, MAX_PATH, "%s\\%s\\%s", *_path_dir_master_cmmn, *dir_name, *name );
		else           ux_sprintf_s( path, MAX_PATH, "%s\\%s", *_path_dir_master_cmmn,           *name );
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

bool pxwFile_make_real_path_master_base( uxDS& real_path_dst, const uxDS& dir_name, const uxDS& file_name )
{
	uxSS<MAX_PATH> real_path = {};

	if( dir_name && dir_name[ 1 ] == ':' && dir_name[ 2 ] == '\\' )
	{
		if( file_name ) ux_sprintf_s( real_path, "%s\\%s", *dir_name, *file_name );
		else            ux_sprintf_s( real_path, "%s", *dir_name            );
		if( !pxStrT_copy_allocate( real_path_dst, real_path ) ) return false;
		return true;
	}

	if( !_path_dir_master_base ) return false;

	if( dir_name )
	{
		if( file_name ) ux_sprintf_s( real_path, "%s\\%s\\%s", *_path_dir_master_base, *dir_name, *file_name );
		else            ux_sprintf_s( real_path, "%s\\%s", *_path_dir_master_base, *dir_name );
	}
	else
	{
		if( file_name ) ux_sprintf_s( real_path, "%s\\%s", *_path_dir_master_base, *file_name );
		else            ux_sprintf_s( real_path, "%s", *_path_dir_master_base );
	}
	if( !pxStrT_copy_allocate( real_path_dst, real_path ) ) return false;
	return true;
}

bool pxwFile_make_real_path_master_cmmn( uxDS& real_path_dst, const uxDS& dir_name, const uxDS& file_name )
{
	if( !_path_dir_master_cmmn ) return NULL;
	uxSS<MAX_PATH> real_path = {};
	if( dir_name ) ux_sprintf_s( real_path, "%s\\%s\\%s", *_path_dir_master_cmmn, *dir_name, *file_name );
	else           ux_sprintf_s( real_path, "%s\\%s", *_path_dir_master_cmmn,           *file_name );
	if( !pxStrT_copy_allocate( real_path_dst, real_path ) ) return false;
	return true;
}

bool pxwFile_make_real_path_trns( uxDS& real_path_dst, const uxDS& dir_name, const uxDS& file_name )
{
	if( !_path_dir_trns ) return NULL;
	uxSS<MAX_PATH> real_path = {};
	if( dir_name ) ux_sprintf_s( real_path, "%s\\%s\\%s", *_path_dir_trns, *dir_name, *file_name );
	else           ux_sprintf_s( real_path, "%s\\%s", *_path_dir_trns,           *file_name );
	if( !pxStrT_copy_allocate( real_path_dst, real_path ) ) return false;
	return true;
}
