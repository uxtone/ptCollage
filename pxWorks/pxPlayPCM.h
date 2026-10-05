// '15/12/29 pxOgg16.
// '16/01/19 pxPlayPCM <- pxOgg16.
// '16/08/18 remove 8bps.

#ifndef pxPlayPCM_H
#define pxPlayPCM_H

#include <pxStdDef.h>

#include "./pxRiffWav.h"
#include "./pxOggVorbis.h"
#include "./pxPlaySampler.h"

enum pxPLAYPCMFMTTYPE
{
	pxPLAYPCMFMT_unknown = 0,
	pxPLAYPCMFMT_wav,
	pxPLAYPCMFMT_ogg,
};

class pxPlayPCM: public pxPlaySampler
{
private:
bool _b_init;

pxPLAYPCMFMTTYPE _fmt_type;

FILE*         _fp      ;
pxDescriptor* _desc    ;

pxRiffWav*    _riff_wav ;
bool          _b_use_ogg;
pxOggVorbis*  _ogg_vbs  ;

void*   _p_src       ;
int32_t _smp_num     ;
int32_t _src_size    ;
int32_t _src_w       ;
int32_t _smp_w       ;

int32_t _src_ch_num  ;
int32_t _src_sps     ;
int32_t _src_byte_per_smp;

int32_t _dst_ch_num  ;
int32_t _dst_sps     ;
int32_t _dst_byte_per_smp;

double  _sq_smp_r     ;
int32_t _sq_pos_chk   ;
int32_t _sq_pos_top   ;
int32_t _sq_pos_tmns  ;
int32_t _sq_pos_rpt   ;

int32_t _dbg_read_count;

void _release();

public:
pxPlayPCM();
~pxPlayPCM();

bool init      ();
bool option_ogg();

    // override.
int32_t     get_smp_num() const;
int32_t     get_smp_w  () const;
int32_t     get_smp_r  () const;
bool        set_smp_r  ( int32_t r, bool b_loop );
bool        is_sampling_end() const;

bool        set_destination_quality( int32_t ch_num, int32_t sps );
bool        sampling        (             void *p1, int *p_req_size, int *p_req_packet );
static bool sampling_wrapper( void *user, void *p1, int *p_req_size, int *p_req_packet );

bool pcm_save       ( const uxDS& path, pxPLAYPCMFMTTYPE fmt_type ) const;
bool pcm_load_atonce( const uxDS& path, pxPLAYPCMFMTTYPE fmt_type );
bool pcm_open       ( const uxDS& path, pxPLAYPCMFMTTYPE fmt_type );
void pcm_release    ();
void pcm_close      ();

int32_t  pop_sample     ( void* p_dst, int32_t req_smp );

pxPLAYPCMFMTTYPE type_get() const;

int32_t get_ch_num      () const;
void*   get_sample      ( int32_t *p_ch_num, int32_t *p_sps, int32_t *p_smp_num ) const;
int32_t get_data_bytes  () const;
void*   get_data_pointer() const;

bool sq_position_set( int32_t    chk, int32_t    top, int32_t    rpt, int32_t    tmns )      ;
bool sq_position_get( int32_t *p_chk, int32_t *p_top, int32_t *p_rpt, int32_t *p_tmns ) const;

bool triming_by_sq  ();

bool make_test_sample_16bps( int32_t ch_num, int32_t sps, float sec );
bool set_sample_16bps      ( int32_t ch_num, int32_t sps, const void* p_src, int32_t smp_num );
};

#endif
