
#include "./pxError.h"
#include "./pxMem.h"
#include "./pxUTF8.h"
#include "./pxShiftJIS.h"
#include "./pxStr.h"

#include "./pxCSV2.h"


static bool _check_normal_ascii( uint8_t code, bool b_deny_ISO646 )
{
	if( code <= 0x1f ) return false;

	if( code == 0x20 ) return false; // SPACE

	if( b_deny_ISO646 )
	{
		if( code == 0x23 ) return false; // #
		if( code == 0x24 ) return false; // $
		if( code == 0x40 ) return false; // @
		if( code == 0x5b ) return false; // [
		if( code == 0x5c ) return false; // YEN
		if( code == 0x5d ) return false; // ]
		if( code == 0x5e ) return false; // ^
		if( code == 0x60 ) return false; // '
		if( code == 0x7b ) return false; // {
		if( code == 0x7c ) return false; // |
		if( code == 0x7d ) return false; // }
		if( code == 0x7e ) return false; // ~
	}

	if( code == 0x7f ) return false; // DEL
	return true;
}

void pxCSV2::_zero()
{
	_rows            = NULL ;
	_row_num         =     0;
	_code_separater  = ','  ;
	_code_text_begin = '"'  ;
	_code_text_term  = '"'  ;
	_code_escape     = '\\' ;
	_b_UTF8          = false;
}


pxCSV2::pxCSV2()
{
	_zero();
}

pxCSV2::pxCSV2( uint8_t separater, uint8_t text_begin, uint8_t text_term )
{
	_zero();

	if( separater  && _check_normal_ascii( separater , false ) ) _code_separater  = separater ;
	if( text_begin && _check_normal_ascii( text_begin, false ) ) _code_text_begin = text_begin;
	if( text_term  && _check_normal_ascii( text_term , false ) ) _code_text_term  = text_term ;
}

bool pxCSV2::_get_top_code( uint32_t* p_code, const char* str_src, int32_t* p_byte ) const
{
	if( _b_UTF8 ) return pxUTF8_get_top_code    ( p_code, str_src, p_byte );
	else          return pxShiftJIS_get_top_code( p_code, str_src, p_byte );
}

bool pxCSV2::is_UTF8  () const{ return _b_UTF8; }

bool pxCSV2::set_codes( uint8_t separater, uint8_t text_begin, uint8_t text_term )
{
	if( !_check_normal_ascii( separater , false ) ) return false;
	if( !_check_normal_ascii( text_begin, false ) ) return false;
	if( !_check_normal_ascii( text_term , false ) ) return false;

	if( separater  ) _code_separater  = separater ;
	if( text_begin ) _code_text_begin = text_begin;
	if( text_term  ) _code_text_term  = text_term ;
	return true;
}

void pxCSV2::_release()
{
	if( _rows )
	{
		pxCSVUTF8ROW* pr = _rows;

		for( int i = 0; i < _row_num; i++, pr++ )
		{
			if( pr->strs )
			{
				for( int s = 0; s < pr->str_num; s++ ) pxStr_free( &pr->strs[ s ] );
				pxMem_free( (void**)&pr->strs );
				pr->str_num = 0;
			}
		}
		_row_num = 0;
		pxMem_free( (void**)&_rows );
	}
}

pxCSV2::~pxCSV2()
{
	_release();
}

// comma or new-row.
bool pxCSV2::_check_size_by_comma( const char* p_data, int32_t data_size, int32_t* p_str_size ) const
{
	if( !p_data || !p_str_size ) return false;

	const char* p      = p_data;
	int32_t     byte   =      0;
	int32_t     bytes  =      0;
	uint32_t    code   =      0;
	bool        b_text = false ;
	bool        b_es   = false ;

	for( int s = 0; s < data_size; s += byte, p += byte )
	{
		if( !_get_top_code( &code, p, &byte ) ) return false;

		if( _code_text_begin == _code_text_term )
		{
			if( code == _code_text_begin && !b_es  ){ b_text = b_text ? false : true; }
		}
		else
		{
			if( code == _code_text_begin && !b_es  ){ b_text =                  true; }
			if( code == _code_text_term  && !b_es  ){ b_text =          false       ; }
		}

		if( code == _code_separater && !b_text ) break;
		if( code == 0x0d ) break;
		if( code == 0x0a ) break;
		if( code == 0x00 ) break;

		if( code == _code_escape ){ if( b_es ) b_es = false; else b_es = true; }
		else                                   b_es = false;
		bytes += byte;
	}
	*p_str_size = bytes;
	return true;
}

bool pxCSV2::_check_skipbytes( int32_t* p_skipbytes, const char* p_data, int32_t data_size, bool* pb_term ) const
{
	if( !p_data      ) return false;
	if( !p_skipbytes ) return false;
	if( pb_term      ) *pb_term = false;

	const char* p = p_data;

	*p_skipbytes = 0;

	if( data_size > 0 )
	{
		if     ( *p == _code_separater ) *p_skipbytes = 1;
		else if( *p == 0x0d )
		{
			if( pb_term ) *pb_term = true;
			if( data_size > 1 && *(p+1) == 0x0a ) *p_skipbytes = 2;
			else                                  *p_skipbytes = 1;
		}
		else if( *p == 0x0a )
		{
			if( pb_term ) *pb_term = true;
			*p_skipbytes = 1;
		}
		else if( *p == 0x00 )
		{
			if( pb_term ) *pb_term = true;
		}
		else
		{
			return false;
		}
	}
	return true;
}

bool pxCSV2::_count_str_by_indention( int32_t* p_count, const char* p_data, int32_t data_size, int32_t* p_bytes ) const
{
	if( !p_data || !p_count || !p_bytes ) return false;

	const char* p            = p_data;
	int32_t     bytes        =      0;
	int32_t     total_byte   =      0;
	int32_t     str_count    =      0;
	bool        b_indention  = false ;

	int32_t     d_size = data_size;

	while( d_size > 0 && !b_indention )
	{
		if( !_check_size_by_comma( p, d_size, &bytes ) ) return false;
		d_size     -= bytes;
		p          += bytes;
		total_byte += bytes;

		{
			int32_t skip_byte;
			if( !_check_skipbytes( &skip_byte, p, d_size, &b_indention ) ) return false;
			d_size     -= skip_byte;
			p          += skip_byte;
			total_byte += skip_byte;
		}
		str_count++;
	}
	*p_count = str_count ;
	*p_bytes = total_byte;
	return true;
}

bool pxCSV2::_scan_allocate_by_comma( char** pp_dst, const char* p_data, int32_t data_size, int32_t* p_bytes ) const
{
	bool        b_ret     = false    ;
	const char* p_d       = p_data   ;
	int32_t     d_size    = data_size;
	int32_t     bytes     =         0;
	int32_t     scan_size =         0;
	int32_t     dst_size  =         0;
	char*       p_str     = NULL     ;

	if( !_check_size_by_comma( p_d, d_size, &scan_size ) ) return false;

	{
		const char* p       = p_data   ;
		const char* p_start = NULL     ;
		const char* p_end   = NULL     ;
		int32_t     left    = scan_size;
		int32_t     byte    =         0;
		uint32_t    code    =         0;

		// remove ' ', '\t'
		for( int s = 0; s < scan_size; s += byte )
		{
			if( !p_start )
			{
				if( *p != ' ' && *p != '\t' ) p_start = p;
			}
			if( !_get_top_code( &code, p, &byte ) ) return false;
			if( code == ' ' || code == '\t' ){ if( !p_end ) p_end = p; }
			else p_end = NULL;
			p += byte; left -= byte;
		}

		if     ( !p_start ){ dst_size = 0; p_start = p_data; }
		else if(  p_end   )  dst_size = p_end - p_start;
		else                 dst_size = scan_size - (p_start - p_data);
		if( dst_size < 0 ) return false;

		// remove text-begin/terminate.
		if( dst_size >= 2 && *p_start == _code_text_begin && *(p_start + dst_size - 1) == _code_text_term )
		{
			p_start  += 1;
			dst_size -= 2;
		}

		if( !pxMem_zero_alloc( (void**)&p_str, dst_size + 1 ) ) return false;
		if( dst_size ) memcpy( p_str, p_start, dst_size );
	}

	d_size -= scan_size;
	p_d    += scan_size;
	bytes  += scan_size;

	{
		int32_t skip_byte = 0;
		if( !_check_skipbytes( &skip_byte, p_d, d_size, NULL ) ) goto term;
		bytes += skip_byte;
	}

	if( pp_dst  ) *pp_dst  = p_str;
	if( p_bytes ) *p_bytes = bytes;
	b_ret = true;
term:
	if( !b_ret ) pxMem_free( (void**)&p_str );

	return b_ret;
}

bool pxCSV2::prepare_rows( int32_t row_num, bool b_UTF8 )
{
	bool b_ret = false;

	_release();

	if( !pxMem_zero_alloc( (void**)&_rows, sizeof(pxCSVUTF8ROW) * row_num ) ) goto term;
	_row_num = row_num;
	_b_UTF8  = b_UTF8 ;

	b_ret = true;
term:
	if( !b_ret ) _release();
	return b_ret;
}

static void pxCSVUTF8ROW_free( pxCSVUTF8ROW* p_r )
{
	if( !p_r->strs ) return;
	for( int s = 0; s < p_r->str_num; s++ ) pxStr_free( &p_r->strs[ s ] );
	pxMem_free( (void**)&p_r->strs );
	p_r->str_num = 0;
}

bool pxCSV2::prepare_strs( int32_t row, const char* name, const char* value )
{
	if( !_rows || row < 0 || row >= _row_num ) return false;

	bool          b_ret   = false;
	int32_t       str_num =     2;
	pxCSVUTF8ROW* p_r     = &_rows[ row ]; pxCSVUTF8ROW_free( p_r );
	if( !pxMem_zero_alloc( (void**)&p_r->strs, sizeof(char*) * str_num ) ) goto term;
	p_r->str_num = str_num;
	if( !pxStr_copy_allocate( &p_r->strs[ 0 ], name  ) ) goto term;
	if( !pxStr_copy_allocate( &p_r->strs[ 1 ], value ) ) goto term;

	b_ret = true;
term:
	if( !b_ret ) pxCSVUTF8ROW_free( p_r );

	return b_ret;
}

bool pxCSV2::prepare_strs( int32_t row, const char* name, int32_t value )
{
	char value_text[ 32 ] = {}; sprintf( value_text, "%d", value );
	return prepare_strs( row, name, value_text );
}

bool pxCSV2::prepare_strs( int32_t row, const char* name, float value )
{
	char value_text[ 32 ] = {}; sprintf( value_text, "%0.4f", (double)value );
	return prepare_strs( row, name, value_text );
}

bool pxCSV2::read( pxDescriptor* desc, bool b_UTF8 )
{
	_release();

	bool    b_ret     = false;
	char*   p_data    = NULL ;
	int32_t data_size =     0;
	int32_t row_num   =     0;

	_b_UTF8 = b_UTF8;

	{
		int32_t file_size = 0;
		if( !desc->get_size_bytes( &file_size ) ) goto term; data_size = file_size + 1;
		if( !pxMem_zero_alloc( (void**)&p_data, data_size ) ) goto term;
		if( !desc->r( p_data, 1, file_size ) ) goto term;
	}

	{
		const char* p    = p_data;
		int32_t     byte =      0;
		uint32_t    code =      0;
		bool        b_last_d = false ;

		for( int s = 0; s < data_size; s += byte, p += byte )
		{
			if( !_get_top_code( &code, p, &byte ) ) goto term;
			if( !code ) break;
			if( !s    ) row_num = 1;
			if( code == 0x0a ){ if( !b_last_d )   row_num++; }
			if( code == 0x0d ){ b_last_d = true ; row_num++; }
			else                b_last_d = false;
		}
	}

	if( !pxMem_zero_alloc( (void**)&_rows, sizeof(pxCSVUTF8ROW) * row_num ) ) goto term;
	_row_num = row_num;

	{
		int32_t d_size    = data_size;
		char*   p_d       = p_data   ;
		int32_t row_bytes =         0;
		int32_t str_num   =         0;

		for( int r = 0; r < row_num; r++ )
		{
			int32_t str_num   =      0;
			int32_t bytes     =      0;

			if( !_count_str_by_indention( &str_num, p_d, d_size, &row_bytes ) ) goto term;

			if( !pxMem_zero_alloc( (void**)&_rows[ r ].strs, sizeof(char*) * str_num )   ) goto term;
			_rows[ r ].str_num = str_num;

			for( int s = 0; s < str_num; s++, p_d += bytes, d_size -= bytes )
			{
				if( !_scan_allocate_by_comma( & _rows[ r ].strs[ s ], p_d, d_size, &bytes ) ) goto term;
			}
		}
	}

	b_ret = true;
term:
	pxMem_free( (void**)&p_data );
	if( !b_ret ) _release();
	return b_ret;
}

bool pxCSV2::write( pxDescriptor* desc, const char* str_line_feed ) const
{
	if( !desc ) return false;

	uint8_t str_sep[ 2 ] = { _code_separater, '\0' };

	for( int r = 0; r < _row_num; r++ )
	{
		for( int s = 0; s < _rows[ r ].str_num; s++ )
		{
			if( s && !desc->w_asfile( str_sep, 1, 1 ) ) return false;
			int32_t bytes = 0;
			if( _b_UTF8 ){ if( !pxUTF8_check_size    ( _rows[ r ].strs [ s ], &bytes, false ) ) return false; }
			else         { if( !pxShiftJIS_check_size( _rows[ r ].strs [ s ], &bytes, false ) ) return false; }
			if( bytes > 0 && !desc->w_asfile( _rows[ r ].strs[ s ], 1, bytes ) ) return false;
		}
		if( !desc->w_asfile( str_line_feed, 1, 2 ) ) return false;
	}

	return true;
}

bool pxCSV2::get_row_enable( int32_t row, const pxCSVUTF8ROW** pp_row ) const
{
	if( !_rows ) return false;
	if( row < 0 || row >= _row_num ) return false;

	const pxCSVUTF8ROW* p_row = NULL;

	if( !get_row  ( &p_row, row ) ) return false;
	if( !p_row->str_num           ) return false;
	if(  p_row->str_num == 1 && *p_row->strs[ 0 ] == 0x00 ) return false;

	if( _b_UTF8 )
	{
		int32_t size = 0;
		if( !pxUTF8_check_size( p_row->strs[ 0 ], &size, false ) ) return false;
		if( size > 1 && !memcmp( p_row->strs[ 0 ], "//", 2 ) ) return false;
	}
	else
	{
		if(  strlen(            p_row->strs[ 0 ] ) > 1        && !memcmp( p_row->strs[ 0 ], "//", 2 ) ) return false;
	}

	if( pp_row ) *pp_row = p_row;

	return true;
}


bool pxCSV2::get_value( const char** pp_str, int32_t row, int32_t column ) const
{
	if( !pp_str || !_rows ) return false;
	if( row    < 0 || row    >= _row_num             ) return false;
	if( column < 0 || column >= _rows[ row ].str_num ) return false;
	*pp_str = _rows[ row ].strs[ column ];
	return true;
}

bool pxCSV2::get_value( const char** pp_str, int32_t get_column, int32_t find_column, const char* find_text ) const
{
	if( !pp_str || !find_text ) return false;
	int32_t r = 0;
	if( !find_value( &r, find_column, find_text ) ) return false;
	if( get_column >= _rows[ r ].str_num          ) return false;
	*pp_str = _rows[ r ].strs[ get_column ];
	return true;
}

bool pxCSV2::get_row  ( const pxCSVUTF8ROW** pp_row, int32_t row ) const
{
	if( !pp_row || !_rows ) return false;
	if( row    < 0 || row    >= _row_num             ) return false;
	*pp_row = &_rows[ row ];
	return true;
}

bool pxCSV2::get_row_count( int32_t* p_count ) const
{
	if( !_rows || !p_count ) return false;
	*p_count = _row_num;
	return true;
}

bool pxCSV2::get_row_value_count( int32_t* p_count, int32_t row ) const
{
	if( !_rows || !p_count ) return false;
	if( row    < 0 || row    >= _row_num             ) return false;
	*p_count = _rows[ row ].str_num;
	return true;
}


bool pxCSV2::find_value( int32_t* p_row, int32_t column, const char* find_text ) const
{
	if( !_rows || !p_row || !find_text ) return false;
	if( column < 0  ) return false;

	int32_t res = 0;

	for( int r = 0; r < _row_num; r++ )
	{
		if( column < _rows[ r ].str_num )
		{
			res = 0;
			if( _b_UTF8 ){
				if( pxUTF8_compare( &res, _rows[ r ].strs[ column ], find_text ) && !res ){
					*p_row = r; return true;
				}
			}
			else         {
				if( !strcmp       (       _rows[ r ].strs[ column ], find_text )         ){
					*p_row = r; return true;
				}
			}
		}
	}
	return false;
}

bool pxCSV2::get_value_by_name( int32_t* p_value, const char* name ) const
{
	int32_t row = 0; if( !find_value( &row, 0, name ) ) return false;
	if( _rows[ row ].str_num < 2 ) return false;
	*p_value = atoi( _rows[ row ].strs[ 1 ] );
	return true;
}

bool pxCSV2::get_value_by_name( float* p_value, const char* name ) const
{
	int32_t row = 0; if( !find_value( &row, 0, name ) ) return false;
	if( _rows[ row ].str_num < 2 ) return false;
	*p_value = (float)atof( _rows[ row ].strs[ 1 ] );
	return true;
}
