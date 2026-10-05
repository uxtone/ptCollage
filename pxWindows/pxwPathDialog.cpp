
#include <pxStrT.h>
#include <pxMem.h>

#include "./pxwFilePath.h"
#include "./pxwPathDialog.h"
#include "./pxwDlg_SelFile.h"

static const TCHAR* _dir_name = _T("pathdlg-last");

pxwPathDialog::pxwPathDialog()
{
	_b_init     = false;

	_ref_file_profile = NULL;

	_fltr1		= NULL ;
	_def_dir1	= NULL ;
	_file_name1	= NULL ;
	_exte1		= NULL ;
	_ttl_save1	= NULL ;
	_ttl_load1	= NULL ;

	_lst_dir1	= NULL ;
	_lst_fname1	= NULL ;
}

void pxwPathDialog::_release()
{
	_b_init = false;
	pxStrT_free( &_fltr1      );
	pxStrT_free( &_def_dir1   );
	pxStrT_free( &_file_name1 );
	pxStrT_free( &_exte1      );
	pxStrT_free( &_ttl_save1  );
	pxStrT_free( &_ttl_load1  );

	pxStrT_free( &_lst_dir1   );
	pxStrT_free( &_lst_fname1 );
}

pxwPathDialog::~pxwPathDialog()
{
	_release();
}


bool pxwPathDialog::init(
	pxFile2*     ref_file_profile,
	const TCHAR* filter    ,
	const TCHAR* ext       ,
	const TCHAR* file_name ,
	const TCHAR* title_save,
	const TCHAR* title_load,
	const TCHAR* def_dir   )
{
	if( _b_init ) return false;

	_ref_file_profile = ref_file_profile;

	if( filter )
	{
		int len   = 0;
		int count = 0;

		while( 1 )
		{
			if( !filter[ len ] ){ if( ++count >= 2 ){ len++; break; } }
			else count = 0;
			len++;
		}
		if( !pxMem_zero_alloc( (void**)&_fltr1,         sizeof(TCHAR) * len ) ) goto term;
		memcpy(                         _fltr1, filter, sizeof(TCHAR) * len );
	}


	if( def_dir    && !pxStrT_copy_allocate( &_def_dir1  , def_dir    ) ) goto term;
	if( file_name  && !pxStrT_copy_allocate( &_file_name1, file_name  ) ) goto term;
	if( ext        && !pxStrT_copy_allocate( &_exte1     , ext        ) ) goto term;
	if( title_save && !pxStrT_copy_allocate( &_ttl_save1 , title_save ) ) goto term;
	if( title_load && !pxStrT_copy_allocate( &_ttl_load1 , title_load ) ) goto term;

	if( !pxMem_zero_alloc( (void**)&_lst_dir1  , sizeof(TCHAR) * MAX_PATH ) ) goto term;
	if( !pxMem_zero_alloc( (void**)&_lst_fname1, sizeof(TCHAR) * MAX_PATH ) ) goto term;

	_b_init = true;
term:
	if( !_b_init ) _release  ();
	else           load_lasts();

	return _b_init;
}

bool pxwPathDialog::save_lasts() const
{
	if( !_b_init || !_file_name1) return false;

	bool          b_ret = false;
	pxDescriptor* desc  = NULL ;
	int32_t       size  =      0;

	if( !_ref_file_profile->open_w( &desc, _dir_name, _file_name1, NULL ) ) goto term;

	size = _tcslen( _lst_dir1   ) * sizeof(TCHAR);
	if( !desc->w_asfile( &size          , sizeof(size),    1 ) ) goto term;
	if( !desc->w_asfile(    _lst_dir1   ,            1, size ) ) goto term;

	size = _tcslen( _lst_fname1 ) * sizeof(TCHAR);
	if( !desc->w_asfile( &size          , sizeof(size),    1 ) ) goto term;
	if( !desc->w_asfile( _lst_fname1    ,            1, size ) ) goto term;

	b_ret = true;
term:
	SAFE_DELETE( desc );
	return b_ret;
}


bool pxwPathDialog::load_lasts()
{
	if( !_b_init ) return false;

	if( !pxMem_zero( _lst_dir1  , sizeof(TCHAR) * MAX_PATH ) ) goto term;
	if( !pxMem_zero( _lst_fname1, sizeof(TCHAR) * MAX_PATH ) ) goto term;

	static bool          b_ret;
	b_ret = false;
	static int32_t       size;
	size = 0;
	static pxDescriptor* desc;
	desc = nullptr;

	if( !_ref_file_profile->open_r( &desc, _dir_name, _file_name1, NULL ) ) goto term;

	if( !desc->r( &size      , sizeof(size),    1 )     ) goto term;
	if( !desc->r( _lst_dir1  ,            1, size )     ) goto term;
	if( !desc->r( &size      , sizeof(size),    1 )     ) goto term;
	if( !desc->r( _lst_fname1,            1, size )     ) goto term;

	b_ret = true;
term:
	SAFE_DELETE( desc );
	return b_ret;
}

void pxwPathDialog::_fix_last_directory()
{
	if( !PathIsDirectory( _lst_dir1 ) && !pxwFilePath_IsDrive( _lst_dir1 ) )
	{
		if( PathIsDirectory( _def_dir1 ) ) _tcscpy( _lst_dir1, _def_dir1 );
		else                               pxwFilePath_GetDesktop( _lst_dir1 );
	}
}

bool pxwPathDialog::dialog_save( HWND hWnd, TCHAR* path_dst, const TCHAR* default_filename )
{
	if( !_b_init ) return false;
	if( !path_dst    ) return false;

	bool enable_dst = false;

	if( _tcslen( path_dst ) )
	{
		TCHAR    path_temp[ MAX_PATH ] = { 0 };
		_tcscpy( path_temp, path_dst );
		PathRemoveFileSpec(  path_temp );
		if( PathIsDirectory( path_temp ) ){ enable_dst = true; _tcscpy( _lst_dir1, path_temp ); }
	}

	if( !enable_dst )
	{
		_fix_last_directory();
		_tcscpy( path_dst, _lst_dir1 );
		if( _tcslen( _lst_dir1 ) != 3 ) _tcscat( path_dst, _T("\\") );
		// file name..
		if     ( _tcslen( _lst_fname1 ) > 0 ) _tcscat( path_dst, _lst_fname1      );
		else if( default_filename           ) _tcscat( path_dst, default_filename );
		else                                  _tcscat( path_dst, _T("NoName")     );
	}

	if( !pxwDlg_SelFile_OpenSave( hWnd, path_dst, _lst_dir1, _ttl_save1, _exte1, _fltr1 ) ) return false;
	_tcscpy( _lst_fname1, PathFindFileName( path_dst ) );
	_tcscpy( _lst_dir1  ,                   path_dst ); PathRemoveFileSpec( _lst_dir1 );
	save_lasts();
	return true;
}


bool pxwPathDialog::dialog_load( HWND hWnd, TCHAR* path_dst )
{
	if( !path_dst    ) return false;
	if( !_lst_dir1   ) return false;
	if( !_lst_fname1 ) return false;

	_fix_last_directory();

	memset( path_dst, 0, MAX_PATH * sizeof(TCHAR) );

	if( !pxwDlg_SelFile_OpenLoad( hWnd, path_dst, _lst_dir1, _ttl_load1, _exte1, _fltr1 ) ) return false;
	_tcscpy( _lst_fname1, PathFindFileName( path_dst ) );
	_tcscpy( _lst_dir1  ,                   path_dst ); PathRemoveFileSpec( _lst_dir1 );
	save_lasts();
	return true;
}

bool pxwPathDialog::entrust_save_path( HWND hwnd, bool b_as, TCHAR* path_dst, const TCHAR* default_name )
{
	if( b_as || !get_last_path( path_dst, MAX_PATH ) )
	{
		if( !dialog_save( hwnd, path_dst, default_name ) ) return false;
	}
	return true;
}


bool pxwPathDialog::last_filename_clear()
{
	if( !_lst_fname1 ) return false;
	memset( _lst_fname1, 0, sizeof(TCHAR) * MAX_PATH );
	return true;
}

bool pxwPathDialog::set_loaded_path( const TCHAR* path_src )
{
	if( !path_src    ) return false;
	if( !_lst_dir1   ) return false;
	if( !_lst_fname1 ) return false;
	_tcscpy( _lst_fname1, PathFindFileName( path_src ) );
	_tcscpy( _lst_dir1  ,                   path_src ); PathRemoveFileSpec( _lst_dir1 );
	_fix_last_directory();
	save_lasts();
	return true;
}

bool pxwPathDialog::get_last_path  (       TCHAR* path_dst, int32_t buf_num )
{
	if( !path_dst || buf_num <= 0         ) return false;
	if( !_lst_dir1   || !_lst_dir1  [ 0 ] ) return false;
	if( !_lst_fname1 || !_lst_fname1[ 0 ] ) return false;
	_stprintf_s( path_dst, buf_num, _T("%s\\%s"), _lst_dir1, _lst_fname1 );
	return true;
}

bool pxwPathDialog::last_filename_get( TCHAR *name ) const
{
	if( !name || !_lst_fname1 || !_tcslen( _lst_fname1 ) ) return false;
	_tcscpy( name, _lst_fname1 );
	return true;
}

void         pxwPathDialog::last_filename_set( const TCHAR *name )
{
	if( !name || !_lst_fname1 ) return;
	_tcscpy( _lst_fname1, name );
}

bool pxwPathDialog::extension_get( TCHAR* exte ) const
{
	if( !exte || !_exte1 || !_tcslen( _exte1 ) ) return false;
	_tcscpy( exte, _exte1 );
	return true;
}
