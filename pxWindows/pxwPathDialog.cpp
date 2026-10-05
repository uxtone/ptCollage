#include <vector>

#include <pxStrT.h>
#include <uxStr.h>
#include <pxMem.h>
#include <pxPath.h>

#include "./pxwFilePath.h"
#include "./pxwPathDialog.h"
#include "./pxwDlg_SelFile.h"

static const uxSS<13>  _dir_name = "pathdlg-last";

pxwPathDialog::pxwPathDialog()
{
	_b_init           = false;
	_ref_file_profile = NULL ;
	// the uxDS members start unset (NULL)
}

void pxwPathDialog::_release()
{
	_b_init = false;
	pxStrT_free( _fltr1      );
	pxStrT_free( _def_dir1   );
	pxStrT_free( _file_name1 );
	pxStrT_free( _exte1      );
	pxStrT_free( _ttl_save1  );
	pxStrT_free( _ttl_load1  );

	pxStrT_free( _lst_dir1   );
	pxStrT_free( _lst_fname1 );
}

pxwPathDialog::~pxwPathDialog()
{
	_release();
}


bool pxwPathDialog::init(
	pxFile2*     ref_file_profile,
	const char*  filter    ,
	const uxDS& ext       ,
	const uxDS& file_name ,
	const uxDS& title_save,
	const uxDS& title_load,
	const uxDS& def_dir   )
{
	if( _b_init ) return false;

	_ref_file_profile = ref_file_profile;

	if( filter )
	{
		// ends at the second of two NULs in a row; that terminator is part of the value.
		size_t len   = 0;
		int    count = 0;
		while( 1 )
		{
			if( !filter[ len ] ){ if( ++count >= 2 ){ len++; break; } }
			else count = 0;
			len++;
		}
		_fltr1 = uxDS( filter, len );
	}

	if( def_dir    ) _def_dir1   = def_dir   ;
	if( file_name  ) _file_name1 = file_name ;
	if( ext        ) _exte1      = ext       ;
	if( title_save ) _ttl_save1  = title_save;
	if( title_load ) _ttl_load1  = title_load;

	_lst_dir1   = "";
	_lst_fname1 = "";

	_b_init = true;
	load_lasts();

	return _b_init;
}

bool pxwPathDialog::save_lasts() const
{
	if( !_b_init || !_file_name1 ) return false;

	bool          b_ret = false;
	pxDescriptor* desc  = NULL ;

	if( !_ref_file_profile->open_w( &desc, _dir_name, _file_name1, NULL ) ) goto term;

	if( !desc->w_utf8( _lst_dir1   ) ) goto term;
	if( !desc->w_utf8( _lst_fname1 ) ) goto term;

	b_ret = true;
term:
	SAFE_DELETE( desc );
	return b_ret;
}

bool pxwPathDialog::load_lasts()
{
	if( !_b_init ) return false;

	_lst_dir1   = "";
	_lst_fname1 = "";

	bool          b_ret = false;
	pxDescriptor* desc  = NULL ;
	uxDS          dir, fname;

	if( !_ref_file_profile->open_r( &desc, _dir_name, _file_name1, NULL ) ) goto term;

	if( !desc->r_utf8( dir  , 4 * MAX_PATH ) ) goto term; // ignores files from before UTF-8 (wide characters)
	if( !desc->r_utf8( fname, 4 * MAX_PATH ) ) goto term;

	_lst_dir1   = dir  ;
	_lst_fname1 = fname;

	b_ret = true;
term:
	SAFE_DELETE( desc );
	return b_ret;
}

void pxwPathDialog::_fix_last_directory()
{
	if( !PathIsDirectory( uxT( _lst_dir1 ) ) && !pxwFilePath_IsDrive( _lst_dir1 ) )
	{
		if( PathIsDirectory( uxT( _def_dir1 ) ) ) _lst_dir1 = _def_dir1;
		else                                      pxwFilePath_GetDesktop( _lst_dir1 );
	}
}

void pxwPathDialog::_remember( const uxDS& path )
{
	const char* name = pxPath_find_filename( path );
	_lst_fname1 = name ? name : *path;
	_lst_dir1   = path;
	pxPath_remove_filename( _lst_dir1 );
}

bool pxwPathDialog::dialog_save( HWND hWnd, uxDS& path_dst, const uxDS& default_filename )
{
	if( !_b_init ) return false;

	bool enable_dst = false;

	if( path_dst.size() )
	{
		uxDS path_temp = path_dst;
		pxPath_remove_filename( path_temp );
		if( PathIsDirectory( uxT( path_temp ) ) ){ enable_dst = true; _lst_dir1 = path_temp; }
	}

	if( !enable_dst )
	{
		_fix_last_directory();
		path_dst = _lst_dir1;
		if( _lst_dir1.size() != 3 ) path_dst += "\\";
		// file name..
		if     ( _lst_fname1.size() > 0 ) path_dst += _lst_fname1  ;
		else if( default_filename       ) path_dst += default_filename;
		else                              path_dst += "NoName"     ;
	}

	if( !pxwDlg_SelFile_OpenSave( hWnd, path_dst, _lst_dir1, _ttl_save1, _exte1, _fltr1 ) ) return false;
	_remember( path_dst );
	save_lasts();
	return true;
}


bool pxwPathDialog::dialog_load( HWND hWnd, uxDS& path_dst )
{
	if( !_b_init ) return false;

	_fix_last_directory();

	path_dst = "";

	if( !pxwDlg_SelFile_OpenLoad( hWnd, path_dst, _lst_dir1, _ttl_load1, _exte1, _fltr1 ) ) return false;
	_remember( path_dst );
	save_lasts();
	return true;
}

bool pxwPathDialog::entrust_save_path( HWND hwnd, bool b_as, uxDS& path_dst, const uxDS& default_name )
{
	if( b_as || !get_last_path( path_dst ) )
	{
		if( !dialog_save( hwnd, path_dst, default_name ) ) return false;
	}
	return true;
}


bool pxwPathDialog::last_filename_clear()
{
	if( !_b_init ) return false;
	_lst_fname1 = "";
	return true;
}

bool pxwPathDialog::set_loaded_path( const uxDS& path_src )
{
	if( !path_src || !_b_init ) return false;
	_remember( path_src );
	_fix_last_directory();
	save_lasts();
	return true;
}

bool pxwPathDialog::get_last_path( uxDS& path_dst )
{
	if( _lst_dir1.empty() || _lst_fname1.empty() ) return false;
	path_dst = _lst_dir1;
	path_dst += "\\";
	path_dst += _lst_fname1;
	return true;
}

bool pxwPathDialog::last_filename_get( uxDS& name ) const
{
	if( _lst_fname1.empty() ) return false;
	name = _lst_fname1;
	return true;
}

void pxwPathDialog::last_filename_set( const uxDS& name )
{
	if( !name || !_b_init ) return;
	_lst_fname1 = name;
}

bool pxwPathDialog::extension_get( uxDS& exte ) const
{
	if( _exte1.empty() ) return false;
	exte = _exte1;
	return true;
}
