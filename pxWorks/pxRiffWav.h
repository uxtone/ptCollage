// '16/01/19 pxRiffWav.h

#ifndef pxRiffWav_H
#define pxRiffWav_H

#include <pxStdDef.h>

#include "./pxDescriptor.h"

class pxRiffWav
{
private:

bool    _b_header;

int32_t _smp_r  ;
int32_t _ch_num ;
int32_t _sps    ;
int32_t _bps    ;
int32_t _smp_num;
int32_t _byte_per_smp;

public:
pxRiffWav();
bool    read_header( pxDescriptor* desc, int32_t *p_ch_num, int32_t *p_sps, int32_t *p_bps, int32_t *p_smp_num );
int32_t read_sample( pxDescriptor* desc, void *p_dst, int32_t smp_num );
};

bool pxRiffWav_save( const TCHAR *path_dst, int32_t ch_num, int32_t sps, int32_t bps, int32_t smp_num, const void* p_src );


class pxRiffWav_output
{
private:

bool    _b_header;
int32_t _smp_w   ;
int32_t _ofs_data_size;

int32_t _ch_num  ;
int32_t _sps     ;
int32_t _bps     ;
int32_t _smp_num ;

int32_t _byte_per_smp;

bool _release();

public:
pxRiffWav_output();

bool write_header( pxDescriptor* desc, int32_t  ch_num, int32_t sps, int32_t bps, int32_t smp_num );
bool write_sample( pxDescriptor* desc, const void *p_src, int32_t smp_num );
bool terminate   ( pxDescriptor* desc );
};


#endif
