#include <pxStdDef.h>

void   if_UnitTray_RedrawAllName( const uxDS& new_font_name );
void   if_UnitTray_Put          ();
void   if_UnitTray_JustScroll   ();

void   if_UnitTray_SetRect( const fRECT *rc_view );
fRECT* if_UnitTray_GetRect();
bool   if_UnitTray_GetNamePosition( int32_t u, float *px, float *py );

void   if_UnitTray_LumpSelect     ( int32_t start_u, int32_t end_u );
