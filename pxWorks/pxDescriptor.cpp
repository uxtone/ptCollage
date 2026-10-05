#include "./pxMem.h"
#include "./pxDescriptor.h"
#include <vector>

#define _BUFSIZE_HEEP 260

pxDescriptor::pxDescriptor()
{
	_p_desc   = NULL ;
	_size     =     0;
	_b_file   = false;
	_b_read   = false;
	_cur      =     0;

	_heep_str = NULL ;
}

pxDescriptor::~pxDescriptor()
{
	clear();
}

void  pxDescriptor::clear()
{
	if( _p_desc && _b_file ){ fclose( (FILE*)_p_desc ); _p_desc = NULL; _b_file = false; }
	pxMem_free( (void**)&_heep_str );

	_p_desc   = NULL ;
	_size     =     0;
	_b_file   = false;
	_b_read   = false;
	_cur      =     0;
}


bool pxDescriptor::_init_heep_str()
{
	if( _heep_str ) return true;
	if( !pxMem_zero_alloc( (void**)&_heep_str, _BUFSIZE_HEEP ) ) return false;
	return true;
}

bool pxDescriptor::get_size_bytes( int32_t* p_size ) const
{
	if( !_p_desc || !p_size ) return false;
	*p_size = _size;
	return true;
}

bool pxDescriptor::set_memory_r( void *p_mem, int size )
{
	if( !p_mem || size < 1 ) return false;
	_p_desc = p_mem;
	_size   = size ;
	_b_file = false;
	_b_read = true ;
	_cur    =     0;
	return true;
}

FILE* pxDescriptor::get_file_r   ()
{
	if( !_p_desc || !_b_file ) return NULL;
	return (FILE*)_p_desc;
}


bool pxDescriptor::set_file_r  ( FILE *fd )
{
	if( !fd ) return false;

	fpos_t sz;
	if( fseek  ( fd, 0, SEEK_END ) ) return false;
	if( fgetpos( fd, &sz         ) ) return false;
	if( fseek  ( fd, 0, SEEK_SET ) ) return false;
	_p_desc = fd  ;

	_size = pxFPOS_OFFSET(sz);

	_b_file = true;
	_b_read = true;
	_cur    =    0;
	return true;
}

bool pxDescriptor::set_file_w  ( FILE *fd )
{
	if( !fd ) return false;

	fpos_t sz;
	if( fgetpos( fd, &sz ) ) return false;

	_size = pxFPOS_OFFSET(sz);

	_p_desc = fd   ;
	_b_file = true ;
	_b_read = false;
	_cur    =    0 ;
	return true;
}

bool  pxDescriptor::get_pos( int32_t* p_pos ) const
{
	if( !_p_desc ) return false;
	if( _b_file )
	{
		fpos_t sz;
		if( fgetpos( (FILE*)_p_desc, &sz ) ) return false;

		*p_pos = pxFPOS_OFFSET(sz);

	}
	else
	{
		*p_pos   = _cur;
	}
	return true;
}


bool pxDescriptor::seek( int mode, int val )
{
	if( _b_file )
	{
		int seek_tbl[ pxSEEK_num ] = {SEEK_SET, SEEK_CUR, SEEK_END};
		if( fseek( (FILE*)_p_desc, val, seek_tbl[ mode ] ) ) return false;

		switch( mode )
		{
		case pxSEEK_set: _cur  =         val; break;
		case pxSEEK_cur: _cur +=         val; break;
		case pxSEEK_end: _cur  = _size + val; break;
		default: return false;
		}
	}
	else
	{
		switch( mode )
		{
		case pxSEEK_set:
			if( val >=         _size ) return false;
			if( val <              0 ) return false;
			_cur = val;
			break;
		case pxSEEK_cur:
			if( _cur  + val >= _size ) return false;
			if( _cur  + val <      0 ) return false;
			_cur += val;
			break;
		case pxSEEK_end:
			if( _size + val >= _size ) return false;
			if( _size + val <      0 ) return false;
			_cur = _size + val;
			break;
		default: return false;
		}
	}
	return true;
}

bool pxDescriptor::w_text_asfile( const char* text )
{
	if( !text ) return false;
	int len = strlen( text );
	return w_asfile( text, 1, len );
}

bool pxDescriptor::w_arg_asfile( const char *fmt, ... )
{
	bool b_ret = false;
	int  len;

	if( !_p_desc || !_b_file || !_init_heep_str() ) goto End;

	va_list ap; va_start( ap, fmt ); vsprintf( _heep_str, fmt, ap ); va_end( ap );

	len = strlen( _heep_str );
	if( fwrite(   _heep_str, 1, len, (FILE*)_p_desc ) != len ) goto End;

	b_ret = true;
End:
	return b_ret;
}


bool pxDescriptor::w_asfile( const void *p, int size, int num )
{
	bool b_ret = false;

	if( !size || !num ) return true; // ignore

	if( !_p_desc || !_b_file || _b_read ) goto End;

	if( fwrite( p, size, num, (FILE*)_p_desc ) != num ) goto End;
	_cur  += size * num;
	if( _size < _cur ) _size = _cur;

	b_ret = true;
End:
	return b_ret;
}

bool  pxDescriptor::w_c_with_size_asfile( const char *pc )
{
	int32_t len = pc ? strlen( pc ) : 0;
	if( !w_asfile( &len, sizeof(len), 1 )  ) return false;
	if( len > 0 && !w_asfile( pc, 1, len ) ) return false;
	return true;
}

bool  pxDescriptor::r_c_with_size( char *pc, int32_t buf_size )
{
	int32_t len = 0;
	memset( pc, 0, buf_size );
	if( !r( &len, sizeof(len), 1 )  ) return false;
	if( len < 0 || len >= buf_size  ) return false;
	if( len > 0 && !r( pc, 1, len ) ) return false;
	return true;
}

bool  pxDescriptor::r_c_alloc( char** ppc )
{
	bool    b_ret = false;
	int32_t len   =     0;
	char*   p     = NULL ;

	if( !r( &len, sizeof(len), 1 )  ) return false;

	if( len )
	{
		if( !pxMem_zero_alloc( (void**)&p, len+1 ) ) goto term;
		if( !r( p, 1,                      len   ) ) goto term;
	}

	*ppc = p;

	b_ret = true;
term:
	if( !b_ret ) pxMem_free( (void**)&p );
	return b_ret;
}


bool pxDescriptor::r_code_check ( const void* src, int32_t size )
{
	bool     b_ret = true;
	uint8_t* p     = NULL;
	if( !pxMem_zero_alloc( (void**)&p,    size ) ) goto term;
	if( !r               (          p, 1, size ) ) goto term;
	if( memcmp           ( src,     p,    size ) ) goto term;
	b_ret = true;
term:
	pxMem_free( (void**)&p );
	return b_ret;
}

bool pxDescriptor::r(       void *p, int size, int num )
{
	if( !_p_desc ) return false;
	if( !_b_read ) return false;

	bool b_ret = false;

	if( _b_file )
	{
		if( fread( p, size, num, (FILE*)_p_desc ) != num ) goto End;
		_cur += size * num;
	}
	else
	{
		for( int  i = 0; i < num; i++ )
		{
			if( _cur + size > _size ) goto End;
			memcpy( &((char*)p)[ i ], (uint8_t*)_p_desc + _cur, size );
			_cur += size;
		}
	}

	b_ret = true;
End:
	return b_ret;
}


int  pxDescriptor_v_chk( int val )
{
	uint32_t  us;

	us = (uint32_t)val;
	if( us <        0x80 ) return 1;    // 1byte( 7bit)
	if( us <      0x4000 ) return 2;    // 2byte(14bit)
	if( us <    0x200000 ) return 3;    // 3byte(21bit)
	if( us <  0x10000000 ) return 4;    // 4byte(28bit)
//	if( value < 0x800000000 ) return 5;	// 5byte(35bit)
	if( us <= 0xffffffff ) return 5;

	return 6;
}

bool pxDescriptor_v_to_int( uint32_t* p_i, const uint8_t* bytes5, int byte_num )
{
	uint8_t b[ 5 ] = {};

	switch( byte_num )
	{
	case 0:
		break;
	case 1:
		b[0] =  (bytes5[0]&0x7F)>>0;
		break;
	case 2:
		b[0] = ((bytes5[0]&0x7F)>>0) | (bytes5[1]<<7);
		b[1] =  (bytes5[1]&0x7F)>>1;
		break;
	case 3:
		b[0] = ((bytes5[0]&0x7F)>>0) | (bytes5[1]<<7);
		b[1] = ((bytes5[1]&0x7F)>>1) | (bytes5[2]<<6);
		b[2] =  (bytes5[2]&0x7F)>>2;
		break;
	case 4:
		b[0] = ((bytes5[0]&0x7F)>>0) | (bytes5[1]<<7);
		b[1] = ((bytes5[1]&0x7F)>>1) | (bytes5[2]<<6);
		b[2] = ((bytes5[2]&0x7F)>>2) | (bytes5[3]<<5);
		b[3] =  (bytes5[3]&0x7F)>>3;
		break;
	case 5:
		b[0] = ((bytes5[0]&0x7F)>>0) | (bytes5[1]<<7);
		b[1] = ((bytes5[1]&0x7F)>>1) | (bytes5[2]<<6);
		b[2] = ((bytes5[2]&0x7F)>>2) | (bytes5[3]<<5);
		b[3] = ((bytes5[3]&0x7F)>>3) | (bytes5[4]<<4);
		b[4] =  (bytes5[4]&0x7F)>>4;
		break;
	case 6:
		return false;
	}

	*p_i = *((int32_t*)b);
	return true;
}

void pxDescriptor_int_to_v( uint8_t* bytes5, int* p_byte_num, uint32_t i )
{
	uint8_t a[ 5 ] = {};

	bytes5[ 0 ] = 0; a[ 0 ] = *( (uint8_t *)(&i) + 0 );
	bytes5[ 1 ] = 0; a[ 1 ] = *( (uint8_t *)(&i) + 1 );
	bytes5[ 2 ] = 0; a[ 2 ] = *( (uint8_t *)(&i) + 2 );
	bytes5[ 3 ] = 0; a[ 3 ] = *( (uint8_t *)(&i) + 3 );
	bytes5[ 4 ] = 0; a[ 4 ] = 0;

	// 1byte(7bit)
	if     ( i < 0x00000080 )
	{
		*p_byte_num = 1;
		bytes5[0]  = a[0];
	}

	// 2byte(14bit)
	else if( i < 0x00004000 )
	{
		*p_byte_num = 2;
		bytes5[0]  =             ((a[0]<<0)&0x7F) | 0x80;
		bytes5[1]  = (a[0]>>7) | ((a[1]<<1)&0x7F);
	}

	// 3byte(21bit)
	else if( i < 0x00200000 )
	{
		*p_byte_num = 3;
		bytes5[0] =             ((a[0]<<0)&0x7F) | 0x80;
		bytes5[1] = (a[0]>>7) | ((a[1]<<1)&0x7F) | 0x80;
		bytes5[2] = (a[1]>>6) | ((a[2]<<2)&0x7F);
	}

	// 4byte(28bit)
	else if( i < 0x10000000 )
	{
		*p_byte_num = 4;
		bytes5[0] =             ((a[0]<<0)&0x7F) | 0x80;
		bytes5[1] = (a[0]>>7) | ((a[1]<<1)&0x7F) | 0x80;
		bytes5[2] = (a[1]>>6) | ((a[2]<<2)&0x7F) | 0x80;
		bytes5[3] = (a[2]>>5) | ((a[3]<<3)&0x7F);
	}

	// 5byte(32bit)
	else
	{
		*p_byte_num = 5;
		bytes5[0] =             ((a[0]<<0)&0x7F) | 0x80;
		bytes5[1] = (a[0]>>7) | ((a[1]<<1)&0x7F) | 0x80;
		bytes5[2] = (a[1]>>6) | ((a[2]<<2)&0x7F) | 0x80;
		bytes5[3] = (a[2]>>5) | ((a[3]<<3)&0x7F) | 0x80;
		bytes5[4] = (a[3]>>4) | ((a[4]<<4)&0x7F);
	}
}

// ..uint32_t
bool pxDescriptor::v_w_asfile( int val, int *p_add )
{
	if( !_p_desc || !_b_file || _b_read ) return 0;

	uint8_t a[ 5 ]   = {};
	int32_t byte_num =  0;

	pxDescriptor_int_to_v( a, &byte_num, val );

	if( fwrite( a, 1, byte_num, (FILE*)_p_desc ) != byte_num ) return false;
	if( p_add ) *p_add += byte_num;
	;            _cur  += byte_num;
	if( _size < _cur ) _size = _cur;

	return true;


	/*
	uint8_t  b[ 5 ] = {};
	uint32_t us     = (uint32_t )val;
	int32_t  bytes  = 0;

	a[ 0 ] = *( (uint8_t *)(&us) + 0 );
	a[ 1 ] = *( (uint8_t *)(&us) + 1 );
	a[ 2 ] = *( (uint8_t *)(&us) + 2 );
	a[ 3 ] = *( (uint8_t *)(&us) + 3 );
	a[ 4 ] = 0;

	// 1byte(7bit)
	if     ( us < 0x00000080 )
	{
		bytes = 1;
		b[0]  = a[0];
	}

	// 2byte(14bit)
	else if( us < 0x00004000 )
	{
		bytes = 2;
		b[0]  =             ((a[0]<<0)&0x7F) | 0x80;
		b[1]  = (a[0]>>7) | ((a[1]<<1)&0x7F);
	}

	// 3byte(21bit)
	else if( us < 0x00200000 )
	{
		bytes = 3;
		b[0] =             ((a[0]<<0)&0x7F) | 0x80;
		b[1] = (a[0]>>7) | ((a[1]<<1)&0x7F) | 0x80;
		b[2] = (a[1]>>6) | ((a[2]<<2)&0x7F);
	}

	// 4byte(28bit)
	else if( us < 0x10000000 )
	{
		bytes = 4;
		b[0] =             ((a[0]<<0)&0x7F) | 0x80;
		b[1] = (a[0]>>7) | ((a[1]<<1)&0x7F) | 0x80;
		b[2] = (a[1]>>6) | ((a[2]<<2)&0x7F) | 0x80;
		b[3] = (a[2]>>5) | ((a[3]<<3)&0x7F);
	}

	// 5byte(32bit)
	else
	{
		bytes = 5;
		b[0] =             ((a[0]<<0)&0x7F) | 0x80;
		b[1] = (a[0]>>7) | ((a[1]<<1)&0x7F) | 0x80;
		b[2] = (a[1]>>6) | ((a[2]<<2)&0x7F) | 0x80;
		b[3] = (a[2]>>5) | ((a[3]<<3)&0x7F) | 0x80;
		b[4] = (a[3]>>4) | ((a[4]<<4)&0x7F);
	}
	if( fwrite( b, 1, bytes, (FILE*)_p_desc )     != bytes ) return false;
	if( p_add ) *p_add += bytes;
	_cur  += bytes;
	if( _size < _cur ) _size = _cur;
	*/
	return true;
}

bool pxDescriptor::v_r  ( int32_t *p  )
{
	if( !_p_desc || !_b_read ) return false;

	uint32_t i      =  0;
	uint8_t  a[ 5 ] = {};
	int  count = 0;
	for( count = 0; count < 5; count++ )
	{
		if( !pxDescriptor::r( &a[ count ], 1, 1 ) ) return false;
		if( !(a[ count ] & 0x80) ) break;
	}

	if( !pxDescriptor_v_to_int( &i, a, count + 1 ) ) return false;
	*p = i;
	return true;
}
/*

// 可変長読み込み（int32_t  までを保証）
bool pxDescriptor::v_r  ( int32_t *p  )
{
	if( !_p_desc ) return false;
	if( !_b_read ) return false;

	int     count = 0;
	uint8_t a[ 5 ] = {0};
	uint8_t b[ 5 ] = {0};
	int32_t bytes  = 0;

	for( count = 0; count < 5; count++ )
	{
		if( !pxDescriptor::r( &a[ count ], 1, 1 ) ) return false;
		if( !(a[ count ] & 0x80) ) break;
	}

	switch( count )
	{
	case 0:
		bytes   = 1;
		b[0]    =  (a[0]&0x7F)>>0;
		break;
	case 1:
		bytes   = 2;
		b[0]    = ((a[0]&0x7F)>>0) | (a[1]<<7);
		b[1]    =  (a[1]&0x7F)>>1;
		break;
	case 2:
		bytes   = 3;
		b[0]    = ((a[0]&0x7F)>>0) | (a[1]<<7);
		b[1]    = ((a[1]&0x7F)>>1) | (a[2]<<6);
		b[2]    =  (a[2]&0x7F)>>2;
		break;
	case 3:
		bytes   = 4;
		b[0]    = ((a[0]&0x7F)>>0) | (a[1]<<7);
		b[1]    = ((a[1]&0x7F)>>1) | (a[2]<<6);
		b[2]    = ((a[2]&0x7F)>>2) | (a[3]<<5);
		b[3]    =  (a[3]&0x7F)>>3;
		break;
	case 4:
		bytes   = 5;
		b[0]    = ((a[0]&0x7F)>>0) | (a[1]<<7);
		b[1]    = ((a[1]&0x7F)>>1) | (a[2]<<6);
		b[2]    = ((a[2]&0x7F)>>2) | (a[3]<<5);
		b[3]    = ((a[3]&0x7F)>>3) | (a[4]<<4);
		b[4]    =  (a[4]&0x7F)>>4;
		break;
	case 5:
		return false;
	}

	*p = *((int32_t*)b);

	_cur += bytes;

	return true;
}
*/

bool pxDescriptor::w_utf8( const uxDS& text )
{
	int32_t size = (int32_t)text.size();
	if( !w_asfile( &size, sizeof(size), 1 ) ) return false;
	return !size || w_asfile( *text, 1, size );
}

bool pxDescriptor::r_utf8( uxDS& text, int32_t max_size )
{
	int32_t size = 0;
	if( !r( &size, sizeof(size), 1 )   ) return false;
	if( size < 0 || size > max_size    ) return false;

	std::vector<char> buf( (size_t)size + 1, 0 );
	if( size && !r( &buf[ 0 ], 1, size ) ) return false;
	if( memchr( &buf[ 0 ], 0, (size_t)size ) ) return false;

	text = uxDS( &buf[ 0 ], (size_t)size );
	return true;
}
