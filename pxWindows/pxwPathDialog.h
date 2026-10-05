// '16/01/17 pxwPathStore from pxtone-project PathStore.
// '17/01/10 pxwPathStore2 TCHAR[MAX_PATH] and const uxDS  -> uxDS  allocate. on "pxtone projects"
// '17/01/18 pxwPathStore2 -> pxwPathDialog.
// '17/10/11 apply pxFile2.

#ifndef pxwPathDialog_H
#define pxwPathDialog_H

#include <pxStdDef.h>

#include <pxFile2.h>

class pxwPathDialog
{
private:

bool   _b_init    ;

pxFile2* _ref_file_profile;

uxDS  _lst_dir1  ;
uxDS  _lst_fname1;
uxDS  _def_dir1  ;
uxDS  _file_name1;

uxDS  _fltr1     ;
uxDS  _exte1     ;
uxDS  _ttl_save1 ;
uxDS  _ttl_load1 ;

void _fix_last_directory();
void _remember( const uxDS& path ); // last file name + directory of a path the user picked

void _release();

public:

pxwPathDialog();
~pxwPathDialog();

bool init(
	pxFile2*     ref_file_profile,
	const char* filter,     // "name\0pattern\0...\0\0": a raw pointer, because the text contains NULs
	const uxDS& ext   ,
	const uxDS& file_name ,
	const uxDS& title_save,
	const uxDS& title_load,
	const uxDS& def_dir );

bool save_lasts   () const;
bool load_lasts   ();

bool entrust_save_path( HWND hwnd, bool b_as, uxDS& path_dst, const uxDS& default_name );

bool dialog_save  ( HWND hWnd, uxDS& path_dst, const uxDS& default_name );
bool dialog_load  ( HWND hWnd, uxDS& path_dst );

bool get_last_path  (       uxDS& path_dst );
bool set_loaded_path( const uxDS& path_src );

bool last_filename_clear();
bool last_filename_get(       uxDS& name ) const;
void last_filename_set( const uxDS& name );

bool extension_get    (       uxDS& exte ) const;
};

#endif
