
#include <stdio.h>
#include <tchar.h>

#include <pxMem.h>

#include "./pxtonewinXA2.h"


pxtonewinXA2::pxtonewinXA2()
{
	_b_init          = false;
	_audio           = NULL ;

	_dst_ch_num      = _safe_ch_num  =     2;
	_dst_sps         = _safe_sps     = 44100;
	_dst_buf_sec     = _safe_buf_sec =  0.1f;

	_safe_smp_count  =     0;

	_thrd_priority   =     1;

	_vc_mtx          = NULL ;
	_vc_master_vol   =  1.0f;
	_vc_tones        = NULL ;
	_vc_play_id_idx  =     1;
	_vc_top          =     0;
	_vc_smooth       =     0;
	_vc_max_tone     =     0;
	_vc_sample_skip  =     1;
	_vc_time_pan_idx =     0;
	_vc_freq         = NULL ;

	_tn_mtx          = NULL;
	_tn_b_sampling   = false;
	pxMem_zero( &_tn_odr, sizeof(_tn_odr) );
}

bool pxtonewinXA2::_release()
{
	if( !_b_init ) return false;
	_b_init = false;

	_pxtn   = NULL ;

	_Voice_Release();

	SAFE_DELETE( _audio );

	return true;
}

pxtonewinXA2::~pxtonewinXA2()
{
	_release();
}

bool pxtonewinXA2::_sampling_proc_static( void *user, void *buf, int *p_res_size, int *p_req_size )
{
	return static_cast<pxtonewinXA2*>(user)->_sampling_proc( buf, p_res_size, p_req_size );
}

bool pxtonewinXA2::_sampling_proc       (             void *buf, int *p_res_size, int *p_req_size )
{
	memset( buf, 0, *p_req_size );
	_tune_sampling ( buf, *p_req_size );
	_voice_sampling( buf, *p_req_size );
	*p_res_size = *p_req_size;
	return true;
}

bool pxtonewinXA2::init( pxtnService* pxtn, int32_t max_voice, int32_t max_vc_odr )
{
	bool b_ret = false;

	if( !pxtn ) goto term;

	if( !( _audio  = new pxwXAudio2() ) || !_audio ->init( 1 ) ) goto term;

	if( !_Voice_Init( max_voice, max_vc_odr ) ) goto term;

	_pxtn   = pxtn;

	_b_init = true;
	b_ret   = true;
term:
	if( !b_ret ) _release();
	return b_ret;
}

bool pxtonewinXA2::stream_start( int32_t ch_num, int32_t sps, float buf_sec )
{
	if( !_b_init ) return false;
	if( _audio->stream_is_working() ) return false;

	bool b_ret = false;
	_safe_ch_num    = _dst_ch_num  = ch_num ;
	_safe_sps       = _dst_sps     = sps    ;
	_safe_buf_sec   = _dst_buf_sec = buf_sec;
	_safe_smp_count = 0;

	if( _pxtn )
	{
		if( !_pxtn->set_destination_quality( ch_num, sps ) ) goto term;
		if(  _pxtn->tones_ready() != pxtnOK ) goto term;
	}

	if( !_Voice_SetConfig() ) goto term;
	if( !_audio->stream_init ( _safe_ch_num, _safe_sps, _safe_buf_sec       ) ) goto term;
	if( !_audio->stream_start( _sampling_proc_static, this, _thrd_priority, "pxtn-xa2" ) ) goto term;

	b_ret = true;
term:

	return b_ret;
}

bool pxtonewinXA2::stream_get_quality_safe( int32_t* p_ch_num, int32_t* p_sps, float* p_buf_sec )
{
	if( !_b_init ) return false;

	if( p_ch_num  ) *p_ch_num  = _safe_ch_num ;
	if( p_sps     ) *p_sps     = _safe_sps    ;
	if( p_buf_sec ) *p_buf_sec = _safe_buf_sec;
	return true;
}

bool pxtonewinXA2::stream_order_stop()
{
	if( !_b_init ) return false;
	return _audio->stream_order_stop();
}

bool pxtonewinXA2::stream_is_working()
{
	if( !_b_init ) return false;
	return _audio->stream_is_working();
}

bool pxtonewinXA2::stream_join( float timeout_sec, bool* p_b_success )
{
	if( !_b_init ) return false;
	return _audio->stream_join( timeout_sec, p_b_success );
}

bool pxtonewinXA2::stream_finalize  ( float interval, int retry, float timeout_sec )
{
	if( !_b_init ) return true;
	if( !_audio->stream_order_stop() ) return false;
	for( int i = 0; i < retry; i++ )
	{
		if( !_audio->stream_is_working() ) break;
		if( i == retry - 1 ) return false;
		Sleep( (DWORD)(interval * 1000) );
	}
	return _audio->stream_join( timeout_sec, NULL );
}
