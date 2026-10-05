// '16/01/30 pxPath.

#ifndef pxPath_H
#define pxPath_H


#include <pxStdDef.h>

enum pxPathMode
{
	pxPathMode_unknown = 0,
	pxPathMode_UTF8    ,
	pxPathMode_ShiftJIS,
	pxPathMode_UTF16LE ,
	pxPathMode_auto    ,
};

void  pxPath_setMode               ( pxPathMode mode   );

const char* pxPath_find_ext       ( const uxDS& path ); // these three point INTO path (valid while it lives)
const char* pxPath_find_filename  ( const uxDS& path );
bool        pxPath_remove_filename(       uxDS& path ); // cuts the file name off in place; false when there is none
bool        pxPath_remove_filename(       char* path ); // same, for a char buffer
bool        pxPath_remove_ext     (       uxDS& path ); // cuts ".ext" off the file name in place; false when there is none
bool        pxPath_remove_ext     (       char* path );

// like the shell's PathFindFileName / PathFindExtension: never NULL (the whole path / "" when there is nothing to find).
// Views into path, valid while it lives.
inline const char* pxPath_name( const uxDS& path ){ const char* f = pxPath_find_filename( path ); return f ? f : *path; }
inline const char* pxPath_ext ( const uxDS& path ){ const char* e = pxPath_find_ext     ( path ); return e ? e : ""        ; }

#endif
