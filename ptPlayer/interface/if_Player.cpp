
#include <pxwDx09Draw.h>
extern pxwDx09Draw*     g_dxdraw  ;

#include <pxtonewinXA2.h>
extern pxtonewinXA2*    g_strm_xa2;

#include <pxtnService.h>
extern pxtnService*     g_pxtn    ;

#include "Interface.h"
#include "SurfaceNo.h"
#include "if_Cursor.h"
#include "if_Player.h"

#define BUTTON_PLAY_X 112
#define BUTTON_PLAY_Y  64
#define BUTTON_FILE_X  80
#define BUTTON_FILE_Y  64

#define BUTTON_COMM_X 128
#define BUTTON_COMM_Y  12

#define BUTTON_LOOP_X  24
#define BUTTON_LOOP_Y  66

#define PROGRESS_WIDTH  126
#define PROGRESS_HEIGHT   6
#define PROGRESS_X       17
#define PROGRESS_Y       35

#define BUTTON_VOLUME_X  64
#define BUTTON_VOLUME_Y  48
#define BUTTON_VOLUME_W  80
#define BUTTON_VOLUME_H   8


typedef struct
{
	float   x, y;
	int32_t play_button_anime;
	int32_t file_button_anime;
	int32_t comm_button_anime;
	int32_t status_flag;
	fRECT   progress_rect;
	float   volume;
}
PLAYERSTRUCT;

static PLAYERSTRUCT _player        = {0};
static fRECT        _rcPanel       = {0};
static int32_t      _sample_offset =  0 ;

#define PROGRESS_a 1.0f
#define PROGRESS_r 0.0f
#define PROGRESS_g ( (float)0xf0 / 0xff )
#define PROGRESS_b ( (float)0x80 / 0xff )

void if_Player_Initialize( HWND hWnd, const pxPalette* palette )
{
	memset( &_player, 0, sizeof(PLAYERSTRUCT) );
	_player.x = 0;
	_player.y = 0;
	_player.status_flag = IF_PLAYER_STATUS_LOOP;
	_player.volume      = 1;
}

static const char *_default_name = "-PXTONE PLAYER-";

void if_Player_RedrawName( const TCHAR* new_font_name )
{
	const char* p_name;
	int32_t     name_size = 0;
	fRECT rc = { 3,3,TUNENAME_WIDTH, TUNENAME_HEIGHT };

	p_name = g_pxtn->text->get_name_buf( &name_size );
	if( !p_name ) p_name = _default_name;

	if( new_font_name ) g_dxdraw->tex_glyph_another_font( SURF_TUNENAME, new_font_name, 12, false );

	g_dxdraw->tex_glyph_clear( SURF_TUNENAME );
	g_dxdraw->tex_Clear      ( SURF_TUNENAME );
	g_dxdraw->tex_glyph_text ( SURF_TUNENAME, p_name, &rc, 1 );
}

void if_Player_SetRect( const fRECT *rc_view )
{
	_rcPanel = *rc_view;
}

fRECT* if_Player_GetRect()
{
	return &_rcPanel;
}

bool if_Player_IsPlayButton(    float x, float y )
{
	if( x >= _player.x + BUTTON_PLAY_X && x <  _player.x + BUTTON_PLAY_X + 32 &&
		y >= _player.y + BUTTON_PLAY_Y && y <  _player.y + BUTTON_PLAY_Y + 16 ) return true;
	return false;
}

bool if_Player_IsFileButton(    float x, float y )
{
	if( x >= _player.x + BUTTON_FILE_X && x <  _player.x + BUTTON_FILE_X + 32 &&
		y >= _player.y + BUTTON_FILE_Y && y <  _player.y + BUTTON_FILE_Y + 16 ) return true;
	return false;
}

bool if_Player_IsCommButton( float x, float y )
{
	if( x >= _player.x + BUTTON_COMM_X && x <  _player.x + BUTTON_COMM_X + 16 &&
		y >= _player.y + BUTTON_COMM_Y && y <  _player.y + BUTTON_COMM_Y + 16 ) return true;
	return false;
}

bool if_Player_IsVolume( float x, float y )
{
	if( x >= _player.x + BUTTON_VOLUME_X && x <  _player.x + BUTTON_VOLUME_X + BUTTON_VOLUME_W &&
		y >= _player.y + BUTTON_VOLUME_Y && y <  _player.y + BUTTON_VOLUME_Y + BUTTON_VOLUME_H ) return true;
	return false;
}



int32_t if_Player_IsStatusButton( float x, float y )
{
	int32_t flag = 0;

	if( x >= _player.x + BUTTON_LOOP_X && x <  _player.x + BUTTON_LOOP_X + 16 &&
		y >= _player.y + BUTTON_LOOP_Y && y <  _player.y + BUTTON_LOOP_Y + 12 ) flag |= IF_PLAYER_STATUS_LOOP;

	return flag;
}

int32_t if_Player_GetStatusFlag()
{
	return _player.status_flag;
}

int32_t if_Player_GetPlayButtonAnime(            ){ return _player.play_button_anime;      }
void    if_Player_SetPlayButtonAnime( int32_t no ){        _player.play_button_anime = no; }
int32_t if_Player_GetFileButtonAnime(            ){ return _player.file_button_anime;      }
void    if_Player_SetFileButtonAnime( int32_t no ){        _player.file_button_anime = no; }
int32_t if_Player_GetCommButtonAnime(            ){ return _player.comm_button_anime;      }
void    if_Player_SetCommButtonAnime( int32_t no ){        _player.comm_button_anime = no; }

void if_Player_Put()
{
	fRECT rcFrame         = {   0, 0,160,88 };
	fRECT rcLoopButton    = {288,  0,304, 16};
	fRECT rcPlayButton[]  =
	{
		{160,  0,192, 16},
		{192,  0,224, 16},
		{224,  0,256, 16},
		{256,  0,288, 16},
	};
	fRECT rcFileButton[] =
	{
		{160, 16,192, 32},
		{192, 16,224, 32},
	};
	fRECT rcCommButton[] =
	{
		{304,  0,320, 16},
		{304, 16,320, 32},
	};
	fRECT rcVolume = {240, 96,  0,104};
	fRECT rcVoltab = {296, 20,304, 32};


	{
		g_dxdraw->tex_Put_View( _player.x, _player.y, &rcFrame, SURF_PLAYER );
		g_dxdraw->tex_Put_View( _player.x + BUTTON_PLAY_X, _player.y + BUTTON_PLAY_Y, &rcPlayButton[ _player.play_button_anime ], SURF_PLAYER );
		g_dxdraw->tex_Put_View( _player.x + BUTTON_FILE_X, _player.y + BUTTON_FILE_Y, &rcFileButton[ _player.file_button_anime ], SURF_PLAYER );
		if( g_pxtn->text->is_comment_buf()              ) g_dxdraw->tex_Put_View( _player.x + BUTTON_COMM_X, _player.y + BUTTON_COMM_Y, &rcCommButton[ _player.comm_button_anime ], SURF_PLAYER );
		if( _player.status_flag & IF_PLAYER_STATUS_LOOP ) g_dxdraw->tex_Put_View( _player.x + BUTTON_LOOP_X, _player.y + BUTTON_LOOP_Y, &rcLoopButton                             , SURF_PLAYER );
	}

	g_dxdraw->FillRect_clip( &_player.progress_rect, PROGRESS_a, PROGRESS_r, PROGRESS_g, PROGRESS_b, &_rcPanel );

	{
		fRECT rcName = {0,0,TUNENAME_WIDTH, TUNENAME_HEIGHT};
		g_dxdraw->tex_Put_View( _player.x + 16, _player.y + 12, &rcName, SURF_TUNENAME );

		int v = (int)( _player.volume * BUTTON_VOLUME_W );
		rcVolume.r = rcVolume.l + v;
		g_dxdraw->tex_Put_View( _player.x + BUTTON_VOLUME_X, _player.y + BUTTON_VOLUME_Y, &rcVolume, SURF_PLAYER );
		g_dxdraw->tex_Put_View( _player.x + BUTTON_VOLUME_X + v - 2, _player.y + BUTTON_VOLUME_Y-2, &rcVoltab, SURF_PLAYER );
	}
}


/////////////////////////////
// 再生処理
/////////////////////////////

extern HWND       g_hWnd_Main;

void Interface_Process( HWND hWnd, bool bDraw );

void if_Player_PushOneStatusFlag( int32_t flag )
{
	switch( flag )
	{
	case IF_PLAYER_STATUS_LOOP: break;
	default: return;
	}

	if( flag & _player.status_flag ) _player.status_flag &= ~flag;
	else                             _player.status_flag |=  flag;

	if( _player.status_flag & IF_PLAYER_STATUS_LOOP ) g_strm_xa2->tune_order_loop( true  );
	else                                              g_strm_xa2->tune_order_loop( false );
}

// 再生開始
bool if_Player_StartPlay()
{
	bool b_ret = false;
	pxtnVOMITPREPARATION prep = {0};

	if( g_strm_xa2->tune_is_sampling() ){ b_ret = true; goto term; }

	{
		int32_t sps;
		int32_t beat_num, beat_clock, meas_num;

		g_strm_xa2->stream_get_quality_safe( NULL, &sps, NULL );
		g_pxtn->master->Get( &beat_num, NULL, &beat_clock, &meas_num );

		if( _player.status_flag & IF_PLAYER_STATUS_LOOP ) prep.flags |= pxtnVOMITPREPFLAG_loop     ;
		if( _player.status_flag & IF_PLAYER_STATUS_MUTE ) prep.flags |= pxtnVOMITPREPFLAG_unit_mute;

		prep.master_volume = _player.volume;
	}

	if( !g_strm_xa2->tune_order_start( &prep ) ) goto term;

	b_ret = true;
term:
	if( b_ret )
	{
		if_Player_SetPlayButtonAnime( 2 );
	}
	else
	{
		if_Player_SetPlayButtonAnime( 0 );
	}

	return true;
}

void if_Player_StopPlay()
{
	g_strm_xa2->tune_order_stop( 0.5f );
	_sample_offset = g_strm_xa2->tune_get_sampling_count_safe();
	if_Player_SetPlayButtonAnime( 0 );
}


void if_Player_ZeroSampleOffset()
{
	_sample_offset = 0;
}


bool if_Player_Callback_Sampled( void* user, const pxtnService* pxtn )
{
	bool b_end = false;

	if( pxtn->moo_is_end_vomit() )
	{
		b_end = true;
	}
	else
	{
		int32_t clock     = pxtn->moo_get_now_clock();
		int32_t clock_end = pxtn->moo_get_end_clock();

		if( !clock_end ) b_end = true;
		else
		{
			int32_t left = PROGRESS_X + clock * PROGRESS_WIDTH  / clock_end;
			if( left != _player.progress_rect.l )
			{
				_player.progress_rect.l = (float)left;
				_player.progress_rect.r = _player.progress_rect.l + 2;
				_player.progress_rect.t = PROGRESS_Y;
				_player.progress_rect.b = PROGRESS_Y + PROGRESS_HEIGHT;

				if( _player.progress_rect.r > PROGRESS_X + PROGRESS_WIDTH )
					_player.progress_rect.r = PROGRESS_X + PROGRESS_WIDTH;
			}
		}
	}

	if( b_end )
	{
		if_Player_SetPlayButtonAnime( 0 );
		_player.progress_rect.l = PROGRESS_X;
		_player.progress_rect.t = PROGRESS_Y;
		_player.progress_rect.b = PROGRESS_Y;
		_player.progress_rect.r = PROGRESS_X;
	}

	WINDOWPLACEMENT place;
	place.length = sizeof(WINDOWPLACEMENT);
	if( GetWindowPlacement( g_hWnd_Main, &place ) )
	{
		if( place.showCmd != SW_SHOWMINIMIZED ) Interface_Process( g_hWnd_Main, true );
	}
	return true;
}


void if_Player_SetVolume_cur_pos( float x, float y )
{
	float v = x - _player.x - BUTTON_VOLUME_X;
	if( v <                0 ) v =                0;
	if( v > BUTTON_VOLUME_W ) v = BUTTON_VOLUME_W;
	_player.volume = (float)v / BUTTON_VOLUME_W;

	g_strm_xa2->tune_order_volume( _player.volume );
}
