#define BUFSIZE_MIDIDEVICENAME 100

#include <pxDescriptor.h>


class ptConfig_MIDI
{
private:


public:

ptConfig_MIDI();
TCHAR   name[ BUFSIZE_MIDIDEVICENAME ];
bool    b_velo    ;
float   key_tuning;

void set_default();
bool write( pxDescriptor* desc ) const;
bool read ( pxDescriptor* desc );
};
