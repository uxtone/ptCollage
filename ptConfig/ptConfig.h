// '16/07/24 ptConfig <- ptCollage.ptConfig

#include <pxFile2.h>

#include "./ptConfig_Stream.h"
#include "./ptConfig_Font.h"
#include "./ptConfig_MIDI.h"

class ptConfig
{
public:
const pxFile2* _ref_app_file_profile;

ptConfig_Stream *strm;
ptConfig_Font   *font;
ptConfig_MIDI   *midi;

ptConfig( const pxFile2* app_file_profile, int32_t sps, int32_t ch_num, float buf_sec );
~ptConfig();

void set_default();
bool save       () const;
bool load       ();
};
