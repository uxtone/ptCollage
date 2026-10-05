
#include <pxwrDirectory.h>

#include "./pxDirectory.h"

bool pxDirectory_file_find( const uxDS& path_dir, const uxDS& ext, bool b_sub_dir, pxfunc_find_path func, void *user )
{
	return pxwrDirectory_find( path_dir, ext, b_sub_dir, func, user );
}

bool pxDirectory_copy_folders( const uxDS& path_dst, const uxDS& path_src )
{
	return pxwrDirectory_copy_folders( path_dst, path_src );
}
