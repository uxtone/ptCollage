

#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../../Generic/KeyControl.h"

#include "SurfaceNo.h"
#include "Interface.h"
#include "if_VolumePanel.h"
#include "if_ToolPanel.h"



#define _VOLUMETYPE_X 24
#define _VOLUMETYPE_Y  8

static bool _bUseVolume  = false;

static VOLUMEMODE _volume_mode; // 0:velocity / 1:pan / 2:volume
static fRECT _rcPanel  = {0};
static fRECT _rcSwitch = {0};

void if_InitVolumePanel()
{
	_volume_mode  = VOLUMEMODE_Velocity;
}

void if_SetVolumeMode( VOLUMEMODE volume_mode )
{
	_volume_mode  = volume_mode;
}

VOLUMEMODE if_GetVolumeMode()
{
	return _volume_mode;
}

void if_VolumePanel_SetRect( const fRECT *rc_view )
{
	_rcPanel.l = rc_view->l;
	_rcPanel.r = rc_view->l + NAMETRAY_WIDTH;
	_rcPanel.b = rc_view->b - SCROLL_HEIGHT;
	if( if_VolumePanel_IsUse() ) _rcPanel.t = rc_view->b - SCROLL_HEIGHT - VOLUME_HEIGHT;
	else                         _rcPanel.t = rc_view->b - SCROLL_HEIGHT;
}

void if_VolumeSwitch_SetRect( const fRECT *rc_view )
{
	_rcSwitch.l = rc_view->l;
	_rcSwitch.r = rc_view->l + NAMETRAY_WIDTH;
	_rcSwitch.t = rc_view->b - SCROLL_HEIGHT;
	_rcSwitch.b = rc_view->b;
}



void if_PutVolumePanel()
{
	fRECT rcVol = {  0,272,112,400};
	fRECT rc    = {112,280,200,292};

	static const fRECT rcScale[] =
	{
		{240,192,272,320},
		{272,192,304,320},
		{272,192,304,320},
		{240,192,272,320},
		{304,192,336,320},
		{336,192,368,320},
		{336,192,368,320},
		{336,192,368,320},
	};

	if( if_ToolPanel_GetMode() != enum_ToolMode_W && _bUseVolume )
	{
		g_dxdraw->tex_Put_View( _rcPanel.l      , _rcPanel.t, &rcVol,  SURF_TABLES );
		g_dxdraw->tex_Put_View( _rcPanel.l + 112, _rcPanel.t, &rcScale[ _volume_mode ], SURF_FIELDS );
		rc.t += _volume_mode * 12;
		rc.b += _volume_mode * 12;
		g_dxdraw->tex_Put_View( _rcPanel.l + _VOLUMETYPE_X, _rcPanel.t + _VOLUMETYPE_Y + _volume_mode * 12, &rc, SURF_TABLES );
	}
}

bool if_VolumePanel_IsUse()
{
	return _bUseVolume;
}

void if_SetUseVolume( bool bUse )
{
	_bUseVolume = bUse;
}

void if_PutVolumeSwitch()
{
	static const fRECT rc[] =
	{
		{  0,400,144,416},
		{  0,416,144,432},
		{  0,432,144,448},
	};

	int32_t i;

	if( if_ToolPanel_GetMode() == enum_ToolMode_W ) i = 2;
	else if( _bUseVolume                          ) i = 1;
	else                                            i = 0;

	g_dxdraw->tex_Put_View( _rcSwitch.l, _rcSwitch.t, &rc[ i ], SURF_TABLES );

}

VOLUMEBUTTON if_VolumePanel_GetHitButton( float cur_x, float cur_y )
{
	if( if_ToolPanel_GetMode() == enum_ToolMode_W ) return VOLUMEBUTTON_unknown;

	// 表示倍率変更
	if( cur_x > _rcSwitch.l + 16 && cur_x < _rcSwitch.l + 16+16 &&
		cur_y > _rcSwitch.t +  2 && cur_y < _rcSwitch.t +  2+12 )
	{
		return VOLUMEBUTTON_Switch;
	}
	else if( _bUseVolume &&
			 cur_x > _rcPanel.l + _VOLUMETYPE_X && cur_x < _rcPanel.l + _VOLUMETYPE_X+88   &&
			 cur_y > _rcPanel.t + _VOLUMETYPE_Y && cur_y < _rcPanel.t + _VOLUMETYPE_Y+12 * 8 )
	{
		return (VOLUMEBUTTON)( (int32_t)( cur_y - _rcPanel.t - _VOLUMETYPE_Y ) / 12 + VOLUMEBUTTON_Velocity );
	}

	return VOLUMEBUTTON_unknown;
}

bool if_VolumePanel_SetHitButton( VOLUMEBUTTON button )
{
	bool bDraw = false;

	switch( button )
	{
	case VOLUMEBUTTON_Switch    :
		if( _bUseVolume ) _bUseVolume = false;
		else              _bUseVolume = true;
		bDraw = true;
		break;

	case VOLUMEBUTTON_Velocity  :
	case VOLUMEBUTTON_Pan_Volume:
	case VOLUMEBUTTON_Pan_Time  :
	case VOLUMEBUTTON_Volume    :
	case VOLUMEBUTTON_KeyPorta  :
	case VOLUMEBUTTON_VoiceNo   :
	case VOLUMEBUTTON_GroupNo   :
	case VOLUMEBUTTON_Tuning    :
		_volume_mode = (VOLUMEMODE)( button - VOLUMEBUTTON_Velocity );
		if_SetVolumeMode( _volume_mode );
		bDraw = true;
		break;
	default: break;
	}

	return bDraw;
}
