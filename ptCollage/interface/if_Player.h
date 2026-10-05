#ifndef if_Player_H
#define if_Player_H

#include <pxStdDef.h>

#include <pxtnService.h>

#include <pxFile2.h>

void    if_Player_init( HWND hWnd, const pxFile2* file_profile );
void    if_Player_Put();
bool    if_Player_SavePosition();
void    if_Player_SetPosition( float x,   float y   );
void    if_Player_GetPosition( float *px, float *py );
void    if_Player_SetRect( const fRECT *rc_view );
fRECT*  if_Player_GetRect();

bool    if_Player_IsPlayButton( float x, float y );
bool    if_Player_IsVolume    ( float x, float y );

int32_t if_Player_GetPlayButtonAnime();
void    if_Player_SetPlayButtonAnime( int32_t no );

bool    if_Player_flag_is_check_mute();
void    if_Player_PushOneStatusFlag( int32_t flag );
bool    if_Player_CheckButton( float x, float y );

bool    if_Player_IsOpen(  void );
void    if_Player_SetOpen();

bool    if_Player_Callback_Sampled( void* user, const pxtnService* pxtn );

void    if_Player_OnMutePlay(); // プロジェクトを読んだり初期化した時
bool    if_Player_StartPlay ();
void    if_Player_StopPlay  ();
void    if_Player_SetFocus  ();

void    if_Player_SetVolume_cur_pos( float x, float y );

#endif
