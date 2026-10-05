// '16/05/12 pxwXAudio2.

#ifndef pxwXAudio2_H
#define pxwXAudio2_H

#include <XAudio2.h>

#include <pxStdDef.h>

#include "./pxwXA2unit.h"
#include "./pxwXA2stream.h"

struct AUDIO_DEVICE_DETAILS {
	WCHAR DeviceID[256];
	WCHAR DisplayName[256];
	XAUDIO2_DEVICE_ROLE Role;
	WAVEFORMATEXTENSIBLE OutputFormat;
};

class pxwXAudio2
{
private:

bool                    _b_init       ;

IXAudio2*               _xa2          ;
IXAudio2MasteringVoice* _voice_master ;

AUDIO_DEVICE_DETAILS    _device_detail;

pxwXA2unit**            _units        ;
int32_t                 _unit_num     ;

pxwXA2stream*           _stream       ;

bool _release();

public:

pxwXAudio2();
~pxwXAudio2();

bool init( int32_t unit_num );

void* get_audio();

bool stream_init      ( int32_t ch_num, int32_t sps, float buf_sec );
bool stream_start     ( pxfunc_sample_pcm sampling_proc, void* sampling_user, int priority, const char* th_name );
bool stream_order_stop();
bool stream_is_working();
bool stream_join      ( float timeout_sec, bool* p_b_success );

bool unit_create_16bps( int32_t no, int32_t ch_num, int32_t sps, const void* p_buf, int32_t smp_num );

bool unit_release( int32_t no );
bool unit_play   ( int32_t no );
bool unit_loop   ( int32_t no );
bool unit_stop   ( int32_t no, bool b_force );
bool unit_volume ( int32_t no, float v );
bool unit_is_used( int32_t no ) const;
};

#endif
