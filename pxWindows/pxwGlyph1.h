// '16/04/07 pxwGlyph.
// '17/10/12 pxwGlyph1.

#ifndef pxwGlyph1_H
#define pxwGlyph1_H

#include <pxStdDef.h>

#include <pxSurface.h>

class pxwGlyph1
{
private:

bool           _b_init      ;
pxGLYPH_PARAM1 _prm         ;
float          _gen_mag     ;
HFONT          _h_font      ;
uint8_t*       _p_glp_buf   ;
uint32_t       _glp_buf_byte;
pxSurface*     _surf_temp   ;

void _release();

public:

pxwGlyph1();
~pxwGlyph1();

bool init         ( const pxGLYPH_PARAM1 *p_prm, float gen_mag );
bool reset_magnify(                              float gen_mag );
bool generate     ( uint32_t moji, bool b_wide_chaset, const pxSurface** pp_surf, sRECT* p_rc_ofs );
bool color        ( uint32_t argb );
bool another_font ( const TCHAR* font_name, int32_t font_h, bool b_bold );
};

#endif
