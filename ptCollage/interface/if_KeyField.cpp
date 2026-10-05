
#include <pxtnService.h>
extern pxtnService *g_pxtn;

#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../../Generic/if_gen_Scroll.h"

#include "../UnitFocus.h"

#include "SurfaceNo.h"
#include "Interface.h"
#include "if_BaseField.h"
#include "if_ToolPanel.h"
#include "if_VolumePanel.h"

extern if_gen_Scroll   g_ScrlKeyV;
extern if_gen_Scroll   g_ScrlEventH;

static uint32_t _color_note_body  [ 2 ] =
{
	0xff202020,
	0xff404040,
};

static uint32_t _color_note_cur[ 3 ] =
{
	0xfff08000, // on current
	0xffC06000, // on current tail
	0xff503020,
};

static uint32_t _color_note_sub[ 3 ] =
{
	0xff3060a0,
	0xff205080, // on sub
	0xff203050, // on sub     tail
};

static fRECT _rcField;
static fRECT _rc_view;


void if_KeyField_ResetScroll()
{
	int32_t scroll_y = ( 0x6C00 - EVENTDEFAULT_BASICKEY ) / 0x100 * KEY_HEIGHT + KEY_HEIGHT / 2;
	scroll_y    -= ( _rcField.b - _rcField.t ) / 2;
	g_ScrlKeyV.SetOffset( scroll_y );
}

bool if_KeyField_init( const pxPalette* palpng )
{
	if( !palpng ) return false;

	if( !palpng->get_color( COLORINDEX_note_body_0, &_color_note_body[ 0 ] ) ) return false;
	if( !palpng->get_color( COLORINDEX_note_body_1, &_color_note_body[ 1 ] ) ) return false;

	if( !palpng->get_color( COLORINDEX_note_cur_0 , &_color_note_cur [ 0 ] ) ) return false;
	if( !palpng->get_color( COLORINDEX_note_cur_1 , &_color_note_cur [ 1 ] ) ) return false;
	if( !palpng->get_color( COLORINDEX_note_cur_2 , &_color_note_cur [ 2 ] ) ) return false;

	if( !palpng->get_color( COLORINDEX_note_sub_0 , &_color_note_sub [ 0 ] ) ) return false;
	if( !palpng->get_color( COLORINDEX_note_sub_1 , &_color_note_sub [ 1 ] ) ) return false;
	if( !palpng->get_color( COLORINDEX_note_sub_2 , &_color_note_sub [ 2 ] ) ) return false;

	if_KeyField_ResetScroll();

	return true;
}

void if_KeyField_SetRect( const fRECT *rc_view )
{
	if( rc_view ) _rc_view = *rc_view;

	_rcField.l = _rc_view.l + NAMETRAY_WIDTH  ;
	_rcField.r = _rc_view.r - SCROLLBUTTONSIZE;
	_rcField.t = _rc_view.t + HEADER_H + TOOLPANEL_HEIGHT + UNIT_HEIGHT + KEY_SEPARATOR;

	switch( if_ToolPanel_GetMode() )
	{
	case enum_ToolMode_K:
		if( if_VolumePanel_IsUse() ) _rcField.b = _rc_view.b - SCROLL_HEIGHT - VOLUME_HEIGHT;
		else                         _rcField.b = _rc_view.b - SCROLL_HEIGHT;
		break;
	default: _rcField.b = _rcField.t;
	}
	g_ScrlKeyV.SetRect( &_rcField );
}

fRECT*if_KeyField_GetRect()
{
	return &_rcField;
}





// キーフィールドを表示
void if_KeyField_PutField()
{
	fRECT   rc;
	int32_t ypos  ;
	int32_t ani_no;

	int32_t offset     = g_ScrlKeyV.GetOffset();
	int32_t key_height = _rcField.b - _rcField.t;
	int32_t start      = ( offset               ) / KEY_HEIGHT;
	int32_t end        = ( offset + key_height  ) / KEY_HEIGHT;

	for( int i = start; i <= end; i++ )
	{
		switch( i % 12 )
		{
		case 0: case 1: case 3: case 5: case 7: case 8: case 10: ani_no = 1; break;
		default:                                                 ani_no = 0; break;
		}

		ypos = _rcField.t + i * KEY_HEIGHT - offset;
		rc   = _rcField;
		rc.t = ypos +  1;
		rc.b = ypos + 16;
		if( ypos > rc.t - KEY_HEIGHT && ypos < rc.b )
		{
			if( rc.b > _rcField.b ) rc.b = _rcField.b;
			if( rc.t < _rcField.t ) rc.t = _rcField.t;
			g_dxdraw->FillRect_view( &rc, _color_note_body[ ani_no ] );
		}
	}

	g_ScrlKeyV.Put();
}


static float   _offset_x  ;
static float   _offset_x2 ;
static float   _offset_y  ;
static int32_t _beat_w    ;
static int32_t _beat_clock;

static void _DxDraw_PaintRect( const fRECT *p_rect, const uint32_t* p_color, int32_t depth )
{
	fRECT rect;

	rect.l = p_rect->l - _offset_x + _rcField.l;
	rect.r = p_rect->r - _offset_x + _rcField.l;

	if( depth < 2 )
	{
		rect.t = p_rect->t - _offset_y + _rcField.t - 4;
		rect.b = p_rect->b - _offset_y + _rcField.t + 4;
	}
	else
	{
		rect.t = p_rect->t - _offset_y + _rcField.t - 1;
		rect.b = p_rect->b - _offset_y + _rcField.t + 1;
	}

	if( rect.l <  _rcField.l ) rect.l = _rcField.l;
	if( rect.r >  _rcField.r ) rect.r = _rcField.r;

	if( rect.t <  _rcField.t ) rect.t = _rcField.t;
	if( rect.b >  _rcField.b ) rect.b = _rcField.b;

	if( rect.t >= _rcField.b ) rect.t = _rcField.b - 1;
	if( rect.b <= _rcField.t ) rect.b = _rcField.t + 1;

	g_dxdraw->FillRect_view( &rect, p_color[ depth ] );
}

static void _PutKeyEvent2( int32_t u, bool bActive )
{
	const EVERECORD* p_eve = NULL;

	fRECT     rc    ;
	int32_t   y_now ;
	int32_t   y_next;
	int32_t   depth = 2; // 0: key 1:on 2:off
	int32_t   x_head;
	int32_t   x_tail;

	uint32_t* p_color = bActive ? _color_note_cur : _color_note_sub;

	const pxtnUnit *p_unit = g_pxtn->Unit_Get( u ); if( !p_unit ) return;
	if( !p_unit->get_operated() && u != UnitFocus_Get() ) return;

	y_now   = ( 0x6C00 - EVENTDEFAULT_BASICKEY ) * KEY_HEIGHT / 0x100 + KEY_HEIGHT/2;

	x_tail = rc.l = rc.r = _offset_x;
	rc.t   = rc.b = y_now;

	// 範囲
	if( int32_t eve_num = if_BaseField_Event_get_View_KeyField( u, &p_eve ) )
	{
		for( int32_t e = 0; e < eve_num; e++, p_eve++ )
		{
			rc.t = y_now;
			rc.b = y_now;

			x_head    = p_eve->clock * _beat_w / _beat_clock;

			// ON 区間
			if( x_head >= x_tail )
			{
				rc.r = x_tail;
				_DxDraw_PaintRect( &rc, p_color, depth );
				if( rc.l < x_tail ) rc.l = x_tail;
				depth = 2;
			}

			// OFF区間
			rc.r = x_head;
			if( bActive || depth == 1 ) _DxDraw_PaintRect( &rc, p_color, depth );
			if( rc.l < x_head ) rc.l = x_head;

			if( p_eve->kind == EVENTKIND_ON )
			{
				depth  = 1;
				x_tail = ( p_eve->clock + p_eve->value ) * _beat_w / _beat_clock;
			}
			else if( p_eve->kind == EVENTKIND_KEY )
			{
				// 縦線
				y_next = ( 0x6C00 - EVENTDEFAULT_BASICKEY - ( p_eve->value - 0x6000 ) ) / 0x100  * KEY_HEIGHT + KEY_HEIGHT / 2;
				if( y_next < y_now ) rc.t = y_next;
				else                 rc.b = y_next;
				rc.l  = rc.r;
				rc.r += 2;
				if( bActive ) _DxDraw_PaintRect( &rc, p_color, 2 );

				// 点
				rc.t = y_next  ;
				rc.b = y_next  ;
				if( bActive )
				{
					_DxDraw_PaintRect( &rc, p_color, 0 );
					rc.l = rc.r;
				}
				y_now = y_next;
			}
		}
		if_BaseField_Event_get_View_Unlock();
	}

	// ON 区間
	if( _offset_x2 >= x_tail )
	{
		rc.r = x_tail;
		_DxDraw_PaintRect( &rc, p_color, depth );
		if( rc.l < x_tail ) rc.l = x_tail;
		depth     = 2;
	}
	// OFF区間
	if( bActive )
	{
		rc.r = _offset_x2;
		_DxDraw_PaintRect( &rc, p_color, depth );
	}

}

// キー情報の表示
void if_KeyField_PutEvent()
{
	int32_t max_unit;
	int32_t u;
	int32_t focus;

	focus = UnitFocus_Get();
	if( focus < 0 ) return;

	// 準備
	{
		max_unit    = g_pxtn->Unit_Num();
		_beat_w     = if_BaseField_GetBeatWidth();
		_beat_clock = g_pxtn->master->get_beat_clock();
		_offset_x   = g_ScrlEventH.GetOffset();
		_offset_x2  = _offset_x + _rcField.r - _rcField.l;
		_offset_y   = g_ScrlKeyV.GetOffset();
	}

	// 描画
	for( u = 0; u < max_unit; u++ ){
		if( u != focus ) _PutKeyEvent2( u, false );
	}
	_PutKeyEvent2( focus, true );
}

// キーの縦スクロール調整
void if_KeyField_JustScroll()
{
	int32_t   scroll_y;

	int32_t   u;
	const EVERECORD* p_eve    ;
	const EVERECORD* p_eve_key = NULL;

	u = UnitFocus_Get(); if( u < 0 ) return;

	// 表示領域を更新
	if_KeyField_SetRect( NULL );

	if( int32_t eve_num = if_BaseField_Event_get_View_KeyField( u, &p_eve ) )
	{
		for( int32_t e = 0; e < eve_num; e++, p_eve++ )
		{
			if( p_eve->unit_no == u && p_eve->kind == EVENTKIND_KEY )
			{
				p_eve_key = p_eve;
				break;
			}
		}

		if( p_eve_key ) scroll_y = ( 0x6C00 - EVENTDEFAULT_BASICKEY - (p_eve_key->value - 0x6000) ) / 0x100 * KEY_HEIGHT + KEY_HEIGHT/2;
		else            scroll_y = ( 0x6C00 - EVENTDEFAULT_BASICKEY                               ) / 0x100 * KEY_HEIGHT + KEY_HEIGHT/2;
		scroll_y -= ( _rcField.b - _rcField.t ) / 2;

		g_ScrlKeyV.SetOffset( scroll_y );

		if_BaseField_Event_get_View_Unlock();
	}
}
