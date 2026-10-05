
#ifndef pxSTDAFX_H
#include <stdlib.h>
#include <string.h>
#endif

#include "./pxError.h"
#include "./pxStr.h"
#include "./pxWords.h"
#include "./pxMem.h"
#include "./pxUTF8.h"
#include "./pxShiftJIS.h"
#include "./pxCSV2.h"

bool pxWords::load( pxDescriptor* desc, bool b_UTF8 )
{
	pxWords::_release();

	pxCSV2      csv( ';', '"', '"' );
	int32_t     row_num       =    0;
	int32_t     row_value_num =    0;
	int32_t     word_idx      =    0;
	const char* p_str         = NULL;
	int32_t     str_size      =    0;

	if( !csv.read         ( desc, b_UTF8 )  ) return false;
	if( !csv.get_row_count( &row_num )      ) return false;

	for( int r = 0; r < row_num; r++ )
	{
		if( !csv.get_row_value_count( &row_value_num, r ) ) goto term;
		if( row_value_num > 1 ) _word_num++;
	}

	if( !pxMem_zero_alloc( (void**)&_words, sizeof(char*) * _word_num ) ) goto term;

	for( int r = 0; r < row_num; r++ )
	{
		if( !csv.get_row_value_count( &row_value_num, r ) ) goto term;

		if( row_value_num > 1 )
		{
			if( !csv.get_value( &p_str, r, 1 ) ) goto term;

			if( b_UTF8 ){ if( !pxUTF8_check_size    ( p_str, &str_size, false ) ) goto term; }
			else        { if( !pxShiftJIS_check_size( p_str, &str_size, false ) ) goto term; }

			if( b_UTF8 ){ if( !pxUTF8_copy_allocate    ( &_words[ word_idx ], p_str ) ) goto term; }
			else        { if( !pxShiftJIS_copy_allocate( &_words[ word_idx ], p_str ) ) goto term; }
			word_idx++;
		}
	}
	if( word_idx != _word_num ) goto term;

	_b_loaded = true;
term:
	if( !_b_loaded ) _release();
	return _b_loaded;
}

int32_t     pxWords::get_num (         ) const
{
	return _word_num;
}


void pxWords::_release()
{
	pxMem_free( (void**)&_words );
	_word_num = 0;
	_b_loaded = false;
}

pxWords::pxWords()
{
	_word_num =     0;
	_words    = NULL ;
	_b_loaded = false;
}

bool pxWords::is_valid() const
{
	if( !_words ) return false;
	return true;
}

pxWords::~pxWords()
{
	_release();
}

static const char *_default_word = "-?-";

const char *pxWords::W( int wid ) const
{
	if( !_words|| wid >= _word_num ){ pxerr( "inv-word:%d", wid ); return _default_word; }
	return _words[ wid ];
}
