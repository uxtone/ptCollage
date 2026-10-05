// '17/01/14 if_WorkTemp.

#ifndef if_WorkTemp_H
#define if_WorkTemp_H

#include <pxStdDef.h>

#include "../WoiceUnit.h"
#include "../ptVoice.h"

#include "./ptuiCursor.h"

#include "./ptui.h"

typedef struct
{
	int32_t    push_ani_no;
	int32_t    temp_ani_no;
	bool       b_temp_on  ;
	WoiceUnit* temp_woice ;
}
WORKTEMPUNIT;

class ptuiWorkTemp: public ptui
{
private:

bool          _b_init  ;
WoiceUnit*    _woice   ;
int32_t       _unit_num;
WORKTEMPUNIT* _units   ;
int32_t       _last_u  ;

bool _cursor_free      ( ptuiCursor* p_cur ) override;
bool _cursor_click_hold( ptuiCursor* p_cur ) override;
void _put        ( const ptuiCursor* p_cur ) const override;

bool _search_button( const ptuiCursor* p_cur, int32_t* p_tgt_id ) const;

void _anime_reset();
void _anime_set  ( int32_t tgt_id, int32_t ani_no );

void _release();

public:

ptuiWorkTemp();
~ptuiWorkTemp();

bool init       ();


void last_unit_clear();
};


#endif
