// '16/09/12 pxStrT.h
// '16/09/13 + pxStrT_size()
// UTF-8: these are thin helpers over uxDS (an owning string); plain assignment does the same job.

#include <pxStdDef.h>

bool    pxStrT_copy_allocate( uxDS& dst, const uxDS& src                ); // dst = src
bool    pxStrT_copy_allocate( uxDS& dst, const uxDS& src, int32_t extra ); // "extra" spare bytes are not needed: uxDS grows
bool    pxStrT_copy         ( uxDS& dst, const uxDS& src );
void    pxStrT_free         ( uxDS& s );                                  // unsets it
int32_t pxStrT_size         ( const uxDS& str );                           // bytes, without the terminator
bool    pxStrT_compare      ( const uxDS& str1, const uxDS& str2, int32_t num, int32_t* p_res ); // num in bytes; 0 = whole string
bool    pxStrT_is_different ( const uxDS& a, const uxDS& b );
