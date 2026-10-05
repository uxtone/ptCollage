#include "./pxStr.h"
#include "./pxStrT.h"

#include "./pxFontParam.h"


pxFontParam::~pxFontParam()
{
	pxStr_free ( &font_name  );
	pxStrT_free( table_file );
}

pxFontParam::pxFontParam()
{
	font_name  = NULL;
	table_file = NULL;
	flags      =    0;

	height     =   16;

	grid_w     =   16;
	grid_h     =   16;
	width_byte =    8;
	width_ilI  =    8;
	width_mwMW =    8;
	interval_x =    1;
	interval_y =    4;

	color_R    = 0xff;
	color_G    = 0xff;
	color_B    = 0xff;
	color_A    = 0xff;
}

pxFontParam::pxFontParam( const pxFontParam* src )
{
	copy_from( src );
}

bool pxFontParam::set_font_name( const char* name )
{
	if( !name ) return false;
	pxStr_free( &font_name );
	return pxStr_copy_allocate( &font_name, name );
}

bool pxFontParam::set_table_file( const uxDS& name )
{
	if( !name ) return false;
	pxStrT_free( table_file );
	return pxStrT_copy_allocate( table_file, name );
}


bool pxFontParam::copy_from( const pxFontParam* src )
{
	pxStr_free ( &font_name  );
	pxStrT_free( table_file );

	if( src->font_name  && !pxStr_copy_allocate ( &font_name , src->font_name  ) ) return false;
	if( src->table_file && !pxStrT_copy_allocate( table_file, src->table_file ) ) return false;

	height     = src->height    ;
	grid_w     = src->grid_w    ;
	grid_h     = src->grid_h    ;
	width_byte = src->width_byte;
	width_ilI  = src->width_ilI ;
	width_mwMW = src->width_mwMW;
	interval_x = src->interval_x;
	interval_y = src->interval_y;
	flags      = src->flags     ;
	color_R    = src->color_R   ;
	color_G    = src->color_G   ;
	color_B    = src->color_B   ;
	color_A    = src->color_A   ;

	return true;
}

bool pxFontParam::is_different( const pxFontParam* src ) const
{
	if( src == this ) return false;

	if( pxStr_is_different ( font_name , src->font_name  ) ) return true;
	if( pxStrT_is_different( table_file, src->table_file ) ) return true;

	if( height     != src->height     ) return true;
	if( grid_w     != src->grid_w     ) return true;
	if( grid_h     != src->grid_h     ) return true;
	if( width_byte != src->width_byte ) return true;
	if( width_ilI  != src->width_ilI  ) return true;
	if( width_mwMW != src->width_mwMW ) return true;
	if( interval_x != src->interval_x ) return true;
	if( interval_y != src->interval_y ) return true;
	if( flags      != src->flags      ) return true;
	if( color_R    != src->color_R    ) return true;
	if( color_G    != src->color_G    ) return true;
	if( color_B    != src->color_B    ) return true;
	if( color_A    != src->color_A    ) return true;

	return false;
}
