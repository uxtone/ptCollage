// '16/08/20 pxCsvUtf8.
// '16/08/30 ->pxCSV2.
// '16/11/11 load( path ) -> load( descriptor ).
// '17/01/21 + load_transaction().

#ifndef pxCSV2_H
#define pxCSV2_H

#include <pxStdDef.h>

#include "./pxDescriptor.h"

typedef struct
{
	char**  strs   ;
	int32_t str_num;
}
pxCSVUTF8ROW;

class pxCSV2
{
private:
void operator = (const pxCSV2& src){}
pxCSV2          (const pxCSV2& src){}

bool          _b_UTF8         ;

pxCSVUTF8ROW* _rows           ;
int32_t       _row_num        ;

uint8_t       _code_separater ;
uint8_t       _code_text_begin;
uint8_t       _code_text_term ;
uint8_t       _code_escape    ;

void _release();

bool _get_top_code           ( uint32_t* p_code, const char* str_src, int32_t* p_byte ) const;

bool _check_size_by_comma    (                       const char* p_data, int32_t data_size, int32_t* p_str_size ) const;
bool _check_skipbytes        ( int32_t* p_skipbytes, const char* p_data, int32_t data_size, bool*    pb_term    ) const;
bool _count_str_by_indention ( int32_t* p_count    , const char* p_data, int32_t data_size, int32_t* p_bytes    ) const;
bool _scan_allocate_by_comma ( char**   pp_dst     , const char* p_data, int32_t data_size, int32_t* p_bytes    ) const;

void _zero();

public:

pxCSV2();
pxCSV2( uint8_t separater, uint8_t text_begin, uint8_t text_term );
~pxCSV2();

bool set_codes( uint8_t separater, uint8_t text_begin, uint8_t text_term );

bool prepare_rows  ( int32_t row_num, bool b_UTF8 );
bool prepare_strs  ( int32_t row, const char* name, const char* value );
bool prepare_strs  ( int32_t row, const char* name, int32_t     value );
bool prepare_strs  ( int32_t row, const char* name, float       value );
bool get_value_by_name( int32_t* p_value, const char* name ) const;
bool get_value_by_name( float*   p_value, const char* name ) const;

bool read          ( pxDescriptor* desc, bool b_UTF8 );
bool write         ( pxDescriptor* desc, const char* str_line_feed ) const;

bool get_row_enable( int32_t row, const pxCSVUTF8ROW** pp_row ) const;

bool get_value( const char** pp_str, int32_t row       , int32_t column ) const;
bool get_value( const char** pp_str, int32_t get_column, int32_t find_column, const char* find_text ) const;
bool get_row  ( const pxCSVUTF8ROW** pp_row, int32_t row ) const;
bool get_row_count      ( int32_t* p_count              ) const;
bool get_row_value_count( int32_t* p_count, int32_t row ) const;
bool is_UTF8     () const;

bool find_value  ( int32_t* p_row, int32_t column, const char* find_text ) const;
};

#endif
