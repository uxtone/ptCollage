
// '??/??/?? pxwWindowRect.
// '17/10/11 pxFile -> pxFile2

#ifndef pxwWindowRect_H
#define pxwWindowRect_H


#include <pxStdDef.h>

#include <pxFile2.h>

void pxwWindowRect_init  ( const pxFile2* ref_file_profile );
void pxwWindowRect_center( HWND hwnd );
bool pxwWindowRect_load  ( HWND hwnd, const uxDS& name, BOOL bSize, bool b_default_center );
bool pxwWindowRect_save  ( HWND hwnd, const uxDS& name );

#endif
