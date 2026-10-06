
#include "./pxwFilePath.h"


#include <pxPath.h>

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



bool pxwFilePath_ArgToPath( const uxDS& arg, uxDS& path_dst )
{
	path_dst = "";
	if( !arg ) return false;

	// the argument may be quoted: "C:\\a path\\file.ptcop"
	const char* p = arg;
	if( *p == '"' ) p++;
	const char* q = p;
	while( *q && *q != '"' ) q++;

	path_dst = uxDS( p, (size_t)( q - p ) );
	return !path_dst.empty();
}

#pragma comment(lib, "shell32")
#include <shlobj.h>

void pxwFilePath_GetSpecial( uxDS& path, SPECIALPATH special )
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
		SHGetPathFromIDList( p_item, uxTOut( path, MAX_PATH ) );
		p_malloc->Free(      p_item );
		p_malloc->Release();
	}
}

void pxwFilePath_GetDesktop( uxDS& path )
{
	pxwFilePath_GetSpecial( path, SPECIALPATH_DESKTOP );
}

bool pxwFilePath_GetShortcutDirectory( const uxDS& path_lnk, uxDS& path_dst )
{
	bool            b_ret = false;
	IShellLink*     psl   = NULL ;  // IShellLinkへのポインタ
	IPersistFile*   ppf   = NULL ;  // IPersistFile へのポインタ
	WIN32_FIND_DATA wfd   = { 0 };

	wchar_t         path_unicode[ MAX_PATH ] = {0}; // Unicode 文字列へのバッファ

	if( CoCreateInstance   ( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl ) ) goto End; // get IShellLink.
	if( psl->QueryInterface( IID_IPersistFile,                                            (void**)&ppf ) ) goto End; // ask IPersistFile.

	MultiByteToWideChar( CP_UTF8, 0, *path_lnk, -1, (LPWSTR)path_unicode, MAX_PATH ); // path_lnk is UTF-8

	// ショートカットをロードする
	if( ppf->Load( (LPCOLESTR)path_unicode, STGM_READ )            ) goto End;

	// リンク先を取得する
	if( psl->GetPath( uxTOut( path_dst, MAX_PATH ), MAX_PATH, &wfd, SLGP_UNCPRIORITY ) ) goto End;

	// ディレクトリかどうか
	if( !( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )       ) goto End;

	b_ret = true;
End:
	if( ppf ) ppf->Release();
	if( psl ) psl->Release();

	return b_ret;
}

bool pxwFilePath_IsDrive( const uxDS& path )
{
	if( !path ) return false;
	// "c:\\"
	if( path.size() == 3 && path[ 1 ] == ':' )
	{
		if( path[ 2 ] == '/' ||  path[ 2 ] == '\\'  ) return true;
	}
	return false;
}

void pxwFilePath_GetModuleDirectory( uxDS& path )
{
	GetModuleFileName ( NULL, uxTOut( path, MAX_PATH ), MAX_PATH );
	pxPath_remove_filename( path );
	if( pxwFilePath_IsDrive( path ) ) path.truncate( 2 );
}

bool pxwFilePath_MakeFolderPath( uxDS& out_path, const uxDS& dir_name, bool b_create )
{

	uxSS<MAX_PATH> path_module = {};
	uxSS<MAX_PATH> path_dst = {};

	GetModuleFileName ( NULL, uxTOut( path_module ), MAX_PATH );
	pxPath_remove_filename( path_module );
	if( dir_name ) ux_sprintf_s( path_dst, MAX_PATH, "%s\\%s", path_module, *dir_name );
	else           ux_sprintf_s( path_dst, MAX_PATH, "%s", path_module           );

	if( !pxStrT_copy_allocate( out_path, path_dst ) ) return false;

	if( b_create ) CreateDirectory( uxT( path_dst ), NULL );
	if( !PathIsDirectory( uxT( path_dst ) ) ){ pxStrT_free( out_path ); return false; }

	return true;
}
