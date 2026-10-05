// '16/05/12 pxwXA2unit.

#ifndef pxwXA2unit_H
#define pxwXA2unit_H

#include <XAudio2.h>

#include <pxStdDef.h>

#include <pxPlayPCM.h>

class pxwXA2unit
{
private:

IXAudio2SourceVoice* _xa2_src;
WAVEFORMATEX         _fmt    ;
XAUDIO2_BUFFER       _xa2_buf;

pxPlayPCM*           _pcm;

public:
pxwXA2unit();
~pxwXA2unit();

void release();

bool test_make   ( IXAudio2 *p_xa2, int32_t ch_num, int32_t sps, float sec );
bool create_16bps( IXAudio2 *p_xa2, int32_t ch_num, int32_t sps, const void* p_buf, int32_t smp_num );
bool play        ( bool b_loop  );
bool stop        ( bool b_force );
bool volume      ( float v      );
};

#endif
