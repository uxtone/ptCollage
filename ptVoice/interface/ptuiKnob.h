// '17/03/03 ptuiKnob.

#ifndef ptuiKnob_H
#define ptuiKnob_H

#include <pxStdDef.h>

#include "./ptuiCursor.h"

typedef struct
{
	bool    b_vertical ;

	float   volume_rate;
	int32_t zero_x     ;
	int32_t zero_y     ;
	int32_t number_x   ;
	int32_t number_y   ;
}
PTUIKNOB;

class ptuiKnob
{
private:

enum
{
	_knob_w = 16,
	_knob_h = 13,
};

PTUIKNOB _prm;

public:
ptuiKnob( const PTUIKNOB* prm );

bool make_volume( const fRECT* rc_prnt, int32_t* pv   , int32_t cur_v           ) const;
bool is_touch   ( const fRECT* rc_prnt, int32_t volume, const ptuiCursor* p_cur ) const;
void put        ( const fRECT* rc_prnt, int32_t volume, int32_t ani_no          ) const;
};


#endif
