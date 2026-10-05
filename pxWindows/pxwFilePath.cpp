
#include "./pxwFilePath.h"

#include <pxStdDef.h>

#include <pxStrT.h>

static bool _IsShiftJIS( unsigned char c )
{
	if( c >= 0x81 && c <= 0x9F ) return true;
	if( c >= 0xE0 && c <= 0xEF ) return true;

	return false;
}

void pxwFilePath_ncomp_x_sjis( char *name )
{
	int32_t i = 0;
	while( name[ i ] != '\0' )
	{
		if( _IsShiftJIS( name[ i ] ) )
		{
			i++;
		}
		else if(
			name[ i ] == '\\' ||
			name[ i ] == '/'  ||
			name[ i ] == ':'  ||
			name[ i ] == ','  ||
			name[ i ] == ';'  ||
			name[ i ] == '*'  ||
			name[ i ] == '?'  ||
			name[ i ] == '\"' ||
			name[ i ] == '<'  ||
			name[ i ] == '>'  ||
			name[ i ] == '|'  ) name[ i ] = 'x';

		i++;
	}
}



bool pxwFilePath_ArgToPath( const TCHAR* arg, TCHAR* path_dst )
{
	const TCHAR* p_src  = NULL;
	TCHAR*       p_dst  = path_dst;

	memset( path_dst, 0, MAX_PATH * sizeof(TCHAR) );

	p_src = arg; if( *p_src == '"' ) p_src++;

// unicode
#ifdef UNICODE
	while( *p_src )
	{
		if( *p_src == '"' ) break;
		*p_dst = *p_src;
		p_dst++; p_src++;
	}
#else
// sjis
	while( *p_src )
	{
		if( *p_src == '"' ) break;
		if( _IsShiftJIS( *p_src ) ){ *p_dst = *p_src; p_dst++; p_src++; }
		{                            *p_dst = *p_src; p_dst++; p_src++; }
	}
#endif

	if( !_tcslen( path_dst ) ) return false;

	return true;
}

#pragma comment(lib, "shell32")
#include <shlobj.h>

void pxwFilePath_GetSpecial( TCHAR *path, SPECIALPATH special )
{
	ITEMIDLIST*    p_item  ;
	IMalloc*       p_malloc;
	unsigned short csidl   ;

	switch( special )
	{
	case SPECIALPATH_DESKTOP   : csidl = CSIDL_DESKTOP ; break;
	case SPECIALPATH_MYDOCUMENT: csidl = CSIDL_PERSONAL; break;
	case SPECIALPATH_MYCOMPUTER: csidl = CSIDL_DRIVES  ; break;
	}

	if( SUCCEEDED( SHGetMalloc( &p_malloc ) ) )
	{
		SHGetSpecialFolderLocation( GetDesktopWindow(), csidl, &p_item );
		SHGetPathFromIDList( p_item, path );
		p_malloc->Free(      p_item );
		p_malloc->Release();
	}
}

void pxwFilePath_GetDesktop( TCHAR *path )
{
	pxwFilePath_GetSpecial( path, SPECIALPATH_DESKTOP );
}

bool pxwFilePath_GetShortcutDirectory( const TCHAR* path_lnk, TCHAR* path_dst )
{
	bool            b_ret = false;
	IShellLink*     psl   = NULL ;  // IShellLinkへのポインタ
	IPersistFile*   ppf   = NULL ;  // IPersistFile へのポインタ
	WIN32_FIND_DATA wfd   = { 0 };

	wchar_t         path_unicode[ MAX_PATH ] = {0}; // Unicode 文字列へのバッファ

	if( CoCreateInstance   ( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl ) ) goto End; // get IShellLink.
	if( psl->QueryInterface( IID_IPersistFile,                                            (void**)&ppf ) ) goto End; // ask IPersistFile.

#ifdef UNICODE
	_tcscpy( path_unicode, path_lnk );
#else
	MultiByteToWideChar( CP_ACP, 0, path_lnk, -1, (LPWSTR)path_unicode, MAX_PATH );
#endif

	// ショートカットをロードする
	if( ppf->Load( (LPCOLESTR)path_unicode, STGM_READ )            ) goto End;

	// リンク先を取得する
	if( psl->GetPath( path_dst, MAX_PATH, &wfd, SLGP_UNCPRIORITY ) ) goto End;

	// ディレクトリかどうか
	if( !( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )       ) goto End;

	b_ret = true;
End:
	if( ppf ) ppf->Release();
	if( psl ) psl->Release();

	return b_ret;
}

bool pxwFilePath_IsDrive( const TCHAR* path )
{
	if( !path ) return false;
	// "c:\\"
	if( _tcslen( path ) == 3 && path[ 1 ] == ':' )
	{
		if( path[ 2 ] == '/' ||  path[ 2 ] == '\\'  ) return true;
	}
	return false;
}

void pxwFilePath_GetModuleDirectory( TCHAR* path )
{
	GetModuleFileName ( NULL, path, MAX_PATH );
	PathRemoveFileSpec(       path           );
	if( pxwFilePath_IsDrive( path ) ) path[ 2 ] = '\0';
}

bool pxwFilePath_MakeFolderPath( TCHAR **p_path_dst, const TCHAR *dir_name, bool b_create )
{
	if( !p_path_dst ) return false;

	TCHAR path_module[ MAX_PATH ] = {};
	TCHAR path_dst   [ MAX_PATH ] = {};

	GetModuleFileName ( NULL, path_module, MAX_PATH );
	PathRemoveFileSpec(       path_module );
	if( dir_name ) _stprintf_s( path_dst, MAX_PATH, _T("%s\\%s"), path_module, dir_name );
	else           _stprintf_s( path_dst, MAX_PATH, _T("%s"    ), path_module           );

	if( !pxStrT_copy_allocate( p_path_dst, path_dst ) ) return false;

	if( b_create ) CreateDirectory( path_dst, NULL );
	if( !PathIsDirectory( path_dst ) ){ pxStrT_free( p_path_dst ); return false; }

	return true;
}

