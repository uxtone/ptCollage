
#include <pxPalette.h>

#include <pxtnService.h>

void    if_Player_Initialize( HWND hWnd, const pxPalette* palette );
void    if_Player_Put();
void    if_Player_SavePosition();
void    if_Player_SetPosition( float x,   float y   );
void    if_Player_GetPosition( float *px, float *py );
void    if_Player_SetRect( const fRECT *rc_view );
fRECT*  if_Player_GetRect();

bool    if_Player_IsPlayButton( float x, float y );
int32_t if_Player_GetPlayButtonAnime();
void    if_Player_SetPlayButtonAnime( int32_t no );

bool    if_Player_IsFileButton( float x, float y );
int32_t if_Player_GetFileButtonAnime();
void    if_Player_SetFileButtonAnime( int32_t no );

bool    if_Player_IsCommButton( float x, float y );
int32_t if_Player_GetCommButtonAnime();
void    if_Player_SetCommButtonAnime( int32_t no );

#define IF_PLAYER_STATUS_LOOP 0x01
#define IF_PLAYER_STATUS_SCRL 0x02
#define IF_PLAYER_STATUS_MUTE 0x04

int32_t if_Player_GetStatusFlag();
void    if_Player_PushOneStatusFlag( int32_t flag );
int32_t if_Player_IsStatusButton( float x, float y );

bool    if_Player_Callback_Sampled( void* user, const pxtnService* pxtn );
bool    if_Player_StartDraw ();
void    if_Player_OnMutePlay(); // プロジェクトを読んだり初期化した時
bool    if_Player_StartPlay ();
void    if_Player_StopPlay  ();
void    if_Player_ZeroSampleOffset();

void    if_Player_RedrawName( const TCHAR* new_font_name );

bool    if_Player_IsVolume         ( float x, float y );
void    if_Player_SetVolume_cur_pos( float x, float y );
