// '16/01/29 pxwDirectory.

#include <pxStr.h>
#include <pxPath.h>
#include <pxError.h>

#include "./pxwDirectory.h"


bool pxwDirectory_find( const TCHAR *path_dir, const TCHAR *ext, bool b_sub_dir, pxfunc_find_path func, void *user )
{
	bool            b_ret = false;
	HANDLE          hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA find;
	TCHAR           path[ MAX_PATH ];

	_stprintf_s( path, MAX_PATH, _T("%s\\*") , path_dir      );
	if( ( hFind = FindFirstFile( path, &find ) ) == INVALID_HANDLE_VALUE ) return false;

	do
	{
		if( _tcscmp( find.cFileName, _T(".") ) && _tcscmp( find.cFileName, _T("..") ) )
		{
			_stprintf_s( path, MAX_PATH, _T("%s\\%s"), path_dir, find.cFileName );
			if( PathIsDirectory( path ) )
			{
				if( b_sub_dir && !pxwDirectory_find( path, ext, b_sub_dir, func, user ) ) goto End;
			}
			else
			{
				const TCHAR* p_ext = pxPath_find_ext( find.cFileName );
				if( p_ext && !_tcsicmp( ext, p_ext ) )
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

bool pxwDirectory_copy_folders( const TCHAR *path_dst, const TCHAR* path_src )
{
	bool            b_ret = false;
	HANDLE          hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA find;
	TCHAR           path_s[ MAX_PATH ];
	TCHAR           path_d[ MAX_PATH ];

	_stprintf_s( path_s, MAX_PATH, _T("%s\\*")   , path_src      );
	if( ( hFind = FindFirstFile( path_s, &find ) ) == INVALID_HANDLE_VALUE ) return false;

	if( !PathIsDirectory( path_dst ) && !CreateDirectory( path_dst, NULL ) ) goto term;

	do
	{
		if( _tcscmp( find.cFileName, _T(".") ) && _tcscmp( find.cFileName, _T("..") ) )
		{
			_stprintf_s( path_s, MAX_PATH, _T("%s\\%s"), path_src, find.cFileName );
			if( PathIsDirectory( path_s ) )
			{
				_stprintf_s( path_d, MAX_PATH, _T("%s\\%s"), path_dst, find.cFileName );

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

bool pxwDirectory_remove( const TCHAR* dir_path )
{
	bool            b_ret = false;
	HANDLE          hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA find;
	TCHAR           path[ MAX_PATH ];

	if( !PathIsDirectory( dir_path ) ) return false;

	_stprintf_s( path, MAX_PATH, _T("%s\\*"), dir_path );

	if( ( hFind = FindFirstFile( path, &find ) ) != INVALID_HANDLE_VALUE )
	{
		do
		{
			if( _tcscmp( find.cFileName, _T(".") ) && _tcscmp( find.cFileName, _T("..") ) )
			{
				_stprintf_s( path, MAX_PATH, _T("%s\\%s"), dir_path, find.cFileName );
				if( PathIsDirectory( path ) )
				{
					if( !pxwDirectory_remove( path ) ){ pxerr_t( "rmv:", path ); goto term; }
				}
				else
				{
					if( !DeleteFile         ( path ) ){ pxerr_t( "rmv:", path ); goto term; }
				}
			}
		}
		while( FindNextFile( hFind, &find ) );
	}

	if( !RemoveDirectory( dir_path ) ){ pxerr_t( "rmv:", dir_path ); goto term; }

	b_ret = true;
term:
	if( hFind != INVALID_HANDLE_VALUE ) FindClose( hFind );
	return b_ret;
}

bool pxwDirectory_create( const TCHAR* dir_path )
{
	if(  PathIsDirectory( dir_path       ) ) return true;
	if( !CreateDirectory( dir_path, NULL ) ) return false;
	return true;
}


bool pxwDirectory_rename( const TCHAR *path_orginal, const TCHAR* path_new, bool b_remove_exist )
{
	if( PathFileExists ( path_new ) )
	{
		if( !b_remove_exist ) return false;

		if( PathIsDirectory( path_new ) )
		{
			if( !pxwDirectory_remove( path_new ) ) return false;
		}
		else
		{
			if( !DeleteFile( path_new ) ) return false;
		}
	}

	if( !MoveFile( path_orginal, path_new ) ) return false;

	return true;

}
