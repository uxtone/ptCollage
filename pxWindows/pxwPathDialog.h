// '16/01/17 pxwPathStore from pxtone-project PathStore.
// '17/01/10 pxwPathStore2 TCHAR[MAX_PATH] and const TCHAR* -> TCHAR* allocate. on "pxtone projects"
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

TCHAR* _lst_dir1  ;
TCHAR* _lst_fname1;
TCHAR* _def_dir1  ;
TCHAR* _file_name1;

TCHAR* _fltr1     ;
TCHAR* _exte1     ;
TCHAR* _ttl_save1 ;
TCHAR* _ttl_load1 ;

void _fix_last_directory();

void _release();

public:

pxwPathDialog();
~pxwPathDialog();

bool init(
	pxFile2*     ref_file_profile,
	const TCHAR* filter,
	const TCHAR* ext   ,
	const TCHAR* file_name ,
	const TCHAR* title_save,
	const TCHAR* title_load,
	const TCHAR* def_dir );

bool save_lasts   () const;
bool load_lasts   ();

bool entrust_save_path( HWND hwnd, bool b_as, TCHAR* path_dst, const TCHAR* default_name );

bool dialog_save  ( HWND hWnd, TCHAR* path_dst, const TCHAR* default_name );
bool dialog_load  ( HWND hWnd, TCHAR* path_dst );

bool get_last_path  (       TCHAR* path_dst, int32_t buf_num );
bool set_loaded_path( const TCHAR* path_src );

bool last_filename_clear();
bool last_filename_get(       TCHAR *name ) const;
void last_filename_set( const TCHAR *name );

bool extension_get    (       TCHAR* exte ) const;
};

#endif
