// '17/02/26 ptuiHeader.

#ifndef ptuiHeader_H
#define ptuiHeader_H

#include "./ptuiCursor.h"
#include "./ptui.h"

#include "../WoiceUnit.h"

class ptuiHeader: public ptui
{
private:

bool       _b_init;
WoiceUnit* _woice ;

void _put ( const ptuiCursor *p_cur ) const override;
void _release();

public:

ptuiHeader();
~ptuiHeader();

bool init( WoiceUnit* woice );
};

#endif
