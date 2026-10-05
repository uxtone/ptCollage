#ifndef if_Copier_H
#define if_Copier_H

#include <pxStdDef.h>

#include <pxFile2.h>

#define COPIERFLAG_EVENT_NUM            9

#define COPIERFLAG_EVENT_ON         0x001
#define COPIERFLAG_EVENT_KEY        0x002
#define COPIERFLAG_EVENT_PAN_VOLUME 0x004
#define COPIERFLAG_EVENT_PAN_TIME   0x008
#define COPIERFLAG_EVENT_VOLUME     0x010
#define COPIERFLAG_EVENT_PORTAMENT  0x020
#define COPIERFLAG_EVENT_VOICENO    0x040
#define COPIERFLAG_EVENT_GROUPNO    0x080
#define COPIERFLAG_EVENT_TUNING     0x100

#define COPIERFLAG_STATUS_OPEN     0x1000


void    if_Copier_AllCheck     ();

void    if_Copier_init         ( HWND hWnd, const pxFile2* file_profile );
bool    if_Copier_SavePosition ();
void    if_Copier_SetPosition  ( float x,   float y   );
void    if_Copier_GetPosition  ( float *px, float *py );
void    if_Copier_SetRect      ( const fRECT *rc_view );
fRECT*  if_Copier_GetRect      ();
bool    if_Copier_CheckButton  ( float x, float y, bool* p_bON );
bool    if_Copier_CheckButton  ( float x, float y, bool    bON );
bool    if_Copier_ShutButton   ( float x, float y );
void    if_Copier_Put          ();

int32_t if_Copier_GetFlags     ();
void    if_Copier_SetFlags     ( int32_t flags );
bool    if_Copier_IsOpen       ();
void    if_Copier_SetOpen      ();


bool*   if_Copier_GetEventKinds();

void    if_Copier_SetFocus     ();

#endif
