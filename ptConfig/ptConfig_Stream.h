#pragma once

#include <pxDescriptor.h>

class ptConfig_Stream
{
private:

int32_t _def_sps    ;
int32_t _def_ch_num ;
float   _def_buf_sec;

public:

int32_t sps    ;
int32_t ch_num ;
float   buf_sec;

ptConfig_Stream( int32_t def_sps, int32_t def_ch_num, float def_buf_sec );

void set_default();
bool write( pxDescriptor* desc ) const;
bool read ( pxDescriptor* desc );
};

