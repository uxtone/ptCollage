
#include <pxtnService.h>
extern pxtnService*  g_pxtn    ;

#include <pxtonewinXA2.h>
extern pxtonewinXA2* g_strm_xa2;

#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include <pxwFilePath.h>

#include <pxDebugLog.h>

#include "../../Generic/Japanese.h"
#include "../../Generic/if_gen_Scroll.h"
#include "../../Generic/if_Generic.h"

#include "../../ptConfig/ptConfig.h"

#include "./Interface.h"
#include "./SurfaceNo.h"
#include "./if_BaseField.h"
#include "./if_PlayField.h"
#include "./if_Cursor.h"

//#include "./if_Player.h"

#define _PLAYERFLAG_CHECK_LOOP   0x01
#define _PLAYERFLAG_CHECK_SCRL   0x02
#define _PLAYERFLAG_CHECK_MUTE   0x04
#define _PLAYERFLAG_STATUS_OPEN  0x10

#define _PLAYER_W       160
#define _PLAYER_H        72

#define _BUTTON_PLAY_X  120
#define _BUTTON_PLAY_Y   48

#define _BUTTON_VOLUME_X 80
#define _BUTTON_VOLUME_Y 30
#define _BUTTON_VOLUME_W 72
#define _BUTTON_VOLUME_H  8


#define _BUTTON_LOOP_X   16
#define _BUTTON_LOOP_Y   18
#define _BUTTON_SCRL_X   16
#define _BUTTON_SCRL_Y   34
#define _BUTTON_MUTE_X   16
#define _BUTTON_MUTE_Y   50

#define _BUTTON_OPEN_X  144
#define _BUTTON_OPEN_Y    0
#define _BUTTON_OPEN_W   16
#define _BUTTON_OPEN_H   16

typedef struct
{
	float    x, y  ;
	int32_t  play_button_anime;
	int32_t  flags ;
	float    volume;
}
PLAYERSTRUCT;

static PLAYERSTRUCT   _player  = {0};
static fRECT          _rcPanel = {0};
static fRECT          _rc_view = {0};
static const TCHAR*   _status_name = _T("player.status");
static const pxFile2* _ref_file_profile = NULL;

void if_Player_init( const pxFile2* file_profile )
{
	_ref_file_profile = file_profile;
}

void if_Player_SetRect( const fRECT *rc_view )
{
	_rc_view = *rc_view;

	if( _player.x < 8 ) _player.x = 8;
	if( _player.y < 8 ) _player.y = 8;
	if( _player.x > rc_view->r - _PLAYER_W - 8 )
		_player.x = rc_view->r - _PLAYER_W - 8;
	if( _player.y > rc_view->b - _PLAYER_H - 8 )
		_player.y = rc_view->b - _PLAYER_H - 8;

	_rcPanel.l = _player.x;
	_rcPanel.t = _player.y;
	_rcPanel.r = _player.x + _PLAYER_W;
	_rcPanel.b = _player.y + _PLAYER_H;
}

bool if_Player_SavePosition()
{
	bool          b_ret = false;
	pxDescriptor* desc  = NULL ;

	// was "if( open_w(..) )" with no braces: it only guarded the first write, so a failed
	// open (read-only folder) went on to write through a NULL descriptor.
	if( !_ref_file_profile->open_w( &desc, _status_name, NULL, NULL ) ) goto term;
	if( !desc->w_asfile( &_player.x     , sizeof(int32_t), 1 ) ) goto term;
	if( !desc->w_asfile( &_player.y     , sizeof(int32_t), 1 ) ) goto term;
	if( !desc->w_asfile( &_player.flags , sizeof(int32_t), 1 ) ) goto term;
	if( !desc->w_asfile( &_player.volume, sizeof(float  ), 1 ) ) goto term;

	b_ret = true;
term:
	SAFE_DELETE( desc );
	return b_ret;
}

// 座標
void if_Player_SetPosition( float x,   float y   )
{
	_player.x = x;
	_player.y = y;
	if( _player.x < 8 ) _player.x = 8;
	if( _player.y < 8 ) _player.y = 8;
	if( _player.x > _rc_view.r - _PLAYER_W - 8 )
		_player.x = _rc_view.r - _PLAYER_W - 8;
	if( _player.y > _rc_view.b - _PLAYER_H - 8 )
		_player.y = _rc_view.b - _PLAYER_H - 8;
}


void if_Player_GetPosition( float *px, float *py ){ *px = _player.x; *py = _player.y; }


fRECT* if_Player_GetRect()
{
	return &_rcPanel;
}

bool if_Player_IsPlayButton( float x, float y )
{
	if( !( _player.flags & _PLAYERFLAG_STATUS_OPEN ) ) return false;
	if( x >= _player.x + _BUTTON_PLAY_X && x < _player.x + _BUTTON_PLAY_X + 32 &&
		y >= _player.y + _BUTTON_PLAY_Y && y < _player.y + _BUTTON_PLAY_Y + 16 ) return true;
	return false;
}

bool if_Player_IsVolume( float x, float y )
{
	if( !( _player.flags & _PLAYERFLAG_STATUS_OPEN ) ) return false;
	if( x >= _player.x + _BUTTON_VOLUME_X &&
		x <  _player.x + _BUTTON_VOLUME_X + _BUTTON_VOLUME_W &&
		y >= _player.y + _BUTTON_VOLUME_Y &&
		y <  _player.y + _BUTTON_VOLUME_Y + _BUTTON_VOLUME_H ) return true;
	return false;
}


bool if_Player_CheckButton( float x, float y )
{
	if( !( _player.flags & _PLAYERFLAG_STATUS_OPEN ) ) return false;

	if( x >= _player.x + _BUTTON_LOOP_X && x < _player.x + _BUTTON_LOOP_X + 16 &&
		y >= _player.y + _BUTTON_LOOP_Y && y < _player.y + _BUTTON_LOOP_Y + 12 )
	{
		if( _player.flags & _PLAYERFLAG_CHECK_LOOP ){ _player.flags &= ~_PLAYERFLAG_CHECK_LOOP; g_strm_xa2->tune_order_loop( false ); }
		else                                        { _player.flags |=  _PLAYERFLAG_CHECK_LOOP; g_strm_xa2->tune_order_loop( true  ); }

		return true;
	}

	if( x >= _player.x + _BUTTON_SCRL_X && x <  _player.x + _BUTTON_SCRL_X + 16 &&
		y >= _player.y + _BUTTON_SCRL_Y && y <  _player.y + _BUTTON_SCRL_Y + 12 )
	{
		if( _player.flags & _PLAYERFLAG_CHECK_SCRL ) _player.flags &= ~_PLAYERFLAG_CHECK_SCRL;
		else                                         _player.flags |=  _PLAYERFLAG_CHECK_SCRL;
		return true;
	}

	if( x >= _player.x + _BUTTON_MUTE_X && x <  _player.x + _BUTTON_MUTE_X + 16 &&
		y >= _player.y + _BUTTON_MUTE_Y && y <  _player.y + _BUTTON_MUTE_Y + 12 )
	{
		if( _player.flags & _PLAYERFLAG_CHECK_MUTE ){ _player.flags &= ~_PLAYERFLAG_CHECK_MUTE; g_strm_xa2->tune_order_unit_mute( false ); }
		else                                        { _player.flags |=  _PLAYERFLAG_CHECK_MUTE; g_strm_xa2->tune_order_unit_mute( true  ); }
		return true;
	}

	if( x >= _player.x + _BUTTON_OPEN_X                  &&
		x <  _player.x + _BUTTON_OPEN_X + _BUTTON_OPEN_W &&
		y >= _player.y + _BUTTON_OPEN_Y                  &&
		y <  _player.y + _BUTTON_OPEN_Y + _BUTTON_OPEN_H )
	{
		_player.flags &= ~_PLAYERFLAG_STATUS_OPEN;
		return true;
	}

	return false;
}

int32_t if_Player_GetStatusFlag()
{
	return _player.flags;
}

bool if_Player_flag_is_check_mute()
{
	if( _player.flags & _PLAYERFLAG_CHECK_MUTE ) return true;
	return false;
}

bool if_Player_IsOpen()
{
	if( _player.flags & _PLAYERFLAG_STATUS_OPEN ) return true;
	return false;
}



int32_t if_Player_GetPlayButtonAnime(            ){ return _player.play_button_anime;      }
void    if_Player_SetPlayButtonAnime( int32_t no ){        _player.play_button_anime = no; }


void if_Player_Put()
{
	if( !( _player.flags & _PLAYERFLAG_STATUS_OPEN ) ) return;

	fRECT rc_frame  = {  0,256,160,328};
	fRECT rc_switch = {160,320,176,332};

	fRECT rcPlayButton[] =
	{
		{160,256,192,272},
		{160,272,192,288},
		{160,288,192,304},
		{160,304,192,320},
	};

	fRECT rcVolume = { 80,336,152,344};
	fRECT rcVoltab = {160,332,163,344};


	g_dxdraw->tex_Put_View( _player.x, _player.y, &rc_frame, SURF_DIALOGS );

	g_dxdraw->tex_Put_View( _player.x + _BUTTON_PLAY_X, _player.y + _BUTTON_PLAY_Y,
							&rcPlayButton[ _player.play_button_anime ], SURF_DIALOGS );
	if( _player.flags & _PLAYERFLAG_CHECK_LOOP )
		g_dxdraw->tex_Put_View( _player.x + _BUTTON_LOOP_X, _player.y + _BUTTON_LOOP_Y, &rc_switch, SURF_DIALOGS );
	if( _player.flags & _PLAYERFLAG_CHECK_SCRL )
		g_dxdraw->tex_Put_View( _player.x + _BUTTON_SCRL_X, _player.y + _BUTTON_SCRL_Y, &rc_switch, SURF_DIALOGS );
	if( _player.flags & _PLAYERFLAG_CHECK_MUTE )
		g_dxdraw->tex_Put_View( _player.x + _BUTTON_MUTE_X, _player.y + _BUTTON_MUTE_Y, &rc_switch, SURF_DIALOGS );

	int v = (int)( _player.volume * _BUTTON_VOLUME_W );
	rcVolume.r = rcVolume.l + v;
	g_dxdraw->tex_Put_View( _player.x + _BUTTON_VOLUME_X, _player.y + _BUTTON_VOLUME_Y, &rcVolume, SURF_DIALOGS );
	g_dxdraw->tex_Put_View( _player.x + _BUTTON_VOLUME_X + v - 2, _player.y + _BUTTON_VOLUME_Y-2, &rcVoltab, SURF_DIALOGS );

#ifdef _DEBUG
	if_gen_num6( _player.x, _player.y    , _player.x, 3 );
	if_gen_num6( _player.x, _player.y + 8, _player.y, 3 );
#endif

}

/////////////////////////////
// 再生処理
/////////////////////////////

#include "if_BaseField.h"
#include "if_PlayField.h"

extern HWND          g_hWnd_Main ;
extern if_gen_Scroll g_ScrlEventH;

void Interface_Process( HWND hWnd, bool bDraw );

static bool _b_1st       = false;
static int32_t _meas         = 0;

void if_Player_OnMutePlay()
{
	_player.flags |= _PLAYERFLAG_CHECK_MUTE;
	g_strm_xa2->tune_order_unit_mute( true );
}

bool if_Player_StartPlay()
{
	bool                 b_ret    = false;
	pxtnVOMITPREPARATION prep     = { 0 };
	int32_t              end_meas =   0  ;

	if( g_strm_xa2->tune_is_sampling() ){ b_ret = true; goto term; }

	dlog_c( "if_Player_StartPlay();" );

	prep.start_pos_meas = if_PlayField_GetStartMeas();
	prep.master_volume  = _player.volume;

	if( _player.flags & _PLAYERFLAG_CHECK_MUTE ) prep.flags |= pxtnVOMITPREPFLAG_unit_mute;
	if( _player.flags & _PLAYERFLAG_CHECK_LOOP ) prep.flags |= pxtnVOMITPREPFLAG_loop     ;

	end_meas = if_PlayField_GetEndMeas();

	if( end_meas != -1 )
	{
		prep.meas_end    = end_meas;
		prep.meas_repeat = if_PlayField_GetStartMeas();
	}

	{
		int   beat_num, beat_clock;
		float beat_tempo;

		g_pxtn->master->Get( &beat_num, &beat_tempo, &beat_clock, NULL );

		_meas         = if_PlayField_GetStartMeas();

		_b_1st        = false;

		if( _player.flags & _PLAYERFLAG_CHECK_SCRL )
		{
			g_ScrlEventH.SetOffset( (float)( if_BaseField_GetBeatWidth() * beat_num * _meas ) );
			Interface_Process( g_hWnd_Main, true );
		}
	}

	if( !g_strm_xa2->tune_order_start( &prep ) )
	{
		Japanese_MessageBox( g_hWnd_Main, _T("ready play"), _T("error"), MB_OK|MB_ICONEXCLAMATION );
		goto term;
	}

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

// 再生停止
void if_Player_StopPlay()
{
	dlog_c( "if_Player_StopPlay();" );
	g_strm_xa2->tune_order_stop( 0 );
	if_Player_SetPlayButtonAnime( 0 );
}

bool if_Player_Callback_Sampled( void* user, const pxtnService* pxtn )
{
	bool    b_update = false;
	HWND    hwnd     = (HWND)user;
	int32_t beat_num =     0;

	if( _player.flags & _PLAYERFLAG_CHECK_SCRL )
	{
		float   beat_tempo;
		int32_t beat_clock;
		int32_t clock     = pxtn->moo_get_now_clock();

		pxtn->master->Get( &beat_num, &beat_tempo, &beat_clock, NULL );

		int32_t meas  = clock / (beat_clock * beat_num);
		if     ( !_b_1st       ){ _b_1st = true; b_update = true; }
		else if( meas != _meas ){                b_update = true; }
		_meas = meas;
	}

	if( pxtn->moo_is_end_vomit() )
	{
		if_Player_SetPlayButtonAnime( 0 );
		b_update = true;
	}

	if( b_update )
	{
		int32_t meas_w  = if_BaseField_GetBeatWidth() * beat_num;
		g_ScrlEventH.SetOffset( (float)( _meas * meas_w ) );
		InvalidateRect( hwnd, NULL, FALSE );
	}

	return true;
}

#include "if_SubWindows.h"

static SUBWINDOWSTRUCT* _handle = NULL;
bool if_Cursor_Action_Player( float cur_x, float cur_y );

void if_Player_init( HWND hWnd, const pxFile2* file_profile )
{
	bool          b_ret = false;
	pxDescriptor* desc  = NULL ;

	_ref_file_profile = file_profile;

	if( !_ref_file_profile->open_r( &desc, _status_name, NULL, NULL ) ||
		!desc->r( &_player.x     , sizeof(int32_t), 1 ) ||
		!desc->r( &_player.y     , sizeof(int32_t), 1 ) ||
		!desc->r( &_player.flags , sizeof(int32_t), 1 ) ||
		!desc->r( &_player.volume, sizeof(float  ), 1 ) )
	{
		_player.x      = 496;
		_player.y      = 384;
		_player.flags  = _PLAYERFLAG_CHECK_LOOP | _PLAYERFLAG_CHECK_SCRL | _PLAYERFLAG_STATUS_OPEN;
		_player.volume = 1;
	}
	SAFE_DELETE( desc );

	_player.flags |= _PLAYERFLAG_CHECK_MUTE;

	_player.play_button_anime = 0;

	if( _player.flags & _PLAYERFLAG_CHECK_LOOP ) g_strm_xa2->tune_order_loop( true  );
	else                                         g_strm_xa2->tune_order_loop( false );

	g_strm_xa2->tune_order_volume       ( _player.volume );
	g_strm_xa2->Voice_order_MasterVolume( _player.volume );

	_handle = if_SubWindows_Create( if_Player_Put, if_Cursor_Action_Player );
}

void if_Player_SetFocus()
{
	if_SubWindows_SetFocus( _handle );
}

void if_Player_SetOpen()
{
	if( _player.flags & _PLAYERFLAG_STATUS_OPEN ){ _player.flags &= ~_PLAYERFLAG_STATUS_OPEN;                       }
	else                                         { _player.flags |=  _PLAYERFLAG_STATUS_OPEN; if_Player_SetFocus(); }
}

void if_Player_SetVolume_cur_pos( float x, float y )
{
	float v = x - _player.x - _BUTTON_VOLUME_X;

	if( v <                0 ) v =                0;
	if( v > _BUTTON_VOLUME_W ) v = _BUTTON_VOLUME_W;

	_player.volume = (float)v / _BUTTON_VOLUME_W;

	g_strm_xa2->tune_order_volume       ( _player.volume );
	g_strm_xa2->Voice_order_MasterVolume( _player.volume );
}
