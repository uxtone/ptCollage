#ifndef pxwFilePath_H
#define pxwFilePath_H

#include <pxStdDef.h>

enum SPECIALPATH
{
	SPECIALPATH_DESKTOP = 0,
	SPECIALPATH_MYDOCUMENT ,
	SPECIALPATH_MYCOMPUTER ,
};

void pxwFilePath_ncomp_x_sjis        ( char *name );

bool pxwFilePath_ArgToPath           ( const uxDS& arg, uxDS& path_dst );

void pxwFilePath_GetSpecial          (       uxDS& path, SPECIALPATH special );
void pxwFilePath_GetDesktop          (       uxDS& path );

bool pxwFilePath_GetShortcutDirectory( const uxDS& path_lnk, uxDS& path_dst );
bool pxwFilePath_IsDrive             ( const uxDS& path );
void pxwFilePath_GetModuleDirectory  (       uxDS& path );

bool pxwFilePath_MakeFolderPath      (       uxDS& out_path, const uxDS& name, bool b_create );

#endif
