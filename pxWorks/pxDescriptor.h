// '16/10/03 pxDescriptor from pxtnDescriptor.

#ifndef pxDescriptor_H
#define pxDescriptor_H

#include <pxStdDef.h>

#include <stdio.h>

enum pxSEEK
{
	pxSEEK_set = 0,
	pxSEEK_cur,
	pxSEEK_end,
	pxSEEK_num
};

class pxDescriptor
{
private:

void operator = (const pxDescriptor& src){}
pxDescriptor    (const pxDescriptor& src){}

void*   _p_desc;
bool    _b_file;
bool    _b_read;
int32_t _size  ;
int32_t _cur   ;

char*   _heep_str;

bool _init_heep_str();

public:

pxDescriptor();
~pxDescriptor();

void  clear        ();

FILE* get_file_r   ();
bool  set_file_r   ( FILE* fp );
bool  set_file_w   ( FILE* fp );
bool  set_memory_r ( void* p_mem, int len );
bool  seek         ( int mode   , int val );

bool  get_pos      ( int32_t* p_pos ) const;

bool  r_code_check ( const void* src, int32_t size );

bool  w_asfile     ( const void* p, int size, int num );
bool  r            (       void* p, int size, int num );
bool  w_text_asfile( const char* text );
bool  w_arg_asfile ( const char *fmt, ... );

    // UTF-8 text as "int32 byte count, then the bytes" (no terminator). r_utf8 refuses a count above max_size or text
    // that contains a NUL (what a file written before the move to UTF-8, with wide characters, looks like).
bool  w_utf8( const uxDS& text );
bool  r_utf8(       uxDS& text, int32_t max_size );

bool  w_c_with_size_asfile( const char* pc                   );
bool  r_c_with_size       (       char* pc, int32_t buf_size );
bool  r_c_alloc           ( char** ppc );

bool  v_w_asfile   ( int32_t  val, int32_t *p_add );
bool  v_r          ( int32_t* p  );

bool  get_size_bytes( int32_t* p_size ) const;
};

int  pxDescriptor_v_chk ( int val );

#endif
