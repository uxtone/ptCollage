
#include <pxStdDef.h>

#include <pxtnService.h>
extern pxtnService *g_pxtn;

#include <pxtonewinXA2.h>
extern pxtonewinXA2* g_strm_xa2;

#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../../Generic/if_gen_Scroll.h"
#include "../../Generic/if_Generic.h"

#include "Interface.h"
#include "SurfaceNo.h"
#include "if_BaseField.h"

#define STARTFIELD_TOP    10
#define STARTFIELD_BOTTOM 26

#define _EVERECORD_NUM 1000


static uint32_t _color_field  = 0xff000010;

static uint32_t _color_scale1 = 0xffF0F0F0; //小節
static uint32_t _color_scale2 = 0xff808080; //拍子
static uint32_t _color_scale3 = 0xff606060;


extern if_gen_Scroll    g_ScrlEventH;

typedef struct
{
	int32_t beat_w  ; // 一小節の幅
	int32_t divi_num; // 拍分割数
	int32_t tgt_beat; // 対象拍(cursor)
}
FIELDINFORMATION;

static FIELDINFORMATION _FieldInfo;
static fRECT             _rcField;

static pxtnEvelist      *_evels = NULL;


// 初期化
bool if_BaseField_init( int32_t beat_w, int32_t divi_num, const pxPalette* palette )
{
	if( !palette ) return false;
	if( !palette->get_color( COLORINDEX_scale_0, &_color_scale1 ) ) return false;
	if( !palette->get_color( COLORINDEX_scale_1, &_color_scale2 ) ) return false;
	if( !palette->get_color( COLORINDEX_scale_2, &_color_scale3 ) ) return false;

	_FieldInfo.beat_w   = beat_w;
	_FieldInfo.divi_num = divi_num;

	_evels = new pxtnEvelist( NULL, NULL, NULL, NULL );
	if( !_evels->Allocate( _EVERECORD_NUM ) ) return false;
	return true;
}

void if_BaseField_Release()
{
	SAFE_DELETE( _evels );
}

void if_BaseField_SetParameter( int32_t beat_w, int32_t divi_num )
{
	if( beat_w   )
	{
		int32_t offset      = g_ScrlEventH.GetOffset();
		int32_t old_beat_w  = _FieldInfo.beat_w;
		_FieldInfo.beat_w   = beat_w;
		g_ScrlEventH.SetOffset( ( (int32_t)offset * beat_w / old_beat_w ) );
	}

	if( divi_num )
	{
		_FieldInfo.divi_num = divi_num;
	}
}


int32_t if_BaseField_GetBeatWidth()
{
	return _FieldInfo.beat_w;
}

int32_t if_BaseField_GetDivideWidth()
{
	int32_t divi_w = _FieldInfo.beat_w / _FieldInfo.divi_num;
	if( !divi_w ) divi_w = 1;
	return divi_w;
}

void if_BaseField_SetRect( const fRECT *rc_view )
{
	_rcField.l = rc_view->l + NAMETRAY_WIDTH  ;
	_rcField.r = rc_view->r - SCROLLBUTTONSIZE;
	_rcField.t = rc_view->t + HEADER_H    ;
	_rcField.b = rc_view->b - SCROLLBUTTONSIZE;

	g_ScrlEventH.SetRect( &_rcField );
}

fRECT*if_BaseField_GetRect()
{
	return &_rcField;
}

void if_BaseField_Put_Field()
{
	g_dxdraw->FillRect_view( &_rcField, _color_field );
}

// フィールドの表示
void if_BaseField_Put_Grid()
{
	fRECT rc;

	int32_t view_w = (int32_t)( _rcField.r - _rcField.l );
	int32_t meas_w = _FieldInfo.beat_w * g_pxtn->master->get_beat_num();
	int32_t offset = g_ScrlEventH.GetOffset();

	int32_t start;
	int32_t end;
	int32_t i;
	float   xpos;

	// 対象拍
	if( !g_strm_xa2->tune_is_sampling() && _FieldInfo.tgt_beat >= 0 )
	{
		int32_t divi_w_x4 = _FieldInfo.beat_w * 4/ _FieldInfo.divi_num;
		start         = (int32_t)( ( _FieldInfo.tgt_beat * _FieldInfo.beat_w ) - offset + _rcField.l );
		rc.t          = _rcField.t + MEASNO_HEIGHT + STARTFIELD_HEIGHT + 1;
		rc.b          = _rcField.b -  0;

		int32_t inc = 1;

		switch( _FieldInfo.divi_num )
		{
		case 12: inc = 4; break;
		case 16: inc = 4; break;
		case 24: inc = 4; break;
		case 48: inc = 4; break;
		}

		for( i = inc; i < _FieldInfo.divi_num; i += inc )
		{
			xpos = (float)( divi_w_x4 * i / 4 + start );
			if( xpos >= _rcField.l && xpos < _rcField.r )
			{
				rc.l = xpos;
				rc.r = rc.l + 1;
				g_dxdraw->FillRect_view( &rc, _color_scale3 );
			}
		}
	}


	// フィールド線
	rc.l = _rcField.l;
	rc.r = _rcField.r;
	rc.t = _rcField.t + TOOLPANEL_HEIGHT - 1;
	rc.b = _rcField.t + TOOLPANEL_HEIGHT - 0;
	g_dxdraw->FillRect_view( &rc, _color_scale2 );

	// 拍線
	start = (int32_t)( offset + _FieldInfo.beat_w - 1 ) / _FieldInfo.beat_w;
	end   = (int32_t)( offset + view_w                ) / _FieldInfo.beat_w;
	rc.t  = _rcField.t +  MEASNO_HEIGHT + STARTFIELD_HEIGHT;
	rc.b  = _rcField.b -  0;

	for( i = start; i <= end; i++ )
	{
		xpos = _FieldInfo.beat_w * i - offset + _rcField.l;
		rc.l = xpos;
		rc.r = rc.l + 1;
		g_dxdraw->FillRect_view( &rc, _color_scale2 );
	}

	// 小節線
	start     = (int32_t)( offset + meas_w - 1 ) / meas_w;
	end       = (int32_t)( offset + view_w     ) / meas_w;
	rc.t      = _rcField.t +  1;
	rc.b      = _rcField.b -  0;

	for( i = start; i <= end; i++ )
	{
		xpos = meas_w * i - offset + _rcField.l;
		rc.l = xpos;
		rc.r = rc.l       + 1;
		rc.t = _rcField.t + 1;
		g_dxdraw->FillRect_view( &rc, _color_scale1 );
	}

	// 小節番号
	start = (int32_t)( offset + (meas_w-1) - 32 ) / meas_w;
	end   = (int32_t)( offset + view_w          ) / meas_w;
	rc.t  = _rcField.t + 1;
	rc.b  = rc.t + 8   + 1;

	for( i = start; i <= end; i++ ){

		xpos = meas_w * i - offset + _rcField.l;
		rc.l = xpos + 1;
		rc.r = rc.l + 24 + 1;

		g_dxdraw->FillRect_view( &rc, _color_scale3 );
		if_gen_num6_clip( xpos + 1, _rcField.t + 1, i, 3, &_rcField );
	}
}


bool if_BaseField_Action( float cur_x, float cur_y )
{
	int32_t offset   = g_ScrlEventH.GetOffset();
	int32_t beat_num = g_pxtn->master->get_beat_num();
	int32_t meas_num = g_pxtn->master->get_meas_num();

	g_ScrlEventH.SetSize( _FieldInfo.beat_w * beat_num * (meas_num + 2) );
	g_ScrlEventH.SetMove( _FieldInfo.beat_w, _FieldInfo.beat_w * beat_num );

	if( g_strm_xa2->tune_is_sampling() ) return false;

	if( cur_y <  _rcField.t ||
		cur_y >= _rcField.b ||
		cur_x <  _rcField.l ||
		cur_x >= _rcField.r ) return false;

	_FieldInfo.tgt_beat = (int32_t)( cur_x + offset - _rcField.l ) / _FieldInfo.beat_w;


	return true;
}


static bool _b_lock = false;

void if_BaseField_Event_get_View_Unlock()
{
	_b_lock = false;
}

int32_t if_BaseField_Event_get_View( int32_t unit_no, int32_t kind, const EVERECORD** pp_eve )
{
	int32_t          beat_clock = g_pxtn->master->get_beat_clock();
	int32_t          offset_x   = g_ScrlEventH.GetOffset();
	int32_t          clock1     = (int32_t)( offset_x                           ) * beat_clock / _FieldInfo.beat_w;
	int32_t          clock2     = (int32_t)( offset_x + _rcField.r - _rcField.l ) * beat_clock / _FieldInfo.beat_w;

	const EVERECORD* p          = g_pxtn->evels->get_Records();
	const EVERECORD* prev       = NULL;

	*pp_eve = NULL;

	// 最初の該当イベント
	for( int i = 0; p; p = p->next, i++ )
	{
		if( p->unit_no == unit_no && p->kind == kind )
		{
			if( Evelist_Kind_IsTail( kind ) && p->clock + p->value > clock1 ) break;
			else if(                           p->clock            > clock1 ) break;
			prev = p;
		}
	}

	if(      prev               ) p = prev;
	else if( !p                 ) return 0;
	else if( p->clock >= clock2 ) return 0;

	int count = 0;
	if( !_b_lock )
	{
		_b_lock = true;

		_evels->Linear_Start();

		for( ; count < _EVERECORD_NUM && p; p = p->next )
		{
			if( p->clock >= clock2 ) break;
			if( p->unit_no == unit_no && p->kind == kind )
			{
				_evels->Linear_Add_i( p->clock, (unsigned char)unit_no, (unsigned char)kind, p->value );
				count++;
			}
		}

		_evels->Linear_End( false );
		*pp_eve = _evels->get_Records();

		if( !count ) _b_lock = false;
	}
	return count;
}

int32_t if_BaseField_Event_get_View( int32_t unit_no, const EVERECORD** pp_eve )
{
	int32_t          beat_clock = g_pxtn->master->get_beat_clock();
	int32_t          offset_x   = g_ScrlEventH.GetOffset();
	int32_t          clock1     = (int32_t)( offset_x                           ) * beat_clock / _FieldInfo.beat_w;
	int32_t          clock2     = (int32_t)( offset_x + _rcField.r - _rcField.l ) * beat_clock / _FieldInfo.beat_w;

	const EVERECORD* p          = g_pxtn->evels->get_Records();
	const EVERECORD* prev       = NULL;

	*pp_eve = NULL;

	// 最初の該当イベント
	int32_t old_clock = 0;
	int32_t c;
	for( int32_t i = 0; p; p = p->next, i++ )
	{
		if( p->unit_no == unit_no )
		{
			if( Evelist_Kind_IsTail( p->kind ) ) c = p->clock + p->value;
			else                                 c = p->clock;
			if(                                  c > clock1    ) break;
			if( !prev ||                         c > old_clock ) prev = p;
		}
	}

	if(      prev               ) p = prev;
	else if( !p                 ) return 0;
	else if( p->clock >= clock2 ) return 0;

	int count = 0;
	if( !_b_lock )
	{
		_b_lock = true;

		_evels->Linear_Start();

		for( ; count < _EVERECORD_NUM && p; p = p->next )
		{
			if( p->clock >= clock2 ) break;
			if( p->unit_no == unit_no )
			{
				_evels->Linear_Add_i( p->clock, (unsigned char)unit_no, (unsigned char)p->kind, p->value );
				count++;
			}
		}

		_evels->Linear_End( false );
		*pp_eve = _evels->get_Records();

		if( !count ) _b_lock = false;
	}
	return count;
}

int32_t if_BaseField_Event_get_View_KeyField( int32_t unit_no, const EVERECORD** pp_eve )
{
	int32_t         beat_clock = g_pxtn->master->get_beat_clock();
	int32_t         offset_x   = g_ScrlEventH.GetOffset();
	int32_t         clock1     = (int32_t)( offset_x                           ) * beat_clock / _FieldInfo.beat_w;
	int32_t         clock2     = (int32_t)( offset_x + _rcField.r - _rcField.l ) * beat_clock / _FieldInfo.beat_w;

	const EVERECORD* p          = g_pxtn->evels->get_Records();
	const EVERECORD* prev_on    = NULL;
	const EVERECORD* prev_key   = NULL;
	int32_t              i_on       = 0;
	int32_t              i_key      = 0;

	// 最初の該当イベント
	for( int32_t i = 0; p; p = p->next, i++ )
	{
		if( p->unit_no == unit_no )
		{
			if(      p->kind == EVENTKIND_ON  )
			{
				if( p->clock + p->value > clock1 ) break;
				prev_on = p;
				i_on    = i;
			}
			else if( p->kind == EVENTKIND_KEY )
			{
				if( p->clock            > clock1 ) break;
				prev_key = p;
				i_key    = i;
			}
		}
	}


	// 一つ前イベント
	if( prev_on && prev_key )
	{
		if( i_on < i_key ) p = prev_on ;
		else               p = prev_key;
	}
	else if( prev_on     ) p = prev_on ;
	else if( prev_key    ) p = prev_key;
	else if( !p                 ) return 0;
	else if( p->clock >= clock2 ) return 0;

	int count = 0;
	if( !_b_lock )
	{
		_b_lock = true;

		_evels->Linear_Start();

		for( ; count < _EVERECORD_NUM && p; p = p->next )
		{
			if( p->clock >= clock2 ) break;
			if( p->unit_no == unit_no )
			{
				if( p->kind == EVENTKIND_ON || p->kind == EVENTKIND_KEY )
				{
					_evels->Linear_Add_i( p->clock, (unsigned char)unit_no, p->kind, p->value );
					count++;
				}
			}
		}

		_evels->Linear_End( false );
		*pp_eve = _evels->get_Records();

		if( !count ) _b_lock = false;
	}

	return count;
}


// 現在表示するべきイベントのポインタと数を取得する
int32_t if_BaseField_Event_get_Value( float cur_x, int32_t unit_no, int32_t kind )
{
	int32_t clock;
	int32_t beat_clock;
	int32_t offset_x = g_ScrlEventH.GetOffset();

	if( cur_x < _rcField.l ) cur_x = _rcField.l;

	beat_clock = g_pxtn->master->get_beat_clock();
	clock      = (int32_t)( offset_x + cur_x - _rcField.l ) * beat_clock / _FieldInfo.beat_w;

	return g_pxtn->evels->get_Value( clock, (unsigned char)unit_no, (unsigned char)kind );
}


