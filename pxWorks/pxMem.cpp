
#include "./pxError.h"
#include "./pxMem.h"

bool pxMem_zero_alloc( void** pp, uint32_t byte_size )
{
	if( byte_size <= 0                  ){
		pxerr("** pxMEM-z **"); return false;
	}
	if( !(  *pp = malloc( byte_size ) ) ){
		pxerr("** pxMEM **"  ); return false;
	}
	memset( *pp, 0,       byte_size );
	return true;
}

bool pxMem_free( void** pp )
{
	if( !pp || !*pp ) return false;
	free( *pp ); *pp = NULL;
	return true;
}

bool pxMem_zero( void*p, uint32_t byte_size )
{
	if( !p ) return false;
	char* p_ = (char*)p;
	for( uint32_t i = 0; i < byte_size; i++, p_++ ) *p_ = 0;
	return true;
}
