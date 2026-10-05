// '17/10/02 pxFile2.

#ifndef pxFile2_H
#define pxFile2_H

#include <pxStdDef.h>

#include "./pxDescriptor.h"

#include "./pxLocalize.h"

// "base/option(def)/sub" or "X:\xxx\yyy\sub"

class pxFile2
{
private:
void operator = (const pxFile2& src){}
pxFile2         (const pxFile2& src){}

bool   _b_init          ;

TCHAR* _base_dir_path   ;
TCHAR* _opt_dir         ; // path or dir-name.
//	TCHAR* _opt_def_dir_name;

TCHAR* _opt_file_dir_path ; // directory files.
TCHAR* _opt_file_ext      ;

const pxLocalize* _ref_lclz;

void _release();

public:

pxFile2();
~pxFile2();

bool init_base     ( const TCHAR* dir_name, bool b_create );
bool init_option   ( const TCHAR* opt_files_dir_name, const TCHAR* default_dir_name, const TCHAR* ext );
void set_localize  ( const pxLocalize* ref_localize );

bool get_directories_file_dir( const TCHAR** p_ref_path ) const;
bool get_directories_file_ext( const TCHAR** p_ref_ext  ) const;

bool load_option   ( const char* name, bool b_save_last );

bool make_real_path( TCHAR**  pp_path ,     const TCHAR* dir_name, const TCHAR* data_name, const TCHAR* ext ) const;
bool is_exist      ( bool*    pb_exist,     const TCHAR* dir_name, const TCHAR* data_name, const TCHAR* ext ) const;
bool get_size      ( int32_t* p_size  ,     const TCHAR* dir_name, const TCHAR* data_name, const TCHAR* ext ) const;

bool open_w        ( pxDescriptor** p_desc, const TCHAR* dir_name, const TCHAR* data_name, const TCHAR* ext ) const;
bool open_a        ( pxDescriptor** p_desc, const TCHAR* dir_name, const TCHAR* data_name, const TCHAR* ext ) const;
bool open_r        ( pxDescriptor** p_desc, const TCHAR* dir_name, const TCHAR* data_name, const TCHAR* ext ) const;
bool open_localize ( pxDescriptor** p_desc,                        const TCHAR* data_name, const TCHAR* ext ) const;
};

bool pxFile2_delete  ( const TCHAR *path         );
bool pxFile2_get_size( FILE* fp, int32_t* p_size );


#endif
