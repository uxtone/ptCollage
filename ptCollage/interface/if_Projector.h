#ifndef if_Projector_H
#define if_Projector_H

#include <pxStdDef.h>

#include <pxFile2.h>

void    if_Projector_init( HWND hWnd, const pxFile2* file_profile );

bool    if_Projector_SavePosition();
void    if_Projector_SetPosition( float x, float y );
void    if_Projector_GetPosition( float *px, float *py );
void    if_Projector_SetRect    ( const fRECT *rc_view );
fRECT*  if_Projector_GetRect    ();
bool    if_Projector_ShutButton ( float x, float y );
bool    if_Projector_HitButton  ( float x, float y );
int32_t if_Projector_GetFlags   ();
void    if_Projector_SetFlags   ( int32_t flags );
bool    if_Projector_IsOpen     ();
void    if_Projector_SetOpen    ();
void    if_Projector_Put        ();

void    if_Projector_RedrawName ( const TCHAR* new_font_name );
void    if_Projector_SetFocus   ();

#endif
