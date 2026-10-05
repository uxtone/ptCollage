// '16/01/29 pxwrDirectory.
// '16/09/07 + pxwrDirectory_copy_folders()

#ifndef pxwrDirectory_H
#define pxwrDirectory_H

#include <pxStdDef.h>

bool pxwrDirectory_find        ( const uxDS& path_dir, const uxDS& ext, bool b_sub_dir, pxfunc_find_path func, void *user );
bool pxwrDirectory_copy_folders( const uxDS& path_dst, const uxDS& path_src );
bool pxwrDirectory_create      ( const uxDS& path_dir );

#endif
