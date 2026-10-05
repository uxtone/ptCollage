// '16/07/29 pxUTF8.
// '16/10/11 pxFile -> pxDescriptor

#ifndef pxUTF8_H
#define pxUTF8_H

#include <pxStdDef.h>

bool    pxUTF8_get_top_code   ( uint32_t* p_code, const char* str_src, int32_t* p_byte );
bool    pxUTF8_check_code_byte(                uint32_t code,          int32_t* p_byte );
bool    pxUTF8_append_code    ( char*       str_dst, uint32_t code                     );
bool    pxUTF8_compare        ( int32_t* p_res, const char* str1, const char* str2     );
bool    pxUTF8_check_size     ( const char* str_src, int32_t* p_size , bool b_permit_illegal );
bool    pxUTF8_count_codes    ( const char* str_src, int32_t* p_count, int32_t data_size );

bool    pxUTF8_free           ( char** pp );
bool    pxUTF8_copy_allocate  ( char** pp, const char* str_src );

bool    pxUTF8_test_reverse   ( const uxDS& path_src, const uxDS& path_dst );

#endif
