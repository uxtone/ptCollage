// '17/01/29 pxSurface. from pxTexture(pxBMP).
// '17/06/15 remove _p_buf_scaled, _scale.
// '17/10/09 file -> descriptor

#ifndef pxSurface_H
#define pxSurface_H

#include <pxStdDef.h>

#include "./pxPalette.h"
#include "./pxDescriptor.h"

enum pxALPHABLEND
{
	pxALPHABLEND_ignore = 0,
	pxALPHABLEND_zero      ,
	pxALPHABLEND_to_white  ,
	pxALPHABLEND_middle    ,
};

class pxSurface
{
private:

static uint32_t _sttc_edit_unique;

void operator = (const pxSurface& src){}
pxSurface       (const pxSurface& src){}

uint32_t   _edit_unique;
uint8_t*   _p_buf      ;
int32_t    _w          ;
int32_t    _h          ;
int32_t    _row_size   ;
int32_t    _depth      ;

pxPalette* _palette    ;

TCHAR*      _png_path   ;


public:
pxSurface();
~pxSurface();

bool create          ( int32_t w, int32_t h );
void release_buffer  ();
bool resize_argb     ( float scale );
bool is_bitmap       () const;

bool copy_from       ( const pxSurface* src    );
bool make_scaled_rgba( uint32_t** p_ref_buf, int32_t* w, int32_t* h, float scene_mag ) const;

bool fill_rgba( const sRECT* rc_dst, uint32_t rgba );
bool fill_rgba(                      uint32_t rgba );

void        edit_unique_update();
uint32_t    edit_unique_get   () const;
uint32_t*   get_buf_vari      ();
const void* get_buf_const     () const;
int32_t     get_pitch         () const;

bool blt_rgba(
	const pxSurface* src_rgba,
	const sRECT*     rc_src  ,
	int32_t          dst_x   ,
	int32_t          dst_y   ,
	pxALPHABLEND     alpha_blend );

const pxPalette* get_palette () const;
bool             get_size    ( int32_t* p_w, int32_t* p_h   ) const;
bool             get_png_path( const TCHAR** p_ref_pnt_path ) const;
bool             set_png_path( const TCHAR*    ref_pnt_path );

bool png_read  ( pxDescriptor* desc, int32_t ofs_x, int32_t ofs_y, const pxPalette* p_pal_opt );
bool png_write ( pxDescriptor* desc ) const;
bool jpeg_read ( pxDescriptor* desc, int32_t ofs_x, int32_t ofs_y );
bool jpeg_write( pxDescriptor* desc ) const;
};

#endif
