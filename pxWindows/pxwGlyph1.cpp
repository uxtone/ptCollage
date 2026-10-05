
#include <pxStr.h>
#include <uxStr.h>
#include <pxMem.h>

#include "./pxwGlyph1.h"

pxwGlyph1::pxwGlyph1()
{
	_b_init = false;
	memset( &_prm, 0, sizeof(_prm) );

	_gen_mag      =  1.0f;
	_h_font       = NULL ;
	_p_glp_buf    = NULL ;
	_glp_buf_byte =     0;
	_surf_temp    = NULL ;
}

pxwGlyph1::~pxwGlyph1()
{
	_release();
}

void pxwGlyph1::_release()
{
	if( !_b_init ) return;
	SAFE_DELETE( _surf_temp );
	pxMem_free( (void**)&_p_glp_buf );
	if( _h_font ) DeleteObject( _h_font ); _h_font = NULL;
	_b_init = false;
}

bool pxwGlyph1::another_font ( const uxDS& font_name, int32_t font_h, bool b_bold )
{
	if( !_b_init ) return false;

	if( font_name.size() >= sizeof( _prm.font_name ) / sizeof(char) ) return false;

	HFONT font = CreateFont(
		font_h,  0, 0, 0,        // h, w, t-kakudo, bx-kakudo.
		(b_bold ? FW_BOLD : FW_NORMAL), // weight.
		FALSE,  FALSE, FALSE,    // italic, under-line, strike-out
		SHIFTJIS_CHARSET    ,    // char-set (English -> ANSI)
		OUT_TT_ONLY_PRECIS  ,    //
		CLIP_DEFAULT_PRECIS ,    // clip
		PROOF_QUALITY       ,    //
		FF_DONTCARE         ,
		uxT( font_name ) );
	if( !font ) return false;

	DeleteObject( _h_font ); _h_font = font;
	strcpy( _prm.font_name, *font_name ); // static buffer: libc
	_prm.font_h      = font_h;
	_prm.b_font_bold = b_bold;

	return true;
}

bool pxwGlyph1::init( const pxGLYPH_PARAM1 *p_prm, float gen_mag )
{
	if( _b_init ) return false;

	if( !p_prm->grid_x || !p_prm->grid_y ) return false;

	_h_font = CreateFont(
		p_prm->font_h,  0, 0, 0, // h, w, t-kakudo, bx-kakudo.
		(p_prm->b_font_bold ? FW_BOLD : FW_NORMAL), // weight.
		FALSE,  FALSE, FALSE,    // italic, under-line, strike-out
		SHIFTJIS_CHARSET    ,    // char-set (English -> ANSI)
		OUT_TT_ONLY_PRECIS  ,    //
		CLIP_DEFAULT_PRECIS ,    // clip
		PROOF_QUALITY       ,    //
//FIXED_PITCH | FF_MODERN,
		FF_DONTCARE         ,
		uxT( p_prm->font_name ) );

	if( !_h_font ) goto term;

	switch( p_prm->type )
	{
	case pxGLYPH_gray: _glp_buf_byte =  p_prm->grid_x * sizeof(uint32_t) * p_prm->grid_y; break;
	case pxGLYPH_mono: _glp_buf_byte = (p_prm->grid_x + 31) / 32 * 4     * p_prm->grid_y; break;
	default: goto term;
	}

	if( !pxMem_zero_alloc( (void**)&_p_glp_buf, _glp_buf_byte ) ) goto term;

	_surf_temp = new pxSurface();
	if( !_surf_temp->create( (int32_t)( (float)p_prm->grid_x * gen_mag ), (int32_t)( (float)p_prm->grid_y * gen_mag ) ) ) goto term;

	_prm     = *p_prm ;
	_gen_mag = gen_mag;

	_b_init = true;
term:
	if( !_b_init ) _release();
	return _b_init;
}

bool pxwGlyph1::color        ( uint32_t argb )
{
	if( !_b_init    ) return false;
	if( !_h_font    ) return false;
	if( !_p_glp_buf ) return false;
	if( !_surf_temp ) return false;

	_prm.font_argb = argb;
	return true;
}


bool pxwGlyph1::reset_magnify( float gen_mag )
{
	if( !_b_init    ) return false;
	if( !_prm.grid_x || !_prm.grid_y || !_p_glp_buf ) return true; // ignore

	SAFE_DELETE( _surf_temp );
	_surf_temp = new pxSurface();
	if( !_surf_temp->create( (int32_t)( (float)_prm.grid_x * gen_mag ), (int32_t)( (float)_prm.grid_y * gen_mag ) ) ) return false;
	_gen_mag = gen_mag;
	return true;
}

bool pxwGlyph1::generate( uint32_t code, bool b_wide_chaset, const pxSurface** pp_surf, sRECT* p_rc_ofs )
{
	if( !_b_init    ) return false;
	if( !_h_font    ) return false;
	if( !_p_glp_buf ) return false;
	if( !p_rc_ofs   ) return false;
	if( !pp_surf    ) return false;

	bool         b_ret    = false;
	HDC          hdc      = NULL;
	HFONT        font_old = NULL;
	TEXTMETRIC   tm;
	GLYPHMETRICS gm;
	MAT2         mat = {{0,1}, {0,0}, {0,0}, {0,1}};
	DWORD        glyph_size ;
	UINT         ggo_mode   ;
	int32_t      gm_pitch   ;
	int32_t      dst_row_size = (int32_t)( (float)_prm.grid_x * _gen_mag );

	DWORD        res = 0;

	hdc = CreateCompatibleDC ( NULL );
	if( !hdc )
	{
		DWORD err = GetLastError();
		err = err;

		LPTSTR lpBuffer = NULL;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), LANG_USER_DEFAULT, (LPTSTR)&lpBuffer, 0, NULL );
		LocalFree(lpBuffer);
	}
	font_old = (HFONT)SelectObject( hdc, _h_font );
	if( !GetTextMetrics( hdc, &tm ) ) goto term;

	switch( _prm.type )
	{
	case pxGLYPH_mono: ggo_mode = GGO_BITMAP      ; break;
	case pxGLYPH_gray: ggo_mode = GGO_GRAY8_BITMAP; break;
	default: goto term;
	}

	if( b_wide_chaset ) glyph_size = GetGlyphOutlineW( hdc, code, ggo_mode, &gm,          0, NULL      , &mat );
	else                glyph_size = GetGlyphOutlineA( hdc, code, ggo_mode, &gm,          0, NULL      , &mat );
	if( glyph_size == GDI_ERROR      ) goto term;
	if( glyph_size >  _glp_buf_byte  ) goto term;
	if( b_wide_chaset ) res        = GetGlyphOutlineW( hdc, code, ggo_mode, &gm, glyph_size, _p_glp_buf, &mat );
	else                res        = GetGlyphOutlineA( hdc, code, ggo_mode, &gm, glyph_size, _p_glp_buf, &mat );

	if( res == GDI_ERROR             ) goto term;
	if( gm.gmBlackBoxX > _prm.grid_x ) goto term;
	if( gm.gmBlackBoxY > _prm.grid_y ) goto term;

	_surf_temp->fill_rgba( 0x00000000 );

	uint32_t sample, a, r, g, b;

	static uint32_t* p_dst_row;
	p_dst_row = _surf_temp->get_buf_vari();// >get_buf_pointer_rgba();// _p_tmp_buf;

	switch( _prm.type )
	{
	case pxGLYPH_mono:

		gm_pitch = (gm.gmBlackBoxX + 31) / 32 * 4;
		if( _gen_mag >= 1.0f )
		{
			int32_t mag1 = (int32_t)_gen_mag;

			for ( uint32_t src_y = 0, dst_y = 0; src_y < gm.gmBlackBoxY; src_y++ )
			{
				for( int32_t mag_y = 0; mag_y < mag1; mag_y++, dst_y++ )
				{
					uint32_t* pd = p_dst_row;
					for ( uint32_t src_x = 0, dst_x = 0; src_x < gm.gmBlackBoxX; src_x++ )
					{
						for( int32_t mag_x = 0; mag_x < mag1; mag_x++, pd++ )
						{
							sample = _p_glp_buf[ ( src_x/8 ) + src_y * gm_pitch ];
							if( sample & ( 0x80 >> src_x%8 ) )
							{
								a  = (_prm.font_argb >> 24) &0xff;
								r  = (_prm.font_argb >> 16) &0xff;
								g  = (_prm.font_argb >>  8) &0xff;
								b  = (_prm.font_argb >>  0) &0xff;
								*pd = (a<<24) | (b << 16) | (g << 8) | (r << 0);
							}
							else *pd = 0;
						}
					}
					p_dst_row += dst_row_size;
				}
			}
		}
		else
		{
			int32_t dst_w         = (int32_t)( (float)gm.gmBlackBoxX * _gen_mag );
			int32_t dst_h         = (int32_t)( (float)gm.gmBlackBoxY * _gen_mag );
			int32_t src_pixel_inc = (int32_t)(                  1.0f / _gen_mag );

			int32_t src_y = 0;

			for( int dst_y = 0; dst_y < dst_h; dst_y++ )
			{
				int32_t   src_x =         0;
				uint32_t* pd    = p_dst_row;

				for( int dst_x = 0; dst_x < dst_w; dst_x++, pd++ )
				{
					sample = _p_glp_buf[ ( src_x/8 ) + src_y * gm_pitch ];
					if( sample & ( 0x80 >> src_x%8 ) )
					{
						a  = (_prm.font_argb >> 24) &0xff;
						r  = (_prm.font_argb >> 16) &0xff;
						g  = (_prm.font_argb >>  8) &0xff;
						b  = (_prm.font_argb >>  0) &0xff;
						*pd = (a<<24) | (b << 16) | (g << 8) | (r << 0);
					}
					else *pd = 0;
					src_x += src_pixel_inc;
				}
				p_dst_row += dst_row_size ;
				src_y     += src_pixel_inc;
			}
		}
		break;

	case pxGLYPH_gray:

		gm_pitch = (gm.gmBlackBoxX +  3) & 0xfffc;

		if( _gen_mag >= 1.0f )
		{
			int32_t mag1 = (int32_t)_gen_mag;

			for ( uint32_t src_y = 0, dst_y = 0; src_y < gm.gmBlackBoxY; src_y++ )
			{
				for( int32_t mag_y = 0; mag_y < mag1; mag_y++, dst_y++ )
				{
					for ( uint32_t src_x = 0, dst_x = 0; src_x < gm.gmBlackBoxX; src_x++ )
					{
						for( int32_t mag_x = 0; mag_x < mag1; mag_x++, dst_x++ )
						{
							sample = _p_glp_buf[ src_x + src_y * gm_pitch ] * 255 / 65;
							if( sample )
							{
								a  = ((_prm.font_argb >> 24) &0xff) * sample / 0xff;
								r  = ((_prm.font_argb >> 16) &0xff) * sample / 0xff;
								g  = ((_prm.font_argb >>  8) &0xff) * sample / 0xff;
								b  = ((_prm.font_argb >>  0) &0xff) * sample / 0xff;
								*(p_dst_row + dst_x + dst_y * dst_row_size) = (sample<<24) | (b << 16) | (g << 8) | (r << 0);
							}
							else
							{
								*(p_dst_row + dst_x + dst_y * dst_row_size) = 0;
							}
						}
					}
				}
			}
		}
		else
		{
			int32_t dst_w         = (int32_t)( (float)gm.gmBlackBoxX * _gen_mag );
			int32_t dst_h         = (int32_t)( (float)gm.gmBlackBoxY * _gen_mag );
			int32_t src_pixel_inc = (int32_t)(                  1.0f / _gen_mag );

			int32_t src_y = 0;

			for( int dst_y = 0; dst_y < dst_h; dst_y++ )
			{
				int32_t   src_x =         0;
				uint32_t* pd    = p_dst_row;

				for( int dst_x = 0; dst_x < dst_w; dst_x++, pd++ )
				{
					sample = _p_glp_buf[ src_x + src_y * gm_pitch ] * 255 / 65;
					if( sample )
					{
						a  = ((_prm.font_argb >> 24) &0xff) * sample / 0xff;
						r  = ((_prm.font_argb >> 16) &0xff) * sample / 0xff;
						g  = ((_prm.font_argb >>  8) &0xff) * sample / 0xff;
						b  = ((_prm.font_argb >>  0) &0xff) * sample / 0xff;
						*pd = (sample<<24) | (b << 16) | (g << 8) | (r << 0);
					}
					else *pd = 0;
					src_x += src_pixel_inc;
				}
				p_dst_row += dst_row_size ;
				src_y     += src_pixel_inc;
			}
		}
		break;

	default: goto term;
	}

	p_rc_ofs->l =              (int32_t)( (float)(              gm.gmptGlyphOrigin.x) * _gen_mag );
	p_rc_ofs->t =              (int32_t)( (float)(tm.tmAscent - gm.gmptGlyphOrigin.y) * _gen_mag );
	p_rc_ofs->r = p_rc_ofs->l + (int32_t)( (float)(             gm.gmBlackBoxX      ) * _gen_mag );
	p_rc_ofs->b = p_rc_ofs->t + (int32_t)( (float)(             gm.gmBlackBoxY      ) * _gen_mag );

	*pp_surf    = _surf_temp;

	b_ret = true;
term:

	if( hdc )
	{
		if( font_old ) SelectObject( hdc, font_old );
		DeleteDC( hdc );
	}

	return b_ret;
}
