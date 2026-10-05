
#include <pxtnService.h>
extern pxtnService *g_pxtn;


#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../../Generic/KeyControl.h"
#include "../../Generic/if_gen_Scroll.h"

#include "SurfaceNo.h"
#include "Interface.h"

#include "if_BaseField.h"


extern if_gen_Scroll    g_ScrlEventH;


static uint32_t _color_out    = 0xff000040;
static uint32_t _color_field  = 0xff000080;
static uint32_t _color_marker = 0xff0000f0;
static fRECT    _rcField      = {0};

typedef struct
{
	int32_t start_meas;
	int32_t end_meas  ;
}
_FIELDINFORMATION;

static _FIELDINFORMATION _info;

bool if_PlayField_Initialize( const pxPalette* palette )
{
	if( !palette ) return false;
	if( !palette->get_color( COLORINDEX_play_out   , &_color_out    ) ) return false;
	if( !palette->get_color( COLORINDEX_play_field , &_color_field  ) ) return false;
	if( !palette->get_color( COLORINDEX_play_marker, &_color_marker ) ) return false;
	_info.start_meas     = -1;
	_info.end_meas       = -1;
	return true;
}

void if_PlayField_Reset()
{
	_info.start_meas     = -1;
	_info.end_meas       = -1;
}

void if_PlayField_SetRect( const fRECT *rc_view )
{
	_rcField.l = rc_view->l + NAMETRAY_WIDTH;
	_rcField.r = rc_view->r - SCROLLBUTTONSIZE;
	_rcField.t = rc_view->t + HEADER_H + MEASNO_HEIGHT;
	_rcField.b = rc_view->t + HEADER_H + MEASNO_HEIGHT + STARTFIELD_HEIGHT;

}

fRECT* if_PlayField_GetRect(){ return &_rcField; }

void if_PlayField_Put()
{
	int32_t view_w;
	int32_t meas_w;
	float   offset;
	float   xpos;
	int32_t beat_w;


	// 再生開始小節
	fRECT rcStart  = { 16,496, 56,504};
	fRECT rcEnd    = { 56,496, 96,504};
	fRECT rcTop    = { 16,480, 56,488};
	fRECT rcLast   = { 56,480, 96,488};
	fRECT rcRepeat = { 16,488, 56,496};
	fRECT rc;

	beat_w = if_BaseField_GetBeatWidth();
	view_w = _rcField.r - _rcField.l;
	meas_w = beat_w * g_pxtn->master->get_beat_num();
	offset = g_ScrlEventH.GetOffset();


	// データ範囲と TOP
	rc.l =                                         - offset + _rcField.l;
	rc.r = meas_w * g_pxtn->master->get_meas_num() - offset + _rcField.l;
	rc.t = _rcField.t;
	rc.b = _rcField.b;
	g_dxdraw->FillRect_clip( &rc, _color_field, &_rcField );
	g_dxdraw->tex_Put_Clip ( rc.l, _rcField.t, &rcTop, SURF_FIELDS, &_rcField );

	// データ範囲外
	rc.l = rc      .r;
	rc.r = _rcField.r;
	g_dxdraw->FillRect_clip(  &rc, _color_out, &_rcField );

	// 編集用再生範囲
	if( _info.start_meas >= 0 && _info.end_meas   >= 1 )
	{
		rc = _rcField;
		rc.t += 11;
		rc.b -=  2;
		rc.l  = meas_w * _info.start_meas - offset + _rcField.l;
		rc.r  = meas_w * _info.end_meas   - offset + _rcField.l;
		g_dxdraw->FillRect_clip(  &rc, _color_marker, &_rcField );
	}

	if( _info.start_meas >= 0 )
	{
		xpos = meas_w * _info.start_meas - offset + _rcField.l;
		g_dxdraw->tex_Put_Clip( xpos, _rcField.t + 8, &rcStart, SURF_FIELDS, &_rcField );
	}
	if( _info.end_meas   >= 1 )
	{
		xpos = meas_w * _info.end_meas   - offset + _rcField.l - 40;
		g_dxdraw->tex_Put_Clip( xpos, _rcField.t + 8, &rcEnd,   SURF_FIELDS, &_rcField );
	}


	// repeat
	int32_t meas;
	meas = g_pxtn->master->get_repeat_meas();
	if( meas )
	{
		xpos = meas_w * meas - offset + _rcField.l;
		g_dxdraw->tex_Put_Clip( xpos, _rcField.t, &rcRepeat, SURF_FIELDS, &_rcField );
	}

	// last
	meas = g_pxtn->master->get_last_meas();
	if( meas )
	{
		xpos = meas_w * meas - offset + _rcField.l - 40;
		g_dxdraw->tex_Put_Clip( xpos, _rcField.t, &rcLast, SURF_FIELDS, &_rcField );
	}


}

void if_PlayField_SetStartMeas( int32_t meas, bool bExistingDelete )
{
	int32_t max_meas;

	if( meas == -1 )
	{
		_info.start_meas = -1;
		return;
	}

	if( bExistingDelete && meas == _info.start_meas ){
		_info.start_meas = -1;
		return;
	}

	max_meas = g_pxtn->master->get_meas_num();
	if( meas >= max_meas ) meas = max_meas - 1;
	if( meas < 0         ) meas = 0;

	_info.start_meas = meas;

	if( _info.end_meas != -1 && _info.end_meas <= _info.start_meas )
		_info.end_meas = _info.start_meas + 1;
}

void if_PlayField_SetEndMeas( int32_t meas, bool bExistingDelete )
{
	int32_t max_meas;

	if( meas == -1 )
	{
		_info.end_meas = -1;
		return;
	}

	if( bExistingDelete && meas == _info.end_meas ){
		_info.end_meas = -1;
		return;
	}

	max_meas = g_pxtn->master->get_meas_num();
	if( meas >  max_meas ) meas = max_meas;
	if( meas < 1         ) meas = 1;

	_info.end_meas = meas;

	if( _info.start_meas != -1 && _info.start_meas >= _info.end_meas )
		_info.start_meas = _info.end_meas - 1;
}

int32_t if_PlayField_GetStartMeas()
{
	if( _info.start_meas < 0 ) return 0;
	return _info.start_meas;
}

int32_t if_PlayField_GetEndMeas()
{
	return _info.end_meas;
}
