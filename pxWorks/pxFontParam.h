// '17/07/24 pxFontParam.

#ifndef pxFontParam_H
#define pxFontParam_H

#include <pxStdDef.h>

#define pxFONTFLAG_bold     0x0001
#define pxFONTFLAG_antialia 0x0002

class pxFontParam
{
private:
void operator = (const pxFontParam& src){}
pxFontParam     (const pxFontParam& src){}
public:

pxFontParam( const pxFontParam* src );
pxFontParam();
~pxFontParam();

char*    font_name ;
uxDS    table_file;
uint32_t flags     ;

int32_t  height    ;

int32_t  grid_w    ;
int32_t  grid_h    ;
int32_t  width_byte;
int32_t  width_ilI ;
int32_t  width_mwMW;
int32_t  interval_x;
int32_t  interval_y;

uint8_t  color_R   ;
uint8_t  color_G   ;
uint8_t  color_B   ;
uint8_t  color_A   ;

bool copy_from     ( const pxFontParam* src );
bool is_different  ( const pxFontParam* src ) const;

bool set_font_name ( const char*  name );
bool set_table_file( const uxDS& name );
};

#endif
