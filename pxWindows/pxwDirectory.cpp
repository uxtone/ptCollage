// '16/01/29 pxwDirectory.

#include <pxStr.h>
#include <uxStr.h>
#include <pxPath.h>
#include <pxError.h>

#include "./pxwDirectory.h"


bool pxwDirectory_find( const uxDS& path_dir, const uxDS& ext, bool b_sub_dir, pxfunc_find_path func, void *user )
{
	bool            b_ret = false;
	HANDLE          hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA find;
	uxSS<MAX_PATH>           path;

	ux_sprintf_s( path, MAX_PATH, "%s\\*" , *path_dir      );
	if( ( hFind = FindFirstFile( uxT( path ), &find ) ) == INVALID_HANDLE_VALUE ) return false;

	do
	{
		uxDS name = uxDS_from_t( find.cFileName ); // UTF-8
		if( name != "." && name != ".." )
		{
			ux_sprintf_s( path, MAX_PATH, "%s\\%s", *path_dir, *name );
			if( PathIsDirectory( uxT( path ) ) )
			{
				if( b_sub_dir && !pxwDirectory_find( path, ext, b_sub_dir, func, user ) ) goto End;
			}
			else
			{
				const char* p_ext = pxPath_find_ext( name );
				if( p_ext && !ux_stricmp( ext, p_ext ) )
				{
					if( !func( user, path ) ) goto End;
				}
			}
		}
	}
	while( FindNextFile( hFind, &find ) );

	b_ret = true;
End:
	if( hFind != INVALID_HANDLE_VALUE ) FindClose( hFind );
	return b_ret;
}

bool pxwDirectory_copy_folders( const uxDS& path_dst, const uxDS& path_src )
{
	bool            b_ret = false;
	HANDLE          hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA find;
	uxSS<MAX_PATH>           path_s;
	uxSS<MAX_PATH>           path_d;

	ux_sprintf_s( path_s, MAX_PATH, "%s\\*"   , *path_src      );
	if( ( hFind = FindFirstFile( uxT( path_s ), &find ) ) == INVALID_HANDLE_VALUE ) return false;

	if( !PathIsDirectory( uxT( path_dst ) ) && !CreateDirectory( uxT( path_dst ), NULL ) ) goto term;

	do
	{
		uxDS name = uxDS_from_t( find.cFileName ); // UTF-8
		if( name != "." && name != ".." )
		{
			ux_sprintf_s( path_s, MAX_PATH, "%s\\%s", *path_src, *name );
			if( PathIsDirectory( uxT( path_s ) ) )
			{
				ux_sprintf_s( path_d, MAX_PATH, "%s\\%s", *path_dst, *name );

				if( !pxwDirectory_copy_folders( path_d, path_s ) ) goto term;
			}
		}
	}
	while( FindNextFile( hFind, &find ) );

	b_ret = true;
term:
	if( hFind != INVALID_HANDLE_VALUE ) FindClose( hFind );
	return b_ret;
}

bool pxwDirectory_remove( const uxDS& dir_path )
{
	bool            b_ret = false;
	HANDLE          hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA find;
	uxSS<MAX_PATH>           path;

	if( !PathIsDirectory( uxT( dir_path ) ) ) return false;

	ux_sprintf_s( path, MAX_PATH, "%s\\*", *dir_path );

	if( ( hFind = FindFirstFile( uxT( path ), &find ) ) != INVALID_HANDLE_VALUE )
	{
		do
		{
			uxDS name = uxDS_from_t( find.cFileName ); // UTF-8
			if( name != "." && name != ".." )
			{
				ux_sprintf_s( path, MAX_PATH, "%s\\%s", *dir_path, *name );
				if( PathIsDirectory( uxT( path ) ) )
				{
					if( !pxwDirectory_remove( path ) ){ pxerr_t( "rmv:", path ); goto term; }
				}
				else
				{
					if( !DeleteFile         ( uxT( path ) ) ){ pxerr_t( "rmv:", path ); goto term; }
				}
			}
		}
		while( FindNextFile( hFind, &find ) );
	}

	if( !RemoveDirectory( uxT( dir_path ) ) ){ pxerr_t( "rmv:", dir_path ); goto term; }

	b_ret = true;
term:
	if( hFind != INVALID_HANDLE_VALUE ) FindClose( hFind );
	return b_ret;
}

bool pxwDirectory_create( const uxDS& dir_path )
{
	if(  PathIsDirectory( uxT( dir_path )       ) ) return true;
	if( !CreateDirectory( uxT( dir_path ), NULL ) ) return false;
	return true;
}


bool pxwDirectory_rename( const uxDS& path_orginal, const uxDS& path_new, bool b_remove_exist )
{
	if( PathFileExists ( uxT( path_new ) ) )
	{
		if( !b_remove_exist ) return false;

		if( PathIsDirectory( uxT( path_new ) ) )
		{
			if( !pxwDirectory_remove( path_new ) ) return false;
		}
		else
		{
			if( !DeleteFile( uxT( path_new ) ) ) return false;
		}
	}

	if( !MoveFile( uxT( path_orginal ), uxT( path_new ) ) ) return false;

	return true;

}
