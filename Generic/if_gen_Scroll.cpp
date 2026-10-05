
#include "../pxWindows/pxwDx09Draw.h"

#include "./if_gen_Scroll.h"
#include "./if_Generic.h"


//////////////////////////////////////////////////////////////
// ■コンストラクタ／デストラクタ ////////////////////////////
//////////////////////////////////////////////////////////////

if_gen_Scroll::if_gen_Scroll()
{
	_act_no       = SCROLLACTION_GRAY;
	_ani_no       = 0;

	_size         = 16;
	_offset       = 0;

	_start_cur    = 0;
	_start_offset = 0;
	_cursor_move  = SCROLLBUTTONSIZE;
	_body_move    = SCROLLBUTTONSIZE;
	_post_move    = 0;
	_counter      = 0;

	memset( &_vw_rect, 0, sizeof(_vw_rect) );

	_b_frame_update = false;
}

if_gen_Scroll::~if_gen_Scroll()
{
}

// 初期化
void if_gen_Scroll::Initialize( pxwDx09Draw *dxdraw, int32_t surf, bool bVertical )
{
	_dxdraw    = dxdraw   ;
	_surf      = surf     ;
	_bVertical = bVertical;
}

// オフセットを設定
void    if_gen_Scroll::SetOffset( int32_t offset )
{
	if( _offset != offset ) _b_frame_update = true;
	_offset = offset; if( _offset < 0    ) _offset = 0;
}
int32_t if_gen_Scroll::GetOffset(){ return _offset; }
void    if_gen_Scroll::SetSize  ( int32_t size   ){ _size   = size  ; if( _size > 300000 ) _size   = 0; }
int32_t if_gen_Scroll::GetSize  (){ return _size;   }

void if_gen_Scroll::SetMove( int32_t cursor_move, int32_t body_move )
{
	_cursor_move  = cursor_move;
	_body_move    = body_move;
}

// ワンタッチスクロール（ move が -2 なら _cursor_move * -2 移動する）
void if_gen_Scroll::Scroll( int32_t view_width, int32_t move )
{
	int32_t max_offset;

	if( view_width < _size ) max_offset = _size - view_width;
	else                     max_offset = 0;

	_offset += (int32_t)( _cursor_move * move );

	if( _offset < 0          ) _offset = 0;
	if( _offset > max_offset ) _offset = max_offset;
}

void if_gen_Scroll::PostMove( int32_t move )
{
	if( _act_no == SCROLLACTION_ENABLE ) _post_move = move;
	else                                 _post_move = 0;
}

void if_gen_Scroll::SetRect( fRECT *vw_rect )
{
	int32_t view_width;
	int32_t max_offset  = 0;
	int32_t knob_width  = 0;
	int32_t knob_offset = 0;

	int32_t old_offset = _offset;

	_vw_rect        = *vw_rect;
	_b_frame_update = false   ;


	if( !_bVertical ) view_width = (int32_t)( _vw_rect.r - _vw_rect.l );
	else              view_width = (int32_t)( _vw_rect.b - _vw_rect.t );

	if( view_width < _size ) max_offset = _size - view_width;
	else                        _offset = 0;

	if( _offset <          0 ) _offset =          0;
	if( _offset > max_offset ) _offset = max_offset;

	if( _size )
	{
		knob_width  = view_width * (view_width - SCROLLBUTTONSIZE*2 ) / _size;
		knob_offset = _offset    * (view_width - SCROLLBUTTONSIZE*2 ) / _size;
	}

	if( !_bVertical )
	{
		_rcBtn1.l = vw_rect->l;
		_rcBtn1.r = vw_rect->l + SCROLLBUTTONSIZE;
		_rcBtn1.t = vw_rect->b;
		_rcBtn1.b = vw_rect->b + SCROLLBUTTONSIZE;

		_rcBtn2.l = _vw_rect.r - SCROLLBUTTONSIZE;
		_rcBtn2.r = _vw_rect.r;
		_rcBtn2.t = _vw_rect.b;
		_rcBtn2.b = _vw_rect.b + SCROLLBUTTONSIZE;

		_rcKnob.l = _vw_rect.l + SCROLLBUTTONSIZE + knob_offset              -1;
		_rcKnob.r = _vw_rect.l + SCROLLBUTTONSIZE + knob_offset + knob_width +1;
		_rcKnob.t = _vw_rect.b;
		_rcKnob.b = _vw_rect.b + SCROLLBUTTONSIZE;
	}
	else
	{
		_rcBtn1.l = vw_rect->r;
		_rcBtn1.r = vw_rect->r  + SCROLLBUTTONSIZE;
		_rcBtn1.t = vw_rect->t;
		_rcBtn1.b = vw_rect->t  + SCROLLBUTTONSIZE;

		_rcBtn2.l = _vw_rect.r;
		_rcBtn2.r = _vw_rect.r + SCROLLBUTTONSIZE;
		_rcBtn2.t = _vw_rect.b - SCROLLBUTTONSIZE;
		_rcBtn2.b = _vw_rect.b;

		_rcKnob.l = _vw_rect.r;
		_rcKnob.r = _vw_rect.r  + SCROLLBUTTONSIZE;
		_rcKnob.t = _vw_rect.t  + SCROLLBUTTONSIZE + knob_offset              -1;
		_rcKnob.b = _vw_rect.t  + SCROLLBUTTONSIZE + knob_offset + knob_width +1;
	}

	if( _offset != old_offset ) _b_frame_update = true;
}

// アクション
bool if_gen_Scroll::Action( int32_t cur_x, int32_t cur_y, bool b_l_clk, bool b_l_trg )
{
	bool b_ret = false;

	int32_t view_width;
	int32_t max_offset  = 0;
	int32_t knob_width  = 0;
	int32_t knob_offset = 0;

	int32_t old_offset  = _offset;

	if( !_bVertical ) view_width = (int32_t)( _vw_rect.r - _vw_rect.l );
	else              view_width = (int32_t)( _vw_rect.b - _vw_rect.t );

	if( view_width < _size )
	{
		max_offset = _size - view_width;
		if( view_width > 2 * SCROLLBUTTONSIZE ) Scroll( view_width, _post_move );
	}
	else
	{
		_offset = 0;
	}
	_post_move  = 0;

	if( _size )
	{
		knob_width  = view_width * (view_width - SCROLLBUTTONSIZE*2 ) / _size;
		knob_offset = _offset    * (view_width - SCROLLBUTTONSIZE*2 ) / _size;
	}

	if( view_width <= 2 * SCROLLBUTTONSIZE ) goto term;

	if( !max_offset )
	{
		_ani_no = 0;
		_act_no = SCROLLACTION_GRAY;
		goto term;
	}

	switch( _act_no )
	{
	// 無効
	case SCROLLACTION_GRAY:
		_ani_no = 0;
		if( view_width < _size ) _act_no = SCROLLACTION_ENABLE;
		else break;

	// 有効
	case SCROLLACTION_ENABLE:
		_ani_no = 1;
		if( view_width >= _size )
		{
			_act_no   = SCROLLACTION_GRAY;
			_offset = 0;
			break;
		}
		// クリック／ドラッグ
		if( b_l_trg )
		{
			if( !knob_width ) knob_width = 1;

			// ←
			if(       cur_x >= _rcBtn1.l && cur_x <  _rcBtn1.r &&
					  cur_y >= _rcBtn1.t && cur_y <  _rcBtn1.b )
			{
				_act_no    = SCROLLACTION_LEFT;
				_ani_no    =  2;
				_offset   -= _cursor_move;
				_counter   =  0;
				b_ret      = true;
			}
			// →
			else if( cur_x >= _rcBtn2.l && cur_x <  _rcBtn2.r &&
					 cur_y >= _rcBtn2.t && cur_y <  _rcBtn2.b )
			{
				_act_no    = SCROLLACTION_RIGHT;
				_ani_no    =  2;
				_offset   += _cursor_move;
				_counter   =  0;
				b_ret      = true;
			}
			// Drag
			else if( cur_x >= _rcKnob.l && cur_x <  _rcKnob.r &&
					 cur_y >= _rcKnob.t && cur_y <  _rcKnob.b )
			{
				_ani_no         =  2;
				_act_no         = SCROLLACTION_DRAG;
				_start_offset   = _offset;
				if( !_bVertical ) _start_cur = cur_x;
				else              _start_cur = cur_y;
				b_ret   = true;
			}
			// click
			else if( cur_x >= _rcBtn1.l && cur_x <  _rcBtn2.r &&
					 cur_y >= _rcBtn1.t && cur_y <  _rcBtn2.b )
			{
				_ani_no    =  2;
				_act_no         = SCROLLACTION_body;
				if( !_bVertical )
				{
					if( cur_x <= _rcKnob.l ) _offset -= _body_move;
					else                     _offset += _body_move;
				}else{
					if( cur_y <= _rcKnob.t ) _offset -= _body_move;
					else                     _offset += _body_move;
				}
				b_ret   = true;
			}

		}
		break;

	// ドラッグ中
	case SCROLLACTION_DRAG:

		b_ret = true;
		if( b_l_clk )
		{
			int32_t drag;
			if( !_bVertical ) drag = ( cur_x - _start_cur ) * _size / (view_width - 32);
			else              drag = ( cur_y - _start_cur ) * _size / (view_width - 32);
			_offset = _start_offset + drag;
		}
		else
		{
			if( view_width >= _size ) _act_no = SCROLLACTION_GRAY;
			else                      _act_no = SCROLLACTION_ENABLE;
		}
		break;

	// ←押し
	case SCROLLACTION_LEFT:
		b_ret   = true;
		if( b_l_clk &&
			cur_x >= _rcBtn1.l && cur_x <  _rcBtn1.r &&
			cur_y >= _rcBtn1.t && cur_y <  _rcBtn1.b )
		{
			_counter++;
			if( _counter > 20 )
			{
				_counter = 18;
				_offset -= _cursor_move;
			}
		}
		else
		{
			if( view_width >= _size ) _act_no = SCROLLACTION_GRAY  ;
			else                      _act_no = SCROLLACTION_ENABLE;
		}
		break;

	// →押し
	case SCROLLACTION_RIGHT:
		b_ret   = true;
		if( b_l_clk &&
			cur_x >= _rcBtn2.l && cur_x <  _rcBtn2.r &&
			cur_y >= _rcBtn2.t && cur_y <  _rcBtn2.b )
		{
			_counter++;
			if( _counter > 20 )
			{
				_counter = 18;
				_offset += _cursor_move;
			}
		}
		else
		{
			if( view_width >= _size ) _act_no = SCROLLACTION_GRAY;
			else                      _act_no = SCROLLACTION_ENABLE;
		}
		break;

	// body
	case SCROLLACTION_body:
		if( b_l_clk ) b_ret = true;
		else _act_no = SCROLLACTION_ENABLE;
		break;
	}

	if( _offset < 0          ) _offset = 0;
	if( _offset > max_offset ) _offset = max_offset;

term:
	if( old_offset != _offset )
		_b_frame_update = true;

	return b_ret;
}

bool if_gen_Scroll::is_frame_update() const
{
	return _b_frame_update;
}

void if_gen_Scroll::Put()
{
	fRECT rcItem;

	if( !_bVertical )
	{
		fRECT rc_left[] =
		{
			{256,496,272,512},
			{272,496,288,512},
			{288,496,304,512},
		};

		fRECT rc_right[] =
		{
			{384,496,400,512},
			{400,496,416,512},
			{416,496,432,512},
		};

		fRECT rcBodyH = {256,448,448,464};

		fRECT rcKnobH[] =
		{
			{  0,  0,  0,  0},
			{256,464,448,480},
			{256,480,448,496},
		};

		rcItem.l = _rcBtn1.r;
		rcItem.r = _rcBtn2.l;
		rcItem.t = _rcBtn1.t;
		rcItem.b = _rcBtn1.b;
		if_gen_tile_h( &rcItem, &rcBodyH,          1, 0, _surf );

		rcItem   = _rcKnob;
		rcItem.l += 1;
		rcItem.r -= 1;
		if( rcItem.r <= rcItem.l ) rcItem.r = rcItem.l + 1;
		if_gen_tile_h( &rcItem, &rcKnobH[_ani_no], 1, 0, _surf );

		_dxdraw->tex_Put_View( _rcBtn1.l, _rcBtn1.t, &rc_left [_ani_no], _surf );
		_dxdraw->tex_Put_View( _rcBtn2.l, _rcBtn2.t, &rc_right[_ani_no], _surf );

	}
	// 垂直
	else
	{
		fRECT rc_up[] =
		{
			{496,256,512,272},
			{496,272,512,288},
			{496,288,512,304},
		};

		fRECT rc_down[] =
		{
			{496,384,512,400},
			{496,400,512,416},
			{496,416,512,432},
		};

		fRECT rcBodyV = {448,256,464,448};

		fRECT rcKnobV[] =
		{
			{  0,  0,  0,  0},
			{464,256,480,448},
			{480,256,496,448},
		};

		rcItem.l = _rcBtn1.l;
		rcItem.r = _rcBtn1.r;
		rcItem.t = _rcBtn1.b;
		rcItem.b = _rcBtn2.t;
		if_gen_tile_v(   &rcItem, &rcBodyV,          1, 0, _surf );
		rcItem   = _rcKnob;
		rcItem.t += 1;
		rcItem.b -= 1;
		if( rcItem.b <= rcItem.t ) rcItem.b = rcItem.t + 1;
		if_gen_tile_v(   &rcItem, &rcKnobV[_ani_no], 1, 0, _surf );

		_dxdraw->tex_Put_View( _rcBtn1.l, _rcBtn1.t, &rc_up  [_ani_no], _surf );
		_dxdraw->tex_Put_View( _rcBtn2.l, _rcBtn2.t, &rc_down[_ani_no], _surf );
	}

}