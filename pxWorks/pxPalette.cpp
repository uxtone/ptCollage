
#include "./pxMem.h"

#include "./pxPalette.h"


pxPalette:: pxPalette()
{
	_tbl = NULL;
	_num =    0;
	_max =    0;
}

pxPalette::~pxPalette()
{
	pxMem_free( (void**)&_tbl );
}

bool    pxPalette::allocate         ( int32_t num )
{
	if( _tbl && num <= _max ){ pxMem_zero( _tbl, sizeof( uint32_t ) * _max ); return true; }
	pxMem_free( (void**)&_tbl );
	if( !pxMem_zero_alloc( (void**)&_tbl, sizeof(uint32_t) * num ) ) return false;
	_max = num;
	_num =   0;
	return true;
}

int32_t pxPalette::count            () const
{
	if( !_tbl ) return 0;
	return _num;
}

bool    pxPalette::copy_from        ( const pxPalette* src_plt )
{
	if( !src_plt || !src_plt->get_color( 0, NULL ) ) return false;
	if( this == src_plt ) return true;

	int32_t num = src_plt->count();

	if( _tbl && num <= _max ){ pxMem_zero( _tbl, sizeof( uint32_t ) * _max ); }
	else
	{
		pxMem_free( (void**)&_tbl );
		if( !pxMem_zero_alloc( (void**)&_tbl, sizeof(uint32_t) * num ) ) return false;
		_max = num;
	}
	_num = src_plt->_num;
	for( int i = 0; i < _num; i++ ) _tbl[ i ] = src_plt->_tbl[ i ];
	return true;
}

int32_t pxPalette::find_or_add_color( uint32_t rgba )
{
	if( !_tbl ) return -1;
	int32_t idx = 0;

	for( idx = 0; idx < _num; idx++ )
	{
		if( _tbl[ idx ] == rgba ) return idx;
	}
	if( idx >= _max ) return -1;
	_tbl[ idx ] = rgba;
	_num++;
	return idx;
}


bool pxPalette::get_color( int32_t idx, int32_t* p_r, int32_t* p_g, int32_t* p_b, int32_t* p_a ) const
{
	if( !_tbl || idx < 0 || idx >= _num ) return false;
	uint32_t w = _tbl[ idx ];
	if( p_r ) *p_r = ( (w&0x000000ff) >>  0 );
	if( p_g ) *p_g = ( (w&0x0000ff00) >>  8 );
	if( p_b ) *p_b = ( (w&0x00ff0000) >> 16 );
	if( p_a ) *p_a = ( (w&0xff000000) >> 24 );
	return true;
}

bool pxPalette::get_color( int32_t idx, uint32_t* p_rgba                                       ) const
{
	if( !_tbl || idx < 0 || idx >= _num ) return false;
	if( p_rgba ) *p_rgba = _tbl[ idx ];
	return true;
}

bool    pxPalette::set_color_force( int32_t idx, int32_t    r, int32_t    g, int32_t    b, int32_t    a )
{
	if( !_tbl || idx < 0 || idx >= _max ) return false;
	_tbl[ idx ] = ((0xff)<<24) | (b<<16) | (g<<8) | (r<<0);
	if( _num <= idx ) _num = idx + 1;
	return true;
}

bool    pxPalette::set_trans_force( int32_t idx, uint8_t t )
{
	if( !_tbl || idx < 0 || idx >= _num ) return false;
	_tbl[ idx ] = (t<<24) | (_tbl[ idx ] & 0x00ffffff);
	return true;
}
