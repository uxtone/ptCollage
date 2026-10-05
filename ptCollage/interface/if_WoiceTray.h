#include <pxStdDef.h>

#include <pxPalette.h>

bool   if_WoiceTray_init           ( const pxPalette* palpng );

void   if_WoiceTray_RedrawName     ( int32_t w );
void   if_WoiceTray_RedrawAllName  ( const uxDS& new_font_name );
void   if_WoiceTray_SetRect        ( const fRECT *rc_view );
fRECT* if_WoiceTray_GetRect        ();
void   if_WoiceTray_Put            ();
bool   if_WoiceTray_GetNamePosition( int32_t w, float *px, float *py );
void   if_WoiceTray_JustScroll     ();
