// '15/12/14 pxwAppEtc.h

#include <pxStdDef.h>
int  pxwAppEtc_version    ( int *p1, int *p2, int *p3, int *p4 );
void pxwAppEtc_sleep      ( float sec );
int  pxwAppEtc_Local      ();
bool pxwAppEtc_DisplaySize( int *pw, int *ph );
void pxwAppEtc_OpenFile   ( HWND hwnd, const uxDS& path_src );

void pxwAppEtc_ErrorMessageBox( HWND hwnd, const uxDS& g_app_name );
