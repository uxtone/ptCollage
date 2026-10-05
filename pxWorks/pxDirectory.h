// '16/01/29 pxDirectory.

#ifndef pxDirectory_H
#define pxDirectory_H

#include <pxStdDef.h>

bool pxDirectory_file_find   ( const uxDS& path_dir, const uxDS& ext, bool b_sub_dir, pxfunc_find_path func, void *user );
bool pxDirectory_copy_folders( const uxDS& path_dst, const uxDS& path_src );

#endif
