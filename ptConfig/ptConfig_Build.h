#pragma once

#include "ptConfig_Stream.h"

enum BUILDSCOPEMODE
{
	BUILDSCOPEMODE_TOPLAST = 0,
	BUILDSCOPEMODE_BYTIME,
};


class ptConfig_Build
{
private:

public:

ptConfig_Build( int32_t def_sps, int32_t def_ch_num );
~ptConfig_Build();

ptConfig_Stream     *strm;
bool                b_mute       ;
BUILDSCOPEMODE      scope_mode   ;
float               sec_playtime ;
float               sec_extrafade;
float               volume       ;

void set_default();
bool write( pxDescriptor* desc ) const;
bool read ( pxDescriptor* desc );
};
