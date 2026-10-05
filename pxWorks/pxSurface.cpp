
#include "./pxStrT.h"
#include "./pxStr.h"
#include "./pxMem.h"

#include "./pxSurface.h"

static bool _copy_argb_with_scale( uint32_t* p_buf_dst, const uint32_t* p_buf_src, int32_t src_w, int32_t src_h, float scale )
{
	const uint32_t* ps_row = p_buf_src;
	const uint32_t* ps     = NULL     ;
	;     uint32_t* pd     = p_buf_dst;

	if( scale >= 1.0f )
	{
		int32_t mag_i = (int32_t)scale;

		for( int y = 0; y < src_h; y++ )
		{
			for( int ym = 0; ym < mag_i; ym++ )
			{
				ps = ps_row;
				for( int x = 0; x < src_w; x++, ps++ )
				{
					for( int xm = 0; xm < mag_i; xm++, pd++ ) *pd = *ps;
				}
			}
			ps_row += src_w;
		}
	}
	else if( scale == 0.5f )
	{
		int32_t h      = (int32_t)( src_h * scale );
		int32_t w      = (int32_t)( src_w * scale );
		int32_t y_skip = (int32_t)(     1 / scale );

		for( int y = 0; y < h; y++ )
		{
			ps = ps_row;
			for( int x = 0; x < w; x++, pd++, ps+=2 )
			{
				*pd = *ps;
			}
			ps_row += src_w * y_skip;
		}
	}
	else return false;

	return true;
}

pxSurface::pxSurface()
{
	_edit_unique =    0;
	_p_buf       = NULL;
	_w           =    0;
	_h           =    0;
	_row_size    =    0;
	_depth       =    0;
	_palette     = NULL;
	_png_path    = NULL;
}

void pxSurface::release_buffer()
{
	pxMem_free ( (void**)&_p_buf    );
	pxStrT_free(         _png_path );

	_w          = 0;
	_h          = 0;
	_depth      = 0;
	_row_size   = 0;

	SAFE_DELETE( _palette );

	edit_unique_update();
}



pxSurface::~pxSurface()
{
	release_buffer();
}

bool pxSurface::is_bitmap() const
{
	if( _p_buf ) return true;
	return false;
}


bool pxSurface::create( int32_t w, int32_t h )
{
	if( !w || !h ) return false;

	release_buffer();

	bool      b_ret     = false;
	int32_t   byte_size = w * h * sizeof(uint32_t);

	if( !pxMem_zero_alloc( (void**)&_p_buf, byte_size ) ) goto term;

	_w        = w;
	_h        = h;
	_depth    = 8;
	_row_size = w * sizeof(uint32_t);

	edit_unique_update();

	b_ret = true;
term:
	if( !b_ret ) release_buffer();
	return b_ret;
}

bool pxSurface::fill_rgba( const sRECT* rc_dst, uint32_t rgba )
{
	if( !_p_buf ) return false;

	sRECT rc1 = *rc_dst;

	if( rc1.l <  0 ) rc1.l =  0;
	if( rc1.t <  0 ) rc1.t =  0;
	if( rc1.r > _w ) rc1.r = _w;
	if( rc1.b > _h ) rc1.b = _h;

	uint32_t* p_row_dst = (uint32_t*)_p_buf;
	uint32_t* p_d;

	p_row_dst += _w * rc1.t;

	for( int y = rc1.t; y < rc1.b; y++ )
	{
		p_d = p_row_dst + rc1.l;
		for( int x = rc1.l; x < rc1.r; x++, p_d++ ) *p_d = rgba;
		p_row_dst += _w;
	}

	edit_unique_update();
	return true;
}


bool pxSurface::fill_rgba( uint32_t rgba )
{
	if( !_p_buf ) return false;

	uint32_t* p = (uint32_t*)_p_buf;

	for( int y = 0; y < _h; y++ )
	{
		for( int x = 0; x < _w; x++, p++ ) *p = rgba;
	}

	edit_unique_update();
	return true;
}

void     pxSurface::edit_unique_update (){ _edit_unique = _sttc_edit_unique; _sttc_edit_unique++; }
uint32_t pxSurface::edit_unique_get    () const { return _edit_unique; }

uint32_t*   pxSurface::get_buf_vari()
{
	return (uint32_t*)_p_buf;
}

const void* pxSurface::get_buf_const() const
{
	return (uint32_t*)_p_buf;
}

int32_t     pxSurface::get_pitch            () const
{
	if( !_p_buf ) return 0;
	return _row_size;
}

bool pxSurface::get_size( int32_t* pw, int32_t* ph ) const
{
	if( !_p_buf ) return false;

	if( pw ) *pw = _w;
	if( ph ) *ph = _h;
	return true;
}

#if 0 // now inline in pxSurface.h: const uxDS& get_png_path() const
bool pxSurface::get_png_path( const uxDS * p_ref_png_path ) const
{
	if( !_png_path     ) return false;
	if( p_ref_png_path ) *p_ref_png_path = _png_path;
	return true;
}
#endif


bool pxSurface::set_png_path( const uxDS& ref_png_path )
{
	if( !ref_png_path ) return false;
	return pxStrT_copy_allocate( _png_path, ref_png_path );
}

bool pxSurface::copy_from( const pxSurface* src )
{
	if( !src ) return false;

	if( src->_p_buf && _edit_unique == src->_edit_unique ) return true;

	release_buffer();

	bool b_ret = false;

	_w        = src->_w       ;
	_h        = src->_h       ;
	_depth    = src->_depth   ;
	_row_size = src->_row_size;

	if( !pxMem_zero_alloc( (void**)&_p_buf,              _row_size * _h ) ) goto term;
	memcpy(                         _p_buf, src->_p_buf, _row_size * _h );

	if( src->_png_path && !pxStrT_copy_allocate( _png_path, src->_png_path ) ) goto term;

	_edit_unique = src->_edit_unique;

	b_ret = true;
term:
	if( !b_ret ) release_buffer();
	return b_ret;
}

bool pxSurface::make_scaled_rgba( uint32_t** p_ref_buf, int32_t* pw, int32_t* ph, float scene_mag ) const
{
	if( !_p_buf ) return false;

	uint32_t* p_buf = NULL;
	bool      b_ret = false;
	int32_t   w     = (int32_t)( _w * scene_mag );
	int32_t   h     = (int32_t)( _h * scene_mag );

	if( !pxMem_zero_alloc( (void**)&p_buf, sizeof(uint32_t) * w * h ) ) goto term;

	if( !_copy_argb_with_scale( (uint32_t*)p_buf, (const uint32_t*)_p_buf, _w, _h, scene_mag ) ) goto term;

	*p_ref_buf = p_buf;
	*pw        = w    ;
	*ph        = h    ;

	b_ret = true;
term:
	if( !b_ret ) pxMem_free( (void**)&p_buf );

	return b_ret;
}

bool pxSurface::resize_argb( float scale )
{
	if( !_p_buf ) return false;

	bool     b_ret    = false;
	uint8_t* p_new    = NULL ;
	int32_t  new_w    = (int32_t)( scale * _w );
	int32_t  new_h    = (int32_t)( scale * _h );
	int32_t  new_size = new_w * new_h * sizeof(uint32_t);

	if( !pxMem_zero_alloc( (void**)&p_new, new_size ) ) return false;

	if( !_copy_argb_with_scale( (uint32_t*)p_new, (const uint32_t*)_p_buf, _w, _h, scale ) ) goto term;

	pxMem_free( (void**)&_p_buf );
	_p_buf    = p_new;
	_w        = new_w;
	_h        = new_h;
	_row_size = _w * sizeof(uint32_t);

	edit_unique_update();

	b_ret = true;
term:
	if( !b_ret ) pxMem_free( (void**)&p_new );

	return b_ret;
}

bool pxSurface::blt_rgba(
	const pxSurface* src_rgba,
	const sRECT*     rc_src,
	int32_t          dst_x ,
	int32_t          dst_y ,
	pxALPHABLEND     alpha_blend )
{
	if( !_p_buf     ) return false;
	if( !src_rgba   ) return false;
	if( dst_x >= _w ) return false;
	if( dst_y >= _h ) return false;

	bool      b_ret = false;
	sRECT     rc_s  = *rc_src;
	uint32_t*       p_dst = (uint32_t*)          _p_buf;
	const uint32_t* p_src = (uint32_t*)src_rgba->_p_buf;

	if( dst_x < 0 ){ rc_s.l -= dst_x; dst_x = 0; }
	if( dst_y < 0 ){ rc_s.b -= dst_y; dst_y = 0; }

	{
		int32_t over_x = ( dst_x + rc_s.w() ) - _w; if( over_x > 0 ) rc_s.r -= over_x;
		int32_t over_y = ( dst_y + rc_s.h() ) - _h; if( over_y > 0 ) rc_s.b -= over_y;
	}

	if( rc_s.w() <= 0 ) return false;
	if( rc_s.h() <= 0 ) return false;

	const uint32_t* p_src_row = &p_src[ rc_s.t * src_rgba->_w + rc_s.l ];
	uint32_t*       p_dst_row = &p_dst[ dst_y  *           _w + dst_x  ];
	const uint32_t* p_s       = p_src_row;
	uint32_t*       p_d       = p_dst_row;

	switch( alpha_blend )
	{
	case pxALPHABLEND_ignore:

		for( int src_y = rc_s.t; src_y < rc_s.b; src_y++ )
		{
			p_s = p_src_row;
			p_d = p_dst_row;
			for( int src_x = rc_s.l; src_x < rc_s.r; src_x++, p_s++, p_d++ ) *p_d = *p_s;
			p_src_row += src_rgba->_w;
			p_dst_row +=           _w;
		}
		break;

	case pxALPHABLEND_zero:
	{
		uint32_t        uv        =     0;
		for( int src_y = rc_s.t; src_y < rc_s.b; src_y++ )
		{
			p_s = p_src_row;
			p_d = p_dst_row;
			for( int src_x = rc_s.l; src_x < rc_s.r; src_x++, p_s++, p_d++ )
			{
				uv = *p_s;
				if( uv & 0xff000000 ) *p_d = uv;
			}
			p_src_row += src_rgba->_w;
			p_dst_row +=           _w;
		}
	}
	break;

	case pxALPHABLEND_to_white:
	{
		int32_t work_s = 0;
		int32_t work_a = 0;
		int32_t r, g, b;
		for( int src_y = rc_s.t; src_y < rc_s.b; src_y++ )
		{
			p_s = p_src_row;
			p_d = p_dst_row;
			for( int src_x = rc_s.l; src_x < rc_s.r; src_x++, p_s++, p_d++ )
			{
				work_a = (*p_s & 0xff000000) >> 24;
				b      = (*p_d & 0x00ff0000) >> 16; work_s = (*p_s & 0x00ff0000) >> 16; b += work_s * work_a / 0xff; if( b > 0xff ) b = 0xff;
				g      = (*p_d & 0x0000ff00) >>  8; work_s = (*p_s & 0x0000ff00) >>  8; g += work_s * work_a / 0xff; if( g > 0xff ) g = 0xff;
				r      = (*p_d & 0x000000ff) >>  0; work_s = (*p_s & 0x000000ff) >>  0; r += work_s * work_a / 0xff; if( r > 0xff ) r = 0xff;
				*p_d   = (*p_d & 0xff000000) + (b<<16) + (g<<8) + (r<<0);
			}
			p_src_row += src_rgba->_w;
			p_dst_row +=           _w;
		}
	}
	break;

	case pxALPHABLEND_middle:
	{
		int32_t s_a;
		int32_t r, g, b;
		int32_t work_s;

		for( int src_y = rc_s.t; src_y < rc_s.b; src_y++ )
		{
			p_s = p_src_row;
			p_d = p_dst_row;
			for( int src_x = rc_s.l; src_x < rc_s.r; src_x++, p_s++, p_d++ )
			{
				s_a    = (*p_s & 0xff000000) >> 24;
				if     ( s_a == 0xff ) *p_d = *p_s;
				else if( s_a == 0x00 ){}
				else
				{
					b      = (*p_d & 0x00ff0000) >> 16; work_s = (*p_s & 0x00ff0000) >> 16; b += (work_s - b) * s_a / 0xff; if( b > 0xff ) b = 0xff;
					g      = (*p_d & 0x0000ff00) >>  8; work_s = (*p_s & 0x0000ff00) >>  8; g += (work_s - g) * s_a / 0xff; if( g > 0xff ) g = 0xff;
					r      = (*p_d & 0x000000ff) >>  0; work_s = (*p_s & 0x000000ff) >>  0; r += (work_s - r) * s_a / 0xff; if( r > 0xff ) r = 0xff;
					*p_d   = (*p_d & 0xff000000) + (b<<16) + (g<<8) + (r<<0);
				}
			}
			p_src_row += src_rgba->_w;
			p_dst_row +=           _w;
		}
	}
	break;
	}

	edit_unique_update();

	b_ret = true;

	return b_ret;
}

const pxPalette* pxSurface::get_palette() const
{
	return _palette;
}
