// '17/02/21 if-OrganKey -> ptuiKeyboard ptuiクラスを継承.
// '17/02/25 Keyboard -> OrganKey. scrollbar inside.

#ifndef ptuiOrganKey_H
#define ptuiOrganKey_H

#include <pxStdDef.h>


#include "../ptVoice.h"
#include "../WoiceUnit.h"

#include "./ptuiCursor.h"

#include "../../Generic/if_gen_Scroll.h"

#include "./ptui.h"

typedef struct
{
	bool     bON ;
	int32_t  code;
	int32_t  id  ;
}
ptuiORGKEY;

class ptuiOrganKey: public ptui
{
private:

bool           _b_init;
WoiceUnit*     _woice ;
char           _push_flag[ ptvORGANKEY_KEYNUM / 8 ];
int32_t        _selected_octave;

if_gen_Scroll* _scrl_v;
ptuiORGKEY   * _keys  ;
int32_t        _key_index_offset;

int32_t        _tone_last_key;
int32_t        _tone_last_id ;


bool _check_on_button  ( const ptuiCursor* p_cur );

void _volume_make      ( int32_t *py, int32_t cur_y ) const;

bool _proc_begin_sub   () override;
bool _cursor_free      (       ptuiCursor* p_cur ) override;
bool _cursor_click_hold(       ptuiCursor* p_cur ) override;
bool _cursor_key_hold  (       ptuiCursor* p_cur ) override;
bool _cursor_keyboard  (       ptuiCursor* p_cur ) override;
bool _cursor_scroll_v  (       ptuiCursor* p_cur ) override;

void _put              ( const ptuiCursor *p_cur ) const override;

void _release();

public:
ptuiOrganKey();
~ptuiOrganKey();

bool init( WoiceUnit* woice );

bool key_proc        ( WoiceUnit* woice );
bool key_reset       ( WoiceUnit* woice );
bool key_octave_shift( bool b_down );
bool key_octave_set  ( int32_t oct );

void key_push  ( int32_t key, bool bOn );
void key_clear ();
void octave_set( int32_t oct );
};

#endif
