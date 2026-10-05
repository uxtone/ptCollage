#include <pxStdDef.h>

#include <pxFile2.h>

void    Woice_init         ( const pxFile2* file_profile );

bool    Woice_Replace      ( int32_t old_pos, int32_t new_pos );
bool    Woice_Add          ( HWND hWnd, const uxDS& path );

int32_t Woice_Dialog_Add   ();
bool    Woice_Dialog_Remove( int w );
void    Woice_Dialog_Edit  ( int w );
void    Woice_Dialog_Change( int w );
bool    Woice_Dialog_Export( int w );
