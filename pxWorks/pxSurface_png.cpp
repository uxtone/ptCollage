
#include <png.h>

#include "./pxError.h"
#include "./pxMem.h"
#include "./pxSurface.h"


uint32_t pxSurface::_sttc_edit_unique = 0;


static void _read_png( png_structp png_ptr, png_bytep data, png_size_t l )
{
	static_cast<pxDescriptor*>(png_get_io_ptr( png_ptr ))->r( data, 1, l );
//	fread( data, 1, l, (FILE*)png_get_io_ptr( png_ptr ) );
}

static void _write_png( png_structp png_ptr, png_bytep data, png_size_t l )
{
	static_cast<pxDescriptor*>(png_get_io_ptr( png_ptr ))->w_asfile( data, 1, l );
//	fread( data, 1, l, (FILE*)png_get_io_ptr( png_ptr ) );
}

static void _write_png_flash( png_structp png_ptr )
{
	return;
}

bool pxSurface::png_read( pxDescriptor* desc, int32_t ofs_x, int32_t ofs_y, const pxPalette* p_pal_opt )
{
	bool        b_ret    = false;

	uint8_t*    p_work   = NULL ;
	uint8_t**   ptrs     = NULL ;
	png_struct* png_ptr  = NULL ;
	png_info*   info_ptr = NULL ;
	png_info*   end_info = NULL ;

	int32_t     png_w = 0, png_h = 0;
	int32_t     row_size   = 0;
	int         color_type = 0;
	int         depth      = 0;

	if( !( png_ptr  = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL ) ) ) goto term;
	if( !( info_ptr = png_create_info_struct( png_ptr ) ) ) goto term;
	if( !( end_info = png_create_info_struct( png_ptr ) ) ) goto term;
	png_set_read_fn( png_ptr, desc, _read_png );
	png_read_info  ( png_ptr, info_ptr );

	{
		int interlace, comp, filter;
		png_uint_32 w = 0, h =   0;
		png_get_IHDR (png_ptr, info_ptr, &w, &h, &depth, &color_type, &interlace, &comp, &filter );

		png_w = w;
		png_h = h;
		if( depth != 8 ) goto term;
	}

	if( _p_buf )
	{
		if( png_w + ofs_x > _w || png_h + ofs_y > _h )
		{
			pxerr( "png large" ); goto term;
		}
	}
	else
	{
		int32_t w = png_w + ofs_x;
		int32_t h = png_h + ofs_y;
		_row_size = w * sizeof(uint32_t);
		if( !pxMem_zero_alloc( (void**)&_p_buf, _row_size * h ) ) goto term;
		_w        = w;
		_h        = h;
		_depth    = 8;
	}

	if( !pxMem_zero_alloc( (void**)&ptrs  , sizeof(uint8_t*) * png_h ) ) goto term;

	switch( color_type )
	{
	case PNG_COLOR_TYPE_PALETTE   :

		row_size = (png_w * depth + 31) / 32 * 4;

		// read palette --------------------
		SAFE_DELETE( _palette );
		_palette = new pxPalette();
		{
			png_color* p_pal;
			int        num = 0;
			int        res = png_get_PLTE( png_ptr, info_ptr, &p_pal, &num );
			if( num )
			{
				if( !_palette->allocate( num ) ) goto term;
				for( int i = 0; i < num; i++, p_pal++ ) _palette->set_color_force( i, p_pal->red, p_pal->green, p_pal->blue, 0xff );
			}
		}

		// trans.
		{
			png_bytep trans     = NULL;
			int       num_trans =    0;
			if (png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, NULL) == PNG_INFO_tRNS && trans != NULL && num_trans > 0 )
			{
				for( int i = 0; i < num_trans; i++ ) _palette->set_trans_force( i, trans[ i ] );
			}
		}

		// allocate work.
		if( !pxMem_zero_alloc( (void**)&p_work, row_size * png_h ) ) goto term;

		// pointer list.
		for( int y = 0; y < png_h; y++ ) ptrs[ y ] = p_work + (y * row_size);

		png_read_image( png_ptr, ptrs     );
		png_read_end  ( png_ptr, end_info );

		{
			uint32_t*        p_dst = NULL;
			uint32_t*        pd    = NULL;
			const uint8_t*   p_src = NULL;
			const uint8_t*   ps    = NULL;
			int32_t          r, g, b, a;
			const pxPalette* pal = p_pal_opt ? p_pal_opt : _palette;

			p_dst = (uint32_t*)(&_p_buf[ _row_size * ofs_y + ofs_x * sizeof(uint32_t) ]);
			p_src = p_work;

			for( int32_t y = 0; y < png_h ; y++ )
			{
				ps = p_src;
				pd = p_dst;
				for ( int32_t x = 0; x < png_w; x++, ps++, pd++ )
				{
					if( !pal->get_color( *ps, &r, &g, &b, &a ) ) goto term;
					r   = r * a / 0xff;
					g   = g * a / 0xff;
					b   = b * a / 0xff;
					*pd = ( (a<<24)|(b<<16)|(g<<8)|(r<<0) );
				}
				p_src += png_w;
				p_dst +=    _w;
			}
		}
		break;


	case PNG_COLOR_TYPE_RGB      :

		row_size =  png_w * 3;

		// allocate work.
		if( !pxMem_zero_alloc( (void**)&p_work, row_size * png_h ) ) goto term;

		// pointer list.
		for( int y = 0; y < png_h; y++ ) ptrs[ y ] = p_work + (y * row_size);

		png_read_image( png_ptr, ptrs     );
		png_read_end  ( png_ptr, end_info );

		{
			uint8_t*       p_dst = &_p_buf[ _row_size * ofs_y + ofs_x * sizeof(uint32_t) ];
			const uint8_t* p_src = p_work;
			uint8_t*       pd    = NULL;
			const uint8_t* ps    = NULL;

			uint8_t a     =            0xff;
			float   alpha = (float)a / 0xff;

			for( int32_t y = 0; y < png_h ; y++ )
			{
				ps = p_src;
				pd = p_dst;
				for ( int32_t x = 0; x < png_w; x++ )
				{
					*pd = (uint8_t)( (float)(*ps) * alpha ); pd++; ps++;
					*pd = (uint8_t)( (float)(*ps) * alpha ); pd++; ps++;
					*pd = (uint8_t)( (float)(*ps) * alpha ); pd++; ps++;
					*pd = a; pd++;
				}
				p_src +=  row_size;
				p_dst += _row_size;
			}
		}

		break;


	case PNG_COLOR_TYPE_RGB_ALPHA:
		row_size =  png_w * 4;

		// allocate work.
		if( !pxMem_zero_alloc( (void**)&p_work, row_size * png_h ) ) goto term;

		// pointer list.
		for( int y = 0; y < png_h; y++ ) ptrs[ y ] = p_work + (y * row_size);

		png_read_image( png_ptr, ptrs     );
		png_read_end  ( png_ptr, end_info );

		{
			uint8_t*       p_dst = &_p_buf[ _row_size * ofs_y + ofs_x * sizeof(uint32_t) ];
			const uint8_t* p_src = p_work;
			uint8_t*       pd    = NULL;
			const uint8_t* ps    = NULL;

			for( int32_t y = 0; y < png_h ; y++ )
			{
				ps = p_src;
				pd = p_dst;
				for ( int32_t x = 0; x < png_w; x++ )
				{
					uint8_t a     =       *(ps + 3);
					float   alpha = (float)a / 0xff;

					*pd = (uint8_t)( (float)(*ps) * alpha ); pd++; ps++;
					*pd = (uint8_t)( (float)(*ps) * alpha ); pd++; ps++;
					*pd = (uint8_t)( (float)(*ps) * alpha ); pd++; ps++;
					*pd =                    *ps           ; pd++; ps++;
				}
				p_src +=  row_size;
				p_dst += _row_size;
			}
		}

		break;

	case PNG_COLOR_TYPE_GRAY      : goto term;
	case PNG_COLOR_TYPE_GRAY_ALPHA: goto term;
	default: goto term;
	}

	edit_unique_update();

	b_ret = true;
term:

	if( png_ptr || info_ptr || end_info )
	{
		png_destroy_read_struct( &png_ptr, &info_ptr, &end_info );
		png_ptr  = NULL;
		info_ptr = NULL;
		end_info = NULL;
	}

	pxMem_free( (void**)&p_work );
	pxMem_free( (void**)&ptrs   );

	return b_ret;
}

bool pxSurface::png_write     ( pxDescriptor* desc ) const
{
	bool        b_ret    = false;
//	FILE*       fp       = NULL ;
	png_structp png_ptr  = NULL ;
	png_infop   info_ptr = NULL ;

	png_color*  pals     = NULL ;
	uint8_t**   row_ptrs = NULL ;
	png_byte*   trns     = NULL ;

	int         color_type = PNG_COLOR_TYPE_RGB_ALPHA;

//	if( !( fp = _tfopen( path_dst, _T("wb") ) ) ) goto term;

	png_ptr  = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info_ptr = png_create_info_struct( png_ptr );
//	png_init_io ( png_ptr, fp );
	png_set_write_fn( png_ptr, desc, _write_png, _write_png_flash );
	png_set_IHDR( png_ptr, info_ptr, _w, _h, _depth,
				  color_type,
				  PNG_INTERLACE_NONE,
				  PNG_COMPRESSION_TYPE_DEFAULT,
				  PNG_FILTER_TYPE_DEFAULT );
/*
	if( color_type == PNG_COLOR_TYPE_PALETTE )
	{
		int32_t pal_num = _palette->count();
		int32_t trn_num = 0;
		int32_t r,g,b,a;

		if( !pxMem_zero_alloc( (void**)&pals, sizeof(png_color) * pal_num ) ) goto term;
		if( !pxMem_zero_alloc( (void**)&trns, sizeof(png_byte ) *     256 ) ) goto term;
		for( int i = 0; i < pal_num; i++ )
		{
			if( !_palette->get_color( i, &r, &g, &b, &a ) ) goto term;
			pals[ i ].red   = r;
			pals[ i ].green = g;
			pals[ i ].blue  = b;
			trns[ i ]       = a;
			if( a != 0xff ) trn_num = i + 1;
		}
		;             png_set_PLTE( png_ptr, info_ptr, pals, pal_num       );
		if( trn_num ) png_set_tRNS( png_ptr, info_ptr, trns, trn_num, NULL );
	}
*/
	if( !pxMem_zero_alloc( (void**)&row_ptrs, sizeof(uint8_t*) * _h ) ) goto term;
	for( int y = 0; y < _h; y++ ) row_ptrs[ y ] = _p_buf + ( _row_size * y );

	png_write_info ( png_ptr, info_ptr );
	png_write_image( png_ptr, row_ptrs );
	png_write_end  ( png_ptr, info_ptr );

	b_ret = true;
term:

	png_destroy_write_struct( &png_ptr, &info_ptr );
//	if( fp ) fclose( fp );

	pxMem_free( (void**)&row_ptrs );
	pxMem_free( (void**)&trns     );
	pxMem_free( (void**)&pals     );

	return b_ret;
}