// '16/01/29 pxwDirectory.
// '17/10/04 + pxwDirectory_remove, pxwDirectory_rename

#ifndef pxwDirectory_H
#define pxwSirectory_H

#include <pxStdDef.h>

bool pxwDirectory_find        ( const uxDS& path_dir, const uxDS& ext, bool b_sub_dir, pxfunc_find_path func, void *user );
bool pxwDirectory_copy_folders( const uxDS& path_dst, const uxDS& path_src );

bool pxwDirectory_remove      ( const uxDS& dir_path );
bool pxwDirectory_create      ( const uxDS& dir_path );
bool pxwDirectory_rename      ( const uxDS& path_orginal, const uxDS& path_new, bool b_remove_exist );

#endif
