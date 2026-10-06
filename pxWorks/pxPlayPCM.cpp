
#define _CRT_SECURE_NO_WARNINGS

#include "./pxError.h"


#include "./pxMem.h"

#include "./pxPlayPCM.h"

pxPlayPCM::pxPlayPCM()
{
	_b_init        = false;
	_fmt_type      = pxPLAYPCMFMT_unknown;

	_fp            = NULL ;
	_desc          = NULL ;
	_riff_wav      = NULL ;
	_ogg_vbs       = NULL ;
	_b_use_ogg     = false;

	_p_src         = NULL ;
	_smp_num       =     0;
	_src_size      =     0;
	_src_w         =     0;
	_smp_w         =     0;

	_src_ch_num    =     0;
	_src_sps       =     0;
	_src_byte_per_smp =  0;

	_dst_ch_num    =     0;
	_dst_sps       =     0;
	_dst_byte_per_smp =  0;

	_sq_flags      =     0;
	_sq_smp_r      =   0.0;
	_sq_pos_chk    =     0;
	_sq_pos_top    =     0;
	_sq_pos_rpt    =     0;
	_sq_pos_tmns   =     0;
}

bool pxPlayPCM::init()
{
	if( _b_init ) return false;

	bool b_ret = false;

	if( !(_desc     = new pxDescriptor() ) ) goto term;
	if( !(_riff_wav = new pxRiffWav   () ) ) goto term;

	_b_init = true;
	b_ret   = true;
term:
	if( !b_ret ) _release();
	return b_ret;
}

bool pxPlayPCM::option_ogg()
{
	if( !_b_init   ) return false;
	if( _b_use_ogg ) return false;

	if( !(_ogg_vbs  = new pxOggVorbis () ) || !_ogg_vbs ->init() ) goto term;
	_b_use_ogg = true ;
term:
	if( !_b_use_ogg ) SAFE_DELETE( _ogg_vbs );

	return _b_use_ogg;
}

void pxPlayPCM::_release()
{
	pcm_release();

	_b_init = false;
	SAFE_DELETE( _riff_wav );
	SAFE_DELETE( _ogg_vbs  );
	SAFE_DELETE( _desc     );
	if( _fp ) fclose( _fp ); _fp = NULL;
}

pxPlayPCM::~pxPlayPCM()
{
	_release();
}

bool pxPlayPCM::set_destination_quality( int32_t ch_num, int32_t sps )
{
	switch( ch_num )
	{
	case 1:
	case 2: break;
	default: return false;
	}

	if( sps < 1 ) return false;

	_dst_ch_num = ch_num;
	_dst_sps    = sps   ;

	_dst_byte_per_smp = pxBITPERSAMPLE16 / 8 * ch_num ;
	return true;
}

bool pxPlayPCM::sampling( void *p1, int *p_req_size, int *p_req_packet )
{
	*p_req_size = 0;
	if( !(*p_req_packet)   ) return false;
	if( !_dst_byte_per_smp ) return false;
	int size1   = *p_req_packet;
	int smp_num = size1 / _dst_byte_per_smp;

	if( size1 % _dst_byte_per_smp ){ pxerr( "req smp inv: %d(%d)", size1, _dst_byte_per_smp ); return false; }

	if( !pop_sample( p1, smp_num ) ) return false;
	*p_req_size = *p_req_packet;
	return true;
}

bool pxPlayPCM::sampling_wrapper( void *user, void *p1, int *p_req_size, int *p_req_packet )
{
	pxPlayPCM *ptr = (pxPlayPCM *)user;
	if( ptr ) return ptr->sampling( p1, p_req_size, p_req_packet );
	return false;
}

bool pxPlayPCM::set_smp_r( int32_t r, bool b_loop )
{
	if( _fmt_type == pxPLAYPCMFMT_unknown ) return false;

	_sq_smp_r  = (double)r;
	_sq_flags &= ~_SQ_FLAG_SAMPLING_END;
	if( b_loop ) _sq_flags |=  _SQ_FLAG_LOOP;
	else         _sq_flags &= ~_SQ_FLAG_LOOP;
	return true;
}


int32_t  pxPlayPCM::get_smp_num() const{ return _smp_num; }
int32_t  pxPlayPCM::get_smp_w  () const{ return _smp_w  ; }
int32_t  pxPlayPCM::get_smp_r  () const{ return (int32_t)_sq_smp_r; }
int32_t  pxPlayPCM::get_ch_num () const{ return _src_ch_num   ; }

void *pxPlayPCM::get_sample( int32_t *p_ch_num, int32_t *p_sps, int32_t *p_smp_num ) const
{
	if( _fmt_type == pxPLAYPCMFMT_unknown ) return NULL;
	if( p_ch_num  ) *p_ch_num  = _src_ch_num;
	if( p_sps     ) *p_sps     = _src_sps   ;
	if( p_smp_num ) *p_smp_num = _smp_num   ;
	return _p_src;
}

int32_t   pxPlayPCM::get_data_bytes  () const
{
	if( _fmt_type == pxPLAYPCMFMT_unknown ) return 0;
	return _src_ch_num * _smp_num * pxBITPERSAMPLE16 / 8;
}

void* pxPlayPCM::get_data_pointer() const
{
	return _p_src;
}



bool pxPlayPCM::pcm_load_atonce( const uxDS& path, pxPLAYPCMFMTTYPE fmt_type )
{
	bool b_ret = false;
	_fmt_type = pxPLAYPCMFMT_unknown;

	if( !pcm_open( path, fmt_type ) ) goto term;

	switch( fmt_type )
	{
	case pxPLAYPCMFMT_wav:
		if( _riff_wav->read_sample( _desc, _p_src, _smp_num ) != _smp_num ) goto term;
		break;
	case pxPLAYPCMFMT_ogg:
		if( !_b_use_ogg ) goto term;
		if( _ogg_vbs ->read_sample( _desc, _p_src, _smp_num ) != _smp_num ) goto term;
		break;
	default: goto term;
	}
	_fmt_type = fmt_type;

	b_ret     = true;
term:
	pcm_close();

	if( !b_ret ){ _fmt_type = pxPLAYPCMFMT_unknown; }
	return b_ret;
}

int32_t pxPlayPCM::pop_sample( void* p_dst, int32_t request_sample )
{
	if( _fmt_type == pxPLAYPCMFMT_unknown ) return 0;
	if( _sq_flags & _SQ_FLAG_SAMPLING_END ) return 0;

	double r_stride = (double)_src_sps / (double)_dst_sps;

	// read file..
	if( _src_w < _src_size )
	{
		int32_t read_size = 0;
		int32_t read_smp  = (int32_t)( (double)request_sample * r_stride) + 1;

		switch( _fmt_type )
		{
		case pxPLAYPCMFMT_wav:
			read_size = _riff_wav->read_sample( _desc, (uint8_t*)_p_src + _src_w, read_smp ) * _src_byte_per_smp;
			break;
		case pxPLAYPCMFMT_ogg:
			if( !_b_use_ogg ) return false;
			read_size = _ogg_vbs ->read_sample( _desc, (uint8_t*)_p_src + _src_w, read_smp ) * _src_byte_per_smp;
			break;
		default : return 0;
		}
		if( read_size <= 0    ) pcm_close();
		else                    _src_w += read_size;
		if( _src_byte_per_smp ) _smp_w  = _src_w / _src_byte_per_smp;
/*
		{
			char str[ 100 ];
			sprintf( str, "read[ %04d ]: %dbyte (%d / %d)\n", _dbg_read_count++, read_size, _src_w, _src_size );
			OutputDebugString( str );
		}
		*/
	}

	int32_t req_smp = request_sample;

	{
		float    wkf_l, wkf_r;
		int32_t  wks_l, wks_r;
		int16_t* pd16 = (int16_t*)p_dst;

		while( req_smp )
		{
			if( _sq_smp_r <  _sq_pos_top  ) _sq_smp_r = _sq_pos_top;
			if( _sq_smp_r >= _sq_pos_tmns )
			{
				if( _sq_flags & _SQ_FLAG_LOOP ){ _sq_smp_r  = _sq_pos_rpt;                  }
				else                           { _sq_flags |= _SQ_FLAG_SAMPLING_END; break; }
			}

			{
				int16_t* ps = ( (int16_t*)_p_src + (int32_t)_sq_smp_r * _src_ch_num );
				{                             wkf_l = (float)(          *ps                 ); }
				if( _src_ch_num == 2 ){ ps++; wkf_r = (float)(          *ps                 ); }
				else                          wkf_r = 0;
			}

			// channel num and float -> int32_t.
			if( _dst_ch_num == 1 )
			{
				if( _src_ch_num == 1 ){ wks_l = (int32_t)(  wkf_l              );   }
				else                  { wks_l = (int32_t)( (wkf_l + wkf_r) / 2 );   }
			}
			else // dst 2ch
			{
				if( _src_ch_num == 1 ){ wks_l = (int32_t)wkf_l; wks_r = (int32_t)wkf_l; }
				else                  { wks_l = (int32_t)wkf_l; wks_r = (int32_t)wkf_r; }
			}

			// max and min.
			{                       if( wks_l < -32768 ) wks_l = 32768; if( wks_l >  32767 ) wks_l = 32767; }
			if( _dst_ch_num == 2 ){ if( wks_r < -32768 ) wks_r = 32768; if( wks_r >  32767 ) wks_r = 32767; }

			{                       *pd16 = (int16_t)   wks_l                ; pd16++; }
			if( _dst_ch_num == 2 ){ *pd16 = (int16_t)   wks_r                ; pd16++; }

			req_smp--; _sq_smp_r += r_stride;
		}
	}

	if( req_smp )
	{
		int16_t *pd = (int16_t*)p_dst ; while( req_smp ){ for( int c = 0; c < _dst_ch_num; c++, pd++ ) *pd =    0; req_smp--; }
		return request_sample;
	}

	return request_sample;
}

void pxPlayPCM::pcm_close()
{
	if( !_b_init ) return;

	if( _b_use_ogg ) _ogg_vbs ->vb_close();

	if( _fp ) fclose( _fp ); _fp = NULL;
	_desc->clear();
}

void pxPlayPCM::pcm_release()
{
	if( !_b_init ) return;

	pcm_close();

	pxMem_free( (void**)&_p_src );

	_fmt_type    = pxPLAYPCMFMT_unknown;
	_src_w       =     0;
	_smp_w       =     0;
	_src_size    =     0;
	_smp_num     =     0;

	_sq_smp_r    =   0.0;
	_sq_pos_chk  =     0;
	_sq_pos_top  =     0;
	_sq_pos_rpt  =     0;
	_sq_pos_tmns =     0;

	_dbg_read_count =  0;
}


bool pxPlayPCM::pcm_open( const uxDS& path, pxPLAYPCMFMTTYPE fmt_type )
{
	if( !_b_init ) return false;

	bool b_ret = false;

	pcm_release();

	if( !(_fp = ux_fopen( path, "rb" ) ) ) goto term;
	if( !_desc->set_file_r( _fp )           ) goto term;

	static int32_t bps;
	bps = 0;

	switch( fmt_type )
	{
	case pxPLAYPCMFMT_wav:
		if( !_riff_wav->read_header( _desc, &_src_ch_num, &_src_sps, &bps, &_smp_num ) ) goto term;
		break;
	case pxPLAYPCMFMT_ogg:
		if( !_b_use_ogg ) goto term;
		if( !_ogg_vbs ->read_header( _desc, &_src_ch_num, &_src_sps, &bps, &_smp_num ) ) goto term;
		break;
	default: goto term;
	}

	if( bps != pxBITPERSAMPLE16 ) goto term;

	_src_byte_per_smp = pxBITPERSAMPLE16 / 8 * _src_ch_num;
	_src_size         = _src_byte_per_smp *_smp_num;
	_sq_pos_tmns      = _smp_num;

	if( !pxMem_zero_alloc( &_p_src, _src_size ) ) goto term;

	_fmt_type  = fmt_type;
	_sq_flags &= ~_SQ_FLAG_SAMPLING_END;
	b_ret      = true;
term:

	if( !b_ret ) pcm_release();

	return b_ret;
}

pxPLAYPCMFMTTYPE pxPlayPCM::type_get() const
{
	if( !_b_init ) return pxPLAYPCMFMT_unknown;

	return _fmt_type;
}

bool pxPlayPCM::is_sampling_end() const
{
	if( !_b_init ) return false;

	if( _sq_flags & _SQ_FLAG_SAMPLING_END ) return true;
	return false;
}

bool pxPlayPCM::sq_position_set( int32_t chk, int32_t top, int32_t rpt, int32_t tmns )
{
	if( !_b_init ) return false;

	if( top <         0 || rpt  <        0 || tmns <        0 ) return false;
	if( top >= _smp_num || rpt  > _smp_num || tmns > _smp_num ) return false;
	if( top >  rpt      || top > tmns      || rpt > tmns      ) return false;

//	_sq_smp_r     = chk  ;

	_sq_pos_chk   = chk  ;
	_sq_pos_top   = top  ;
	_sq_pos_rpt   = rpt  ;
	_sq_pos_tmns  = tmns ;
	return true;
}

bool pxPlayPCM::sq_position_get( int32_t *p_chk, int32_t *p_top, int32_t *p_rpt, int32_t *p_tmns ) const
{
	if( !_b_init ) return false;

	if( p_chk  ) *p_chk  = _sq_pos_chk ;
	if( p_top  ) *p_top  = _sq_pos_top ;
	if( p_rpt  ) *p_rpt  = _sq_pos_rpt ;
	if( p_tmns ) *p_tmns = _sq_pos_tmns;

	return true;
}


bool pxPlayPCM::pcm_save( const uxDS& path, pxPLAYPCMFMTTYPE fmt_type ) const
{
	if( !_b_init ) return false;

	switch( fmt_type )
	{
	case pxPLAYPCMFMT_wav: if( !pxRiffWav_save( path, _src_ch_num, _src_sps, pxBITPERSAMPLE16, _smp_num, _p_src ) ) return false; break;
	case pxPLAYPCMFMT_ogg: return false;
	default              : return false;
	}
	return true;
}

bool pxPlayPCM::triming_by_sq  ()
{
	if( !_b_init ) return false;

	bool b_ret = false;
	void *p    = NULL ;
	if( _fmt_type == pxPLAYPCMFMT_unknown || !_p_src ) goto End;

	static int32_t trimmed_smp_num;
	trimmed_smp_num = _sq_pos_tmns - _sq_pos_top;

	if( !( p = malloc( trimmed_smp_num * _src_byte_per_smp ) ) ) goto End;

	memcpy( p, (uint8_t*)_p_src + _sq_pos_top * _src_byte_per_smp, trimmed_smp_num * _src_byte_per_smp );

	_sq_smp_r    -= _sq_pos_top; if( _sq_smp_r    < 0 ) _sq_smp_r    = 0;
	_sq_pos_chk  -= _sq_pos_top; if( _sq_pos_chk  < 0 ) _sq_pos_chk  = 0;
	_sq_pos_tmns -= _sq_pos_top; if( _sq_pos_tmns < 0 ) _sq_pos_tmns = 0;
	_sq_pos_rpt  -= _sq_pos_top; if( _sq_pos_rpt  < 0 ) _sq_pos_rpt  = 0;
	_sq_pos_top  -= _sq_pos_top; if( _sq_pos_top  < 0 ) _sq_pos_top  = 0;

	free( _p_src ); _p_src = p;
	_smp_num = trimmed_smp_num;
	b_ret = true;
End:
	return b_ret;
}

bool pxPlayPCM::set_sample_16bps( int32_t ch_num, int32_t sps, const void* p_src, int32_t smp_num )
{
	if( !_b_init ) return false;

	bool b_ret = false;

	pcm_release();

	_src_ch_num       = ch_num ;
	_src_sps          = sps    ;
	_smp_num          = smp_num;
	_fmt_type         = pxPLAYPCMFMT_wav;
	_src_byte_per_smp = pxBITPERSAMPLE16 / 8 * _src_ch_num;
	_src_size         = _src_byte_per_smp *_smp_num;
	_sq_pos_tmns      = _smp_num;

	int32_t buf_size = _src_byte_per_smp * _smp_num;

	if( !pxMem_zero_alloc( &_p_src, buf_size ) ) goto term;

	memcpy( _p_src, p_src, buf_size );

	b_ret = true;
term:

	return b_ret;
}


bool pxPlayPCM::make_test_sample_16bps( int32_t ch_num, int32_t sps, float sec )
{
	if( !_b_init ) return false;

	bool b_ret = false;

	float  beep_smp_num = (float)sps /   4;
	float  beep_count   = 0;
	float  wave_smp_num = (float)sps / 220;
	float  wave_count   = 0;

	pcm_release();

	_src_ch_num = ch_num;
	_src_sps    = sps   ;
	_smp_num    = (int32_t)( (float)sps * sec );

	int32_t buf_size = pxBITPERSAMPLE16 / 8 * ch_num * _smp_num;

	if( !( _p_src = malloc( buf_size ) ) ) return false;

	{
		int16_t*p = (int16_t*)_p_src;

		for( int s = 0; s < _smp_num; s++ )
		{
			for( int ch = 0; ch < ch_num; ch++ )
			{
				if( beep_count < beep_smp_num )
				{
					if( wave_count < wave_smp_num/2 ) *p = -(0x1000);
					else                              *p = +(0x1000);
				}
				else *p = 0;

				p++;
				beep_count += 1; if( beep_count > sps          ) beep_count -= sps         ;
				wave_count += 1; if( wave_count > wave_smp_num ) wave_count -= wave_smp_num;
			}
		}
	}

	_fmt_type = pxPLAYPCMFMT_wav;
	_src_byte_per_smp = pxBITPERSAMPLE16 / 8 * _src_ch_num;
	_src_size         = _src_byte_per_smp *_smp_num;
	_sq_pos_tmns      = _smp_num;

	return true;
}
