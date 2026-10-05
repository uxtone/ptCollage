
#include <pxStr.h>

#ifdef pxPLATFORM_windows
#include <pxwDirectory.h>
#elif defined pxSCE
#else
#include <pxpDirectory.h>
#endif

#include "./pxwrDirectory.h"


bool pxwrDirectory_find( const uxDS& path_dir, const uxDS& ext, bool b_sub_dir, pxfunc_find_path func, void *user )
{
#ifdef pxPLATFORM_windows
	return pxwDirectory_find( path_dir, ext, b_sub_dir, func, user );
#elif defined pxSCE
	return false;
#else
	return pxpDirectory_find( path_dir, ext, b_sub_dir, func, user );
#endif
}

bool pxwrDirectory_copy_folders( const uxDS& path_dst, const uxDS& path_src )
{
#ifdef pxPLATFORM_windows
	return pxwDirectory_copy_folders( path_dst, path_src );
#elif defined pxSCE
	return false;
#else
	return false;
#endif
}

bool pxwrDirectory_create( const uxDS& path_dir )
{
#ifdef pxPLATFORM_windows
	return pxwDirectory_create( path_dir );
#else
	return false;
#endif
}
