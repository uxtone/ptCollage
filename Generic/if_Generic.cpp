
#include "../pxWindows/pxwDx09Draw.h"

static pxwDx09Draw *_dxdraw = NULL;
static int32_t _surf_tenkey     =    0;

fRECT _rcNum[] =
{
	{  0, 96,  8,104},
	{  8, 96, 16,104},
	{ 16, 96, 24,104},
	{ 24, 96, 32,104},
	{ 32, 96, 40,104},
	{ 40, 96, 48,104},
	{ 48, 96, 56,104},
	{ 56, 96, 64,104},
	{ 64, 96, 72,104},
	{ 72, 96, 80,104},

	{  0,104,  8,112},
	{  8,104, 16,112},
	{ 16,104, 24,112},
	{ 24,104, 32,112},
	{ 32,104, 40,112},
	{ 40,104, 48,112},
	{ 48,104, 56,112},
	{ 56,104, 64,112},
	{ 64,104, 72,112},
	{ 72,104, 80,112},
};

#define PUTNUMFLAG_RIGHT 0x10


void if_gen_init( pxwDx09Draw *dxdraw, int32_t surf_tenkey )
{
	_dxdraw          = dxdraw       ;
	_surf_tenkey     = surf_tenkey  ;
}

void if_gen_num6_clip( float x, float y, int32_t value, uint32_t flag, const fRECT *p_rc_clip )
{
	if( !_dxdraw ) return;

	fRECT   rcMinus = {80,96,88,104};
	int32_t tbl0[]  = {1,10,100,1000,10000,100000};
	int32_t tbl9[]  = {9,99,999,9999,99999,999999};

	int32_t a;
	bool    bPut   = false;
	bool    bMinus = false;
	int32_t offset =     0;
	float   xpos;

	if( !(flag&0x0F) ) return;

	if( value > tbl9[(flag&0x0F)-1] )
		value = tbl9[(flag&0x0F)-1];
	if( value < 0    )
	{
		value *= -1;
		bMinus = true;
	}
	
	xpos = x;

	for( offset = (flag&0x0F)-1; offset >= 0; offset-- )
	{
		a     = value/tbl0[offset];
		value = value%tbl0[offset];
		if( bMinus && a && !bPut )
		{
			if( p_rc_clip ) _dxdraw->tex_Put_Clip( xpos - 8, y, &rcMinus, _surf_tenkey, p_rc_clip );
			else            _dxdraw->tex_Put_View( xpos - 8, y, &rcMinus, _surf_tenkey            );
		}
		if( bPut || a || !offset )
		{
			if( p_rc_clip ) _dxdraw->tex_Put_Clip( xpos,     y, &_rcNum[a], _surf_tenkey, p_rc_clip );
			else            _dxdraw->tex_Put_View( xpos,     y, &_rcNum[a], _surf_tenkey            );
		}
		if(          a           ) bPut = true;
		xpos += 8;
	}	
}

void if_gen_num6( float x, float y, int32_t value, uint32_t flag )
{
	if_gen_num6_clip( x, y, value, flag, NULL );
}

void if_gen_float6_clip( float x, float y, float f, const fRECT *p_rc_clip )
{
	if( !_dxdraw ) return;

	fRECT   rcDot  = { 88, 96, 96,104};
	int32_t buf[ 6 ];
	int32_t xpos = 0;
	int32_t l;

	memset( buf, 0, sizeof(int32_t) * 6 );
	l = (int32_t)( f * 100000.0f + 0.5f );
	if( l > 999999 ) l = 999999;
	if( l < 0      ) l = 0     ;
	while( l >= 100000 ){ buf[ 0 ]++; l -= 100000; }
	while( l >=  10000 ){ buf[ 1 ]++; l -=  10000; }
	while( l >=   1000 ){ buf[ 2 ]++; l -=   1000; }
	while( l >=    100 ){ buf[ 3 ]++; l -=    100; }
	while( l >=     10 ){ buf[ 4 ]++; l -=     10; }
	while( l >=      1 ){ buf[ 5 ]++; l -=      1; }
	
	for( int32_t i = 0; i < 6; i++, xpos += 8 )
	{
		if( p_rc_clip ) _dxdraw->tex_Put_Clip( x + xpos, y, &_rcNum[ buf[ i ] ], _surf_tenkey, p_rc_clip );
		else            _dxdraw->tex_Put_View( x + xpos, y, &_rcNum[ buf[ i ] ], _surf_tenkey            );
		if( !xpos )
		{
			xpos += 8;
			if( p_rc_clip ) _dxdraw->tex_Put_Clip( x + xpos, y, &rcDot, _surf_tenkey, p_rc_clip );
			else            _dxdraw->tex_Put_View( x + xpos, y, &rcDot, _surf_tenkey            );
		}
	}
}

float if_gen_float_n( float x, float y, float f, int32_t under_zero )
{
	if( !_dxdraw ) return 0;

	fRECT rcDot  = { 88, 96, 96,104};
	int32_t  val = (int32_t)( f );
	int32_t  number;
	int32_t  k;

	// 整数
	for( k = 100000; k > 1; k /= 10 ){ if( val >= k ) break; }
	for(           ; k;     k /= 10 )
	{
		number  = val    / k;
		_dxdraw->tex_Put_View( x, y, &_rcNum[ number ], _surf_tenkey );
		val    -= number * k;
		x      += 8;
	}

	if( !under_zero ) return x;

	_dxdraw->tex_Put_View( x, y, &rcDot, _surf_tenkey );
	x += 8;

	// 少数以下
	val = 1;
	for( int32_t k = 0; k < under_zero; k++ ){ val *= 10; }
	val = (int32_t)( ( f - (int32_t)( f ) ) * val+0.5 );

	k = 1;
	for( int32_t j = 1; j < under_zero; j++ ){ k *= 10; }

	for( ; k; k /= 10 )
	{
		number  = val    / k;
		_dxdraw->tex_Put_View( x, y, &_rcNum[ number ], _surf_tenkey );
		val    -= number * k;
		x      += 8;
	}

	return x;
}

float if_gen_float_n( float x, float y, float f, int32_t over_zero, int32_t under_zero )
{
	if( !_dxdraw ) return 0;

	fRECT   rcDot = { 88, 96, 96,104};
	int32_t val   = (int32_t)( f );
	int32_t number;

	int32_t max   = 1;
	float   min   = 1;
	int32_t k;

	for( k = 0; k < over_zero ; k++ ){ max *= 10; }
	for( k = 0; k < under_zero; k++ ){ min /= 10; }
	if( f > max - min ) f = (float)( max - min );

	// 整数
	for( k = max / 10; k > 1; k /= 10 )
	{
		if( val >= k ) break;
		x += 8;
	}
	for( ; k; k /= 10 )
	{
		number  = val    / k;
		_dxdraw->tex_Put_View( x, y, &_rcNum[ number ], _surf_tenkey );
		val    -= number * k;
		x      += 8;
	}

	if( !under_zero ) return x;

	_dxdraw->tex_Put_View( x, y, &rcDot, _surf_tenkey );
	x += 8;

	// 少数以下
	val = 1;
	for( k = 0; k < under_zero; k++ ){ val *= 10; }
	val = (int32_t)( ( f - (int32_t)( f ) ) * val+0.5 );

	k = 1;
	for( int32_t j = 1; j < under_zero; j++ ){ k *= 10; }

	for( ; k; k /= 10 )
	{
		number  = val    / k;
		_dxdraw->tex_Put_View( x, y, &_rcNum[ number ], _surf_tenkey );
		val    -= number * k;
		x      += 8;
	}
	return x;
}

void if_gen_tile_h( const fRECT *p_field_rect, const fRECT *p_image_rect, int32_t rect_num, int32_t offset, int32_t surf )
{
	if( !_dxdraw ) return;

	int32_t image_width = (int32_t)( p_image_rect[0].r - p_image_rect[0].l ); if( !image_width ) return;
	int32_t index       = ( offset / image_width ) % rect_num;
	float   x           = p_field_rect->l - (offset % image_width);
	float   y           = p_field_rect->t;

	while( x <= p_field_rect->r )
	{
		_dxdraw->tex_Put_Clip( x, p_field_rect->t, &p_image_rect[index], surf, p_field_rect );
		x += image_width;
		index = (index+1) %rect_num;
	}

}

void if_gen_tile_v( const fRECT *p_field_rect, const fRECT *p_image_rect, int32_t rect_num, int32_t offset, int32_t surf )
{
	if( !_dxdraw ) return;

	int32_t image_height = (int32_t)( p_image_rect[0].b - p_image_rect[0].t ); if( !image_height ) return;
	int32_t index        = ( offset / image_height ) % rect_num;
	float x              = p_field_rect->l;
	float y              = p_field_rect->t - (offset % image_height);

	while( y <= p_field_rect->b )
	{
		_dxdraw->tex_Put_Clip( x, y, &p_image_rect[index], surf, p_field_rect );
		y += image_height;
		index = (index+1) %rect_num;
	}
}

bool if_gen_splash( HWND hwnd, pxwDx09Draw *dxdraw, int32_t surf, float mag )
{
	fRECT rc_logo = {  0,  0, 80 * mag, 16 * mag};
	RECT  rc_client;

	if( dxdraw->tex_load( _T("img"), _T("logo.png"), surf, mag ) < 0 ) return false;

	GetClientRect( hwnd, &rc_client );
	rc_client.right  = (LONG)( rc_client.right  / dxdraw->get_screen_mag() );
	rc_client.bottom = (LONG)( rc_client.bottom / dxdraw->get_screen_mag() );

	dxdraw->SetViewport( 0, 0, (float)rc_client.right, (float)rc_client.bottom, 0, 0 );

	dxdraw->Begin( NULL );
	{
		dxdraw->Fill( 1.0f, 0.0f, 0.0f, 0.0f );
		dxdraw->tex_Put_View(
			((rc_client.right - rc_client.left) - rc_logo.w())/2,
			((rc_client.bottom - rc_client.top) - rc_logo.h())/2,
			&rc_logo, surf );
	}

	dxdraw->End( true );

	return true;
}
