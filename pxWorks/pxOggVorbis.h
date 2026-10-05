// '16/01/19 pxOggVorbis.h

#ifndef pxOggVorbis_H
#define pxOggVorbis_H

#include <pxStdDef.h>

#include "./pxDescriptor.h"

class pxOggVorbis
{
private:

bool _b_init ;

bool     _b_header   ;

int32_t  _smp_r      ;
int32_t  _ch_num     ;
int32_t  _sps        ;
int32_t  _bps        ;
int32_t  _smp_num    ;

enum
{
	_BUFSIZE_VB_READ = 4096, // take 4k out of the data segment, not the stack
};

uint8_t* _vb_read_buf;
void*    _vb_file    ; // OggVorbis_File
int      _ov_cur     ; // current_section of reading file.

int32_t  _byte_per_smp;
int32_t  _buf_size    ;

void _release();

public:
pxOggVorbis();
~pxOggVorbis();

bool    init       ();
bool    read_header( pxDescriptor* desc, int32_t *p_ch_num, int32_t *p_sps, int32_t *p_bps, int32_t *p_smp_num );
int32_t read_sample( pxDescriptor* desc, void *p_dst, int32_t smp_num );
void    vb_close   ();

static bool sttc_save( const uxDS& path_dst, int32_t ch_num, int32_t sps, int32_t bps, int32_t smp_num, const void* p_src );
};


#endif
