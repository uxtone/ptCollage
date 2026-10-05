#include <pxStdDef.h>
#define BUFSIZE_MIDIDEVICENAME 100

#include <pxDescriptor.h>


class ptConfig_MIDI
{
private:


public:

ptConfig_MIDI();
uxSS<BUFSIZE_MIDIDEVICENAME>    name;
bool    b_velo    ;
float   key_tuning;

void set_default();
bool write( pxDescriptor* desc ) const;
bool read ( pxDescriptor* desc );
};
