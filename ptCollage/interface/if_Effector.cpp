
#include <pxtnService.h>
extern pxtnService *g_pxtn;

#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../../Generic/KeyControl.h"
#include "../../Generic/if_Generic.h"

#include "Interface.h"
#include "SurfaceNo.h"
#include "if_Effector.h"

#define EFFECTORFLAG_STATUS_OPEN     0x1000

#define _EFFECTOR_W    272
#define _EFFECTOR_H    152

#define _BUTTON_CLOSE_X 256
#define _BUTTON_CLOSE_Y   0
#define _BUTTON_CLOSE_W  16
#define _BUTTON_CLOSE_H  16

#define _PLAYED_X     56
#define _PLAYED_H     16
#define _PLAYED_W     16

#define _RECORD_H     16
#define _RECORD_W    184
#define _RECORD_X     72
#define _PADDING_Y     4

#define _DELAY_Y      32
#define _OVERDRIVE_Y 108

#define _GROUP_X      12

// delay x
#define _SCALE_X      44
#define _HERTZ_X      84
#define _PERCENT_X   140

// over x
#define _AMP_X        92

bool Function_IDM_DELAY(     int32_t index );
bool Function_IDM_OVERDRIVE( int32_t index );

enum _CURSOR{
	_CURSOR_NONE = 0,

	_CURSOR_DELAY_0,
	_CURSOR_DELAY_1,
	_CURSOR_DELAY_2,
	_CURSOR_DELAY_3,

	_CURSOR_OVERDRIVE_0,
	_CURSOR_OVERDRIVE_1,
};

typedef struct
{
	float   x, y;
	int32_t flags;
}
_EFFECTORSTRUCT;

static _EFFECTORSTRUCT _effector    = {0};
static fRECT           _rcPanel     = {0};
static fRECT           _rc_view     = {0};
static const uxSS<16>     _status_name = "effector.status";
static _CURSOR         _cur         = _CURSOR_NONE;

static const pxFile2* _ref_file_profile = NULL;

void if_Effector_SavePosition()
{
	bool          b_ret = false;
	pxDescriptor* desc  = NULL ;

	if( _ref_file_profile->open_w( &desc, _status_name, NULL, NULL ) )
	{
		desc->w_asfile( &_effector, sizeof(_effector), 1 );
		SAFE_DELETE( desc );
	}
}

// 座標
void if_Effector_SetPosition( float x, float y )
{
	_effector.x = x;
	_effector.y = y;
	if( _effector.x < 8 ) _effector.x = 8;
	if( _effector.y < 8 ) _effector.y = 8;
	if( _effector.x > _rc_view.r - _EFFECTOR_W - 8 ) _effector.x = _rc_view.r - _EFFECTOR_W - 8;
	if( _effector.y > _rc_view.b - _EFFECTOR_H - 8 ) _effector.y = _rc_view.b - _EFFECTOR_H - 8;

}
void if_Effector_GetPosition( float *px, float *py ){ *px = _effector.x; *py = _effector.y; }

// 領域
void if_Effector_SetRect( const fRECT *rc_view )
{
	_rc_view = *rc_view;

	if( _effector.x <                            8 ) _effector.x =                            8;
	if( _effector.y <                            8 ) _effector.y =                            8;
	if( _effector.x > rc_view->r - _EFFECTOR_W - 8 ) _effector.x = rc_view->r - _EFFECTOR_W - 8;
	if( _effector.y > rc_view->b - _EFFECTOR_H - 8 ) _effector.y = rc_view->b - _EFFECTOR_H - 8;

	_rcPanel.l = _effector.x;
	_rcPanel.t = _effector.y;
	_rcPanel.r = _effector.x + _EFFECTOR_W;
	_rcPanel.b = _effector.y + _EFFECTOR_H;
}

static int _GetCursorHit_Record( float cur_x, float cur_y )
{
	float   x, y;
	int32_t num = g_pxtn->Delay_Max();

	x   = _effector.x + _RECORD_X;
	y   = _effector.y + _DELAY_Y ;

	if( cur_x >= x && cur_x < x + _RECORD_W && cur_y >= y && cur_y < y + _RECORD_H * num )
	{
		return _CURSOR_DELAY_0 + (int32_t)( cur_y - y ) / _RECORD_H;
	}

	x   = _effector.x + _RECORD_X   ;
	y   = _effector.y + _OVERDRIVE_Y;
	num = g_pxtn->OverDrive_Max();
	if( cur_x >= x && cur_x < x + _RECORD_W && cur_y >= y && cur_y < y + _RECORD_H * num )
	{
		return _CURSOR_OVERDRIVE_0 + (int32_t)( cur_y - y ) / _RECORD_H;
	}

	return _CURSOR_NONE;
}

static int _GetCursorHit_Played( float cur_x, float cur_y )
{
	int32_t num = g_pxtn->Delay_Max();
	float x     = _effector.x + _RECORD_X - _PLAYED_W;
	float y     = _effector.y + _DELAY_Y ;

	if( cur_x >= x && cur_x < x + _PLAYED_W && cur_y >= y && cur_y < y + _PLAYED_H * num )
	{
		return _CURSOR_DELAY_0 + (int32_t)( cur_y - y ) / _RECORD_H;
	}

	y   = _effector.y + _OVERDRIVE_Y;
	num = g_pxtn->OverDrive_Max();
	if( cur_x >= x && cur_x < x + _PLAYED_W && cur_y >= y && cur_y < y + _PLAYED_H * num )
	{
		return _CURSOR_OVERDRIVE_0 + (int32_t)( cur_y - y ) / _RECORD_H;
	}

	return _CURSOR_NONE;
}


void if_Effector_SetCursor( float cur_x, float cur_y )
{
	_cur = (_CURSOR)_GetCursorHit_Record( cur_x, cur_y );
}

fRECT*if_Effector_GetRect()
{
	return &_rcPanel;
}

bool if_Effector_ShutButton( float x, float y )
{
	if( !( _effector.flags & EFFECTORFLAG_STATUS_OPEN ) ) return false;

	// オープンクローズ
	if( x >= _effector.x + _BUTTON_CLOSE_X                   &&
		x <  _effector.x + _BUTTON_CLOSE_X + _BUTTON_CLOSE_W &&
		y >= _effector.y + _BUTTON_CLOSE_Y                   &&
		y <  _effector.y + _BUTTON_CLOSE_Y + _BUTTON_CLOSE_H )
	{
		_effector.flags &= ~EFFECTORFLAG_STATUS_OPEN;
		return true;
	}
	return false;
}



int32_t if_Effector_GetFlags()
{
	return _effector.flags;
}

void if_Effector_SetFlags( int32_t flags )
{
	_effector.flags = flags;
}

bool if_Effector_IsOpen()
{
	if( _effector.flags & EFFECTORFLAG_STATUS_OPEN ) return true;
	return false;
}

bool if_Effector_HitButton( float cur_x, float cur_y )
{
	int ofs = _GetCursorHit_Record( cur_x, cur_y );
	int cur = 0;
	int cur_max = 0;

	switch( ofs )
	{
	case _CURSOR_DELAY_0    :
	case _CURSOR_DELAY_1    :
	case _CURSOR_DELAY_2    :
	case _CURSOR_DELAY_3    :

		cur = ofs - _CURSOR_DELAY_0;
		if( cur >= g_pxtn->Delay_Num() ) cur = -1;


		Function_IDM_DELAY(     cur     );
		KeyControl_Clear();
		return true;

	case _CURSOR_OVERDRIVE_0:
	case _CURSOR_OVERDRIVE_1:

		cur = ofs - _CURSOR_OVERDRIVE_0;
		if( cur >= g_pxtn->OverDrive_Num() ) cur = -1;
		Function_IDM_OVERDRIVE( cur );
		KeyControl_Clear();
		return true;
	}

	return false;

}

bool if_Effector_HitPlayed( float cur_x, float cur_y, bool* p_b )
{
	pxtnDelay     *p_delay;
	pxtnOverDrive *p_ovdrv;

	int cur = _GetCursorHit_Played( cur_x, cur_y );
	switch( cur )
	{
	case _CURSOR_DELAY_0    :
	case _CURSOR_DELAY_1    :
	case _CURSOR_DELAY_2    :
	case _CURSOR_DELAY_3    :

		if( p_delay = g_pxtn->Delay_Get(  cur - _CURSOR_DELAY_0 ) )
		{
			*p_b = p_delay->switch_played();
			return true;
		}
		break;

	case _CURSOR_OVERDRIVE_0:
	case _CURSOR_OVERDRIVE_1:

		if( p_ovdrv = g_pxtn->OverDrive_Get( cur - _CURSOR_OVERDRIVE_0 ) )
		{
			*p_b = p_ovdrv->switch_played();
			return true;
		}
		break;
	}

	return false;
}

void if_Effector_SetPlayed( float cur_x, float cur_y, bool b )
{
	pxtnDelay     *p_delay;
	pxtnOverDrive *p_ovdrv;

	int cur = _GetCursorHit_Played( cur_x, cur_y );
	switch( cur )
	{
	case _CURSOR_DELAY_0    :
	case _CURSOR_DELAY_1    :
	case _CURSOR_DELAY_2    :
	case _CURSOR_DELAY_3    :

		if( p_delay = g_pxtn->Delay_Get( cur - _CURSOR_DELAY_0 ) ) p_delay->set_played( b );
		break;

	case _CURSOR_OVERDRIVE_0:
	case _CURSOR_OVERDRIVE_1:

		if( p_ovdrv = g_pxtn->OverDrive_Get( cur - _CURSOR_OVERDRIVE_0 ) ) p_ovdrv->set_played( b );
		break;
	}

}


void if_Effector_Put()
{
	if( !( _effector.flags & EFFECTORFLAG_STATUS_OPEN ) ) return;

	fRECT rcFrame   = {240,  0,512,152};
	fRECT rc_hz     = {240,192,256,200};
	fRECT rc_per    = {240,200,248,208};
	fRECT rc_played = {296,152,312,168};
	fRECT rc_amp    = {248,200,272,208};

	fRECT rc_delay_type[] =
	{
		{240,168,272,176},
		{240,176,272,184},
		{240,184,272,192},
	};

	fRECT rc_cur = {312,152,496,168};

	float x, y;
	int  num ;

	g_dxdraw->tex_Put_View( _effector.x, _effector.y, &rcFrame, SURF_DIALOGS );


	y = 0;
	switch( _cur )
	{
	case _CURSOR_DELAY_0    :
	case _CURSOR_DELAY_1    :
	case _CURSOR_DELAY_2    :
	case _CURSOR_DELAY_3    :
		y = _effector.y + _DELAY_Y     + ( _cur - _CURSOR_DELAY_0     ) * _RECORD_H;
		break;
	case _CURSOR_OVERDRIVE_0:
	case _CURSOR_OVERDRIVE_1:
		y = _effector.y + _OVERDRIVE_Y + ( _cur - _CURSOR_OVERDRIVE_0 ) * _RECORD_H;
		break;
	}
	if( y ) g_dxdraw->tex_Put_View( _effector.x + _RECORD_X, y, &rc_cur, SURF_DIALOGS );

	// ディレイ

	pxtnDelay *p_delay;
	num = g_pxtn->Delay_Num();
	for( int e = 0; e < num; e++ )
	{
		p_delay = g_pxtn->Delay_Get( e );

		x = _effector.x + _PLAYED_X;
		y = _effector.y + _DELAY_Y + _RECORD_H * e;
		if( p_delay->get_played() ) g_dxdraw->tex_Put_View( x, y, &rc_played, SURF_DIALOGS );

		y = _effector.y + _DELAY_Y + _PADDING_Y + _RECORD_H * e;

		x = _effector.x + _RECORD_X + _GROUP_X;
		if_gen_num6(  x, y, p_delay->get_group(), 2 );

		x = _effector.x + _RECORD_X + _SCALE_X;
		g_dxdraw->tex_Put_View( x, y, &rc_delay_type[ p_delay->get_unit() ], SURF_DIALOGS );

		x = _effector.x + _RECORD_X + _HERTZ_X;
		if_gen_float_n(  x,      y, p_delay->get_freq(), 3,1 );
		g_dxdraw->tex_Put_View( x + 40, y, &rc_hz, SURF_DIALOGS );

		x = _effector.x + _RECORD_X + _PERCENT_X;
		if_gen_float_n( x,      y, p_delay->get_rate(), 3,0 );
		g_dxdraw->tex_Put_View( x + 24, y, &rc_per, SURF_DIALOGS );
	}

	// オーバードライブ
	pxtnOverDrive *p_ovdrv;
	num = g_pxtn->OverDrive_Num();
	for( int e = 0; e < num; e++ )
	{
		p_ovdrv = g_pxtn->OverDrive_Get( e );

		x = _effector.x + _PLAYED_X;
		y = _effector.y + _OVERDRIVE_Y + _RECORD_H * e;
		if( p_ovdrv->get_played() ) g_dxdraw->tex_Put_View( x, y, &rc_played, SURF_DIALOGS );

		y = _effector.y + _OVERDRIVE_Y + _PADDING_Y + _RECORD_H * e;

		x = _effector.x + _RECORD_X + _GROUP_X;
		if_gen_num6( x, y, p_ovdrv->get_group(), 2 );

		x = _effector.x + _RECORD_X + _SCALE_X;
		if_gen_float_n( x,      y, p_ovdrv->get_cut(), 2,1 );
		g_dxdraw->tex_Put_View( x + 32, y, &rc_per, SURF_DIALOGS );

		x = _effector.x + _RECORD_X + _AMP_X;
		g_dxdraw->tex_Put_View( x     , y, &rc_amp, SURF_DIALOGS );
		if_gen_float_n( x + 24, y, p_ovdrv->get_amp(), 1,1 );
	}

#ifdef _DEBUG
	if_gen_num6( _effector.x, _effector.y    , _effector.x, 3 );
	if_gen_num6( _effector.x, _effector.y + 8, _effector.y, 3 );
#endif

}


#include "if_SubWindows.h"

static SUBWINDOWSTRUCT* _handle = NULL;
bool if_Cursor_Action_Effector( float cur_x, float cur_y );

void if_Effector_init( HWND hWnd, const pxFile2* file_profile )
{
	_ref_file_profile = file_profile;

	bool          b_ret = false;
	pxDescriptor* desc  = NULL ;

	if( !_ref_file_profile->open_w( &desc, _status_name, NULL, NULL ) ||
		!desc->r( &_effector, sizeof(_effector), 1 ) )
	{
		_effector.x  =   252;
		_effector.y  =   368;
	}
	SAFE_DELETE( desc );

	_handle = if_SubWindows_Create( if_Effector_Put, if_Cursor_Action_Effector );
}

void if_Effector_SetFocus()
{
	if_SubWindows_SetFocus( _handle );
}

void if_Effector_SetOpen()
{
	if( _effector.flags & EFFECTORFLAG_STATUS_OPEN ){ _effector.flags &= ~EFFECTORFLAG_STATUS_OPEN;                         }
	else                                            { _effector.flags |=  EFFECTORFLAG_STATUS_OPEN; if_Effector_SetFocus(); }
}
