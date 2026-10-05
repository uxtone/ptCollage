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

uxDS   _base_dir_path   ;
uxDS   _opt_dir         ; // path or dir-name.
//	uxDS   _opt_def_dir_name;

uxDS   _opt_file_dir_path ; // directory files.
uxDS   _opt_file_ext      ;

const pxLocalize* _ref_lclz;

void _release();

public:

pxFile2();
~pxFile2();

bool init_base     ( const uxDS& dir_name, bool b_create );
bool init_option   ( const uxDS& opt_files_dir_name, const uxDS& default_dir_name, const uxDS& ext );
void set_localize  ( const pxLocalize* ref_localize );

const uxDS& get_directories_file_dir() const { return _opt_file_dir_path; } // a view of the member: valid while this lives
const uxDS& get_directories_file_ext () const { return _opt_file_ext; }

bool load_option   ( const char* name, bool b_save_last );

bool make_real_path( uxDS&  path_dst,       const uxDS& dir_name, const uxDS& data_name, const uxDS& ext ) const;
bool is_exist      ( bool*    pb_exist,     const uxDS& dir_name, const uxDS& data_name, const uxDS& ext ) const;
bool get_size      ( int32_t* p_size  ,     const uxDS& dir_name, const uxDS& data_name, const uxDS& ext ) const;

bool open_w        ( pxDescriptor** p_desc, const uxDS& dir_name, const uxDS& data_name, const uxDS& ext ) const;
bool open_a        ( pxDescriptor** p_desc, const uxDS& dir_name, const uxDS& data_name, const uxDS& ext ) const;
bool open_r        ( pxDescriptor** p_desc, const uxDS& dir_name, const uxDS& data_name, const uxDS& ext ) const;
bool open_localize ( pxDescriptor** p_desc,                      const uxDS& data_name, const uxDS& ext ) const;
};

bool pxFile2_delete  ( const uxDS& path         );
bool pxFile2_get_size( FILE* fp, int32_t* p_size );

#endif
