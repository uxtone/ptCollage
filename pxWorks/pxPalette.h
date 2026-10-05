// '17/01/29 pxPalette.

#ifndef pxPalette_H
#define pxPalette_H

class pxPalette
{
private:
void operator = (const pxPalette& src){}
pxPalette       (const pxPalette& src){}

uint32_t* _tbl;
int32_t   _num;
int32_t   _max;

void _release();

public:
pxPalette();
~pxPalette();

int32_t count            () const;
bool    allocate         ( int32_t num );
bool    copy_from        ( const pxPalette* src_plt );
int32_t find_or_add_color( uint32_t rgba );
bool    get_color      ( int32_t idx, int32_t* p_r, int32_t* p_g, int32_t* p_b, int32_t* p_a ) const;
bool    get_color      ( int32_t idx, uint32_t* p_rgba                                       ) const;
bool    set_color_force( int32_t idx, int32_t    r, int32_t    g, int32_t    b, int32_t    a );
bool    set_trans_force( int32_t idx, uint8_t t );
};

#endif
