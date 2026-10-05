
#include <pxMem.h>

#include "./pxwXA2stream.h"

static void _set_pcm_format_16bps( WAVEFORMATEX* p_fmt, int32_t ch_num, int32_t sps )
{
	p_fmt->cbSize          =               0;
	p_fmt->wFormatTag      = WAVE_FORMAT_PCM;
	p_fmt->nChannels       = (WORD )ch_num;
	p_fmt->nSamplesPerSec  = (DWORD)sps   ;
	p_fmt->wBitsPerSample  = (WORD )pxBITPERSAMPLE16;
	p_fmt->nBlockAlign     = p_fmt->nChannels   * p_fmt->wBitsPerSample / 8;
	p_fmt->nAvgBytesPerSec = p_fmt->nBlockAlign * p_fmt->nSamplesPerSec;
}

void pxwXA2strm_callback::OnBufferEnd( void * buf_cntx )
{
	SetEvent( h_bufend );
}

pxwXA2stream::pxwXA2stream()
{
	_b_init         = false;
	_b_order_stop   = false;

	_xa2_src        = NULL ;
	_th_stream      = NULL ;
	_mtx_order_stop = NULL ;
	_def_byte       =     0;

	_clbk           = NULL ;

	memset( _bufs, NULL, sizeof(_bufs) );
	memset( &_fmt,    0, sizeof(_fmt ) );
}

bool pxwXA2stream::_release()
{
	if( !_b_init ) return false;
	_xa2_src->DestroyVoice(); _xa2_src = NULL;
	SAFE_DELETE( _clbk           );
	SAFE_DELETE( _th_stream      );
	SAFE_DELETE( _mtx_order_stop );
	return true;
}

pxwXA2stream::~pxwXA2stream()
{
	_release();
}

bool pxwXA2stream::init( IXAudio2 *p_xa2, int32_t ch_num, int32_t sps, float buf_sec )
{
	if( _b_init ) return false;

	bool b_ret = false;

	_def_byte = 0x00; // 16bit;

	if( !(_th_stream      = new pxwThread()) || !_th_stream     ->init() ) goto term;
	if( !(_mtx_order_stop = new pxwMutex ()) || !_mtx_order_stop->init() ) goto term;
	if( !(_clbk           = new pxwXA2strm_callback() )                  ) goto term;

	_set_pcm_format_16bps( &_fmt, ch_num, sps );

	if( FAILED( p_xa2->CreateSourceVoice( &_xa2_src, &_fmt, 0, XAUDIO2_DEFAULT_FREQ_RATIO, _clbk, NULL, NULL ) ) ) goto term;

	_buf_sec  = buf_sec;
	_smp_num  = (int32_t)(buf_sec * sps);
	_buf_size = _smp_num * ch_num * pxBITPERSAMPLE16 / 8;

	for( int i = 0; i < _BUFFER_COUNT; i++ )
	{
		if( !(  _bufs[ i ] = (uint8_t*)malloc( _buf_size )) ) goto term;
		memset( _bufs[ i ], _def_byte, _buf_size );
	}

	_b_init  = true;
	b_ret    = true;
term:

	if( !b_ret ) _release();
	return b_ret;
}

bool pxwXA2stream::start( pxfunc_sample_pcm sampling_proc, void* sampling_user, int priority, const char* th_name )
{
	if( !_b_init ) return false;

	_sampling_proc = sampling_proc;
	_sampling_user = sampling_user;

	bool b_ret = false;

	if( !_th_stream->standby( _proc_stream_static, this, th_name ) ) goto term;
	if( !_th_stream->resume ( priority )                           ) goto term;

	b_ret = true;
term:

	return b_ret;
}

bool pxwXA2stream::_proc_stream_static( void* arg ){ return reinterpret_cast<pxwXA2stream*>(arg)->_proc_stream(); }
bool pxwXA2stream::_proc_stream       ()
{
	bool b_ret   = false;
	bool b_break = false;
	int  buf_idx =     0;

	XAUDIO2_VOICE_STATE state = {0};

	if( FAILED( _xa2_src->Start( 0, 0 ) ) ) goto term;

	do
	{
		while( _xa2_src->GetState( &state ), state.BuffersQueued >= _BUFFER_COUNT - 1 )
		{
			WaitForSingleObject( _clbk->h_bufend, INFINITE );
		}

		_mtx_order_stop->lock  ();
		if( _b_order_stop ) b_break = true;
		_mtx_order_stop->unlock();

		{
			int res_size =         0;
			int req_size = _buf_size;
			XAUDIO2_BUFFER xabuf = {0};
			xabuf.AudioBytes = _buf_size;
			xabuf.pAudioData = _bufs[ buf_idx ];

			if( !_sampling_proc || !_sampling_proc( _sampling_user, _bufs[ buf_idx ], &res_size, &req_size ) )
			{
				memset( _bufs[ buf_idx ], _def_byte, _buf_size );
			}
			if( b_break )
				xabuf.Flags = XAUDIO2_END_OF_STREAM;
			_xa2_src->SubmitSourceBuffer( &xabuf );
		}

		buf_idx++; buf_idx %= _BUFFER_COUNT;
	}
	while( !b_break );

	while( _xa2_src->GetState( &state ), state.BuffersQueued > 0 )
	{
		WaitForSingleObject( _clbk->h_bufend, INFINITE );
	}
	b_ret = true;
term:
	return b_ret;
}

bool pxwXA2stream::order_stop()
{
	if( !_b_init ) return false;
	_mtx_order_stop->lock  ();
	_b_order_stop = true;
	_mtx_order_stop->unlock();
	return true;
}

bool pxwXA2stream::is_working()
{
	if( !_b_init ) return false;
	return _th_stream->is_working();
}

bool pxwXA2stream::join( float timeout_sec, bool* p_b_success )
{
	return _th_stream->join( timeout_sec, p_b_success );
}
