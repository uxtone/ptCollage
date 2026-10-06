
#include <vector>




#include "./pxwFilePath.h"

#ifdef UNICODE
// UTF-8 -> wide with an explicit length, so the NULs inside "name\0pattern\0...\0\0" survive.
static std::vector<TCHAR> _wide_n( const char* utf8, size_t len )
{
	std::vector<TCHAR> w( 2, 0 );
	if( !utf8 || !len ) return w;
	int n = MultiByteToWideChar( CP_UTF8, 0, utf8, (int)len, NULL, 0 );
	if( n <= 0 ) return w;
	w.assign( (size_t)n + 2, 0 );
	MultiByteToWideChar( CP_UTF8, 0, utf8, (int)len, (LPWSTR)&w[ 0 ], n );
	return w;
}
#endif

// One dialog for both: the Open and the Save dialog take the same OPENFILENAME.
// path is read as the initial file and receives the picked one (UTF-8 both ways).
static bool _select( bool b_save, HWND hWnd, uxDS& path, const uxDS& path_def_dir, const uxDS& title, const uxDS& ext, const uxDS& filter )
{
	// the OS wants TCHAR text that stays valid for the whole call: these live until the end of the function.
	uxT                t_def_dir( path_def_dir );
	uxT                t_title  ( title        );
	uxT                t_ext    ( ext          );
	uxT                t_path   ( path         );
	std::vector<TCHAR> file( MAX_PATH, 0 );
#ifdef UNICODE
	std::vector<TCHAR> t_filter = _wide_n( *filter, filter.size() );
#endif

	if( (const TCHAR*)t_path )
	{
		const TCHAR* p = t_path;
		for( int i = 0; i < MAX_PATH - 1 && p[ i ]; i++ ) file[ i ] = p[ i ];
	}

	OPENFILENAME ofn = {0};

	ofn.lStructSize     = sizeof(OPENFILENAME);
	ofn.hwndOwner       = hWnd;
#ifdef UNICODE
	ofn.lpstrFilter     = filter ? &t_filter[ 0 ] : NULL;       // "wave files {*.wav}\0*.wav\0" "All files {*.*}\0*.*\0\0"
#else
	ofn.lpstrFilter     = *filter;
#endif
	ofn.lpstrFile       = &file[ 0 ];                           // initial file name, receives the result
	ofn.nMaxFile        = MAX_PATH;
	ofn.lpstrFileTitle  = NULL;
	ofn.nMaxFileTitle   = 0;
	ofn.lpstrInitialDir = t_def_dir;                            // NULL: the current directory
	ofn.lpstrTitle      = t_title;
	ofn.lpstrDefExt     = t_ext;                                // appended when the user types no extension

	if( b_save )
	{
		ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;     // no read-only checkbox; ask before replacing
	}
	else
	{
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST; // existing file and path only
	}

	if( !( b_save ? GetSaveFileName( &ofn ) : GetOpenFileName( &ofn ) ) ) return false;

	path = uxDS_from_t( &file[ 0 ] );
	return (bool)path;
}

bool pxwDlg_SelFile_OpenLoad( HWND hWnd, uxDS& path_get, const uxDS& path_def_dir, const uxDS& title, const uxDS& ext, const uxDS& filter )
{
	return _select( false, hWnd, path_get, path_def_dir, title, ext, filter );
}

bool pxwDlg_SelFile_OpenSave( HWND hWnd, uxDS& path_get, const uxDS& path_def_dir, const uxDS& title, const uxDS& ext, const uxDS& filter )
{
	return _select( true , hWnd, path_get, path_def_dir, title, ext, filter );
}
