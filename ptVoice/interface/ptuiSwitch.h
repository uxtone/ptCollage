// '17/03/04 ptuiSwitch.

#ifndef ptuiSwitch_H
#define ptuiSwitch_H

#include <pxStdDef.h>

#include "./ptuiCursor.h"

typedef struct
{
	int32_t      img_idx ;
	float        x       ;
	float        y       ;
	const fRECT* p_rc_src;
}
PTUISWITCH;

class ptuiSwitch
{
private:

PTUISWITCH _prm ;

public:
ptuiSwitch   ( const PTUISWITCH* prm );
bool is_touch( const fRECT* rc_prnt, const ptuiCursor* p_cur ) const;
void put     ( const fRECT* rc_prnt, int32_t value ) const;
void set_img_idx( int32_t img_idx );
};

#endif
