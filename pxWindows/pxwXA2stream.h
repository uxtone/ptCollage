// '16/06/06  pxwXA2stream.
// '16/06/08 +pxwXA2stream_callback

#ifndef pxwXA2stream_H
#define pxwXA2stream_H


#include <comdef.h>
#include <XAudio2.h>

#include <pxStdDef.h>

#include "./pxwThread.h"
#include "./pxwMutex.h"


class pxwXA2strm_callback : public IXAudio2VoiceCallback
{
public:
HANDLE h_bufend;

pxwXA2strm_callback(): h_bufend( CreateEvent( NULL, FALSE, FALSE, NULL ) ){}
~pxwXA2strm_callback(){ CloseHandle( h_bufend ); }

void STDMETHODCALLTYPE OnBufferEnd( void * buf_cntx );
void STDMETHODCALLTYPE OnStreamEnd               (){}
void STDMETHODCALLTYPE OnVoiceProcessingPassEnd  (){}
void STDMETHODCALLTYPE OnVoiceProcessingPassStart( UINT32 req_smp              ){}
void STDMETHODCALLTYPE OnBufferStart             ( void* buf_cntx              ){}
void STDMETHODCALLTYPE OnLoopEnd                 ( void* buf_cntx              ){}
void STDMETHODCALLTYPE OnVoiceError              ( void* buf_cntx, HRESULT err ){}
};

class pxwXA2stream
{
private:

enum
{
	_BUFFER_COUNT = 3,
};

bool       _b_init        ;
bool       _b_order_stop  ;
pxwThread* _th_stream     ;
pxwMutex*  _mtx_order_stop;

pxwXA2strm_callback* _clbk;


uint8_t*   _bufs[ _BUFFER_COUNT ];

float      _buf_sec ;
int32_t    _smp_num ;
int32_t    _buf_size;
uint8_t    _def_byte;

IXAudio2SourceVoice* _xa2_src ;
WAVEFORMATEX         _fmt     ;

pxfunc_sample_pcm    _sampling_proc;
void*                _sampling_user;

static bool _proc_stream_static( void* arg );
bool        _proc_stream       ();
bool        _release           ();
public:

pxwXA2stream();
~pxwXA2stream();

bool init      ( IXAudio2 *p_xa2, int32_t ch_num, int32_t sps, float buf_sec );
bool start     ( pxfunc_sample_pcm sampling_proc, void* sampling_user, int priority, const char* th_name );
bool order_stop();
bool is_working();
bool join      ( float timeout_sec, bool* p_b_success );

};

#endif
