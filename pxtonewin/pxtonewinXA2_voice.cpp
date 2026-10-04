
#include <stdio.h>
#include <tchar.h>

#include <pxMem.h>

#include "./pxtonewinXA2.h"


// I/O..
static bool _io_read( void* user, void* p_dst, int32_t size, int32_t num )
{
	if( fread( p_dst, size, num, (FILE*)user ) != num ) return false; return true;
}

static bool _io_write( void* user, const void* p_dst, int32_t size, int32_t num )
{
	if( fwrite( p_dst, size, num, (FILE*)user ) != num ) return false; return true;
}

static  bool _io_seek( void* user,       int   mode , int32_t size              )
{
	if( fseek( (FILE*)user, size, mode ) ) return false; return true;
}

static bool _io_pos( void* user, int32_t* p_pos )
{
	fpos_t sz{};
	if( fgetpos( (FILE*)user, &sz ) ) return false;
	*p_pos  = pxFPOS_OFFSET(sz);
	return true;
}



void _log( const TCHAR *fmt, ... )
{
	TCHAR str[ 100 ];
	va_list ap; va_start( ap, fmt ); _vstprintf_s( str, 100, fmt, ap ); va_end( ap );
	OutputDebugString( str );
}

bool pxtonewinXA2::_Voice_SetConfig()
{
	_vc_smooth      = _dst_sps /  100;
	_vc_sample_skip = 44100.0 / _dst_sps;
	_vc_top         = 0x7fff;
	return true;
}

void pxtonewinXA2::_Voice__Envelope( pxtnVOICEINSTANCE* p_vi, pxtnVOICETONE* p_vt, bool bON )
{
	if( p_vt->life_count <= 0 || !p_vi->env_size ) return;

	if( bON )
	{
		if( p_vt->env_pos < p_vi->env_size )
		{
			p_vt->env_volume = p_vi->p_env[ p_vt->env_pos ];
			p_vt->env_pos++;
		}
	}
	else
	{
		if( p_vt->env_pos < p_vi->env_release )
		{
			p_vt->env_volume = p_vt->env_start + ( 0 - p_vt->env_start ) * p_vt->env_pos / p_vi->env_release;
		}
		else
		{
			p_vt->life_count = 0;
			p_vt->env_volume = 0;
		}
		p_vt->env_pos++;
	}
}

void pxtonewinXA2::_Voice_Envelope()
{
	int32_t  t, v;
	STREAMINGVOICETONE2* p_tone;

	for( t = 0; t < _vc_max_tone; t++ )
	{
		p_tone = &_vc_tones[ t ];

		if( p_tone->b_act )
		{
			for( v = 0; v < p_tone->voice_num; v++ ) _Voice__Envelope( &p_tone->voinsts[ v ], &p_tone->voitones[ v ], p_tone->b_on );
			for( v = 0; v < p_tone->voice_num; v++ ){ if( p_tone->voitones[ v ].life_count >= 0 ) break; }
			if(  v == p_tone->voice_num ) p_tone->b_act = false;
		}
	}
}

//////// サンプリング進行 ////////
void pxtonewinXA2::_Voice_ToNextSample()
{
	pxtnVOICEINSTANCE*   p_vi  ;
	pxtnVOICETONE*       p_vt  ;
	STREAMINGVOICETONE2* p_tone;

	for( int32_t t = 0; t < _vc_max_tone; t++ )
	{
		p_tone = &_vc_tones[ t ];

		if( p_tone->b_act )
		{
			for( int32_t v = 0; v < p_tone->voice_num; v++ )
			{
				p_vi = &p_tone->voinsts [ v ];
				p_vt = &p_tone->voitones[ v ];

				if( p_vt->life_count >= 0 )
				{
					p_vt->smp_pos += p_tone->freq_rate * p_vt->offset_freq * _vc_sample_skip;

					if( p_tone->b_loop )
					{
						if( p_vt->smp_pos >= p_vi->smp_body_w ) p_vt->smp_pos -= p_vi->smp_body_w;
						if( p_vt->smp_pos >= p_vi->smp_body_w ) p_vt->smp_pos  = 0;
						if( !p_vi->smp_tail_w && !p_vi->env_release && !p_tone->b_on ) p_vt->smp_count++;
					}
					else
					{
						if( p_vt->smp_pos >= p_vi->smp_body_w ) p_vt->life_count = 0;
						if( !p_tone->b_on                     ) p_vt->smp_count++;
					}
					if( p_vt->smp_count >= p_tone->min_sample_count ) p_vt->life_count = 0;
				}

			}

			p_tone->b_act = false;
			for( int32_t v = 0; v < p_tone->voice_num; v++ )
			{
				if( p_tone->voitones[ v ].life_count > 0 ){ p_tone->b_act = true; break; }
			}
		}
	}
	_vc_time_pan_idx = ( _vc_time_pan_idx + 1 ) & ( pxtnBUFSIZE_TIMEPAN - 1 );
}

void pxtonewinXA2::_Voice_MakeSample( void* p_buf )
{
	pxtnVOICEINSTANCE*   p_vi  ;
	pxtnVOICETONE*       p_vt  ;
	STREAMINGVOICETONE2* p_tone;

	for( int32_t ch = 0; ch < _dst_ch_num; ch++ )
	{
		for( int32_t t = 0; t < _vc_max_tone; t++ )
		{
			int32_t time_pan_buf = 0;

			p_tone = &_vc_tones[ t ];

			if( p_tone->b_act  )
			{
				for( int32_t v = 0; v < p_tone->voice_num; v++ )
				{
					p_vi = &p_tone->voinsts [ v ];
					p_vt = &p_tone->voitones[ v ];

					int32_t work = 0;

					if( p_vt->life_count >  0 )
					{

						int32_t pos = (int32_t)( p_vt->smp_pos ) * 4 + ch * 2;
						work    += *( (short *)&p_vi->p_smp_w[ pos ] );

						if( _dst_ch_num == 1 )
						{
							work += *( (short *)&p_vi->p_smp_w[ pos + 2 ] );
							work  = work / 2;
						}
						work = ( work * p_tone->velos   [ v  ] ) / 128;
						work =   work * p_tone->pan_vols[ ch ]   /  64;

						// smooth
						if( p_vi->env_release )
						{

							if( p_vt->env_volume > 30 &&
								p_vt->env_volume < 100 )
							{
								p_vt->env_volume = p_vt->env_volume;
							}
							work = work * p_vt->env_volume / 128;
						}
						else if( p_tone->b_loop && p_vt->smp_count > p_tone->min_sample_count -_vc_smooth )
						{
							work  = work * ( p_tone->min_sample_count - p_vt->smp_count ) /_vc_smooth;
						}

					} // voice.bActive;

					time_pan_buf += work;

				}// v
			}
			p_tone->pan_time_bufs[ ch ][ _vc_time_pan_idx ] = time_pan_buf;
		}// t

	}

	// time pan
	for( int32_t ch = 0; ch < _dst_ch_num; ch++ )
	{
		int32_t work = 0;
		int32_t index;

		for( int32_t t = 0; t < _vc_max_tone; t++ )
		{
			p_tone = &_vc_tones[ t ];
			index = ( _vc_time_pan_idx - p_tone->pan_times[ ch ] ) & ( pxtnBUFSIZE_TIMEPAN - 1 );
			work += p_tone->pan_time_bufs[ ch ][ index ];
		}

		work = (int32_t)( (float)work * _vc_master_vol );

		// to buf.
		if( work >  _vc_top ) work =  _vc_top;
		if( work < -_vc_top ) work = -_vc_top;
		*( (short*)p_buf + ch ) = (short)( work );
	}
}


bool pxtonewinXA2::_voice_set_new( const pxtnxaVOICE_ORDER* po )
{
	int32_t  t, v;

	pxtnxaVOICE_ORDER odr = *po;

	STREAMINGVOICETONE2*     p_tone  ;
	pxtnVOICETONE*           p_vt    ;
	pxtnVOICEINSTANCE*       p_vi    ;

	const pxtnVOICEUNIT*     p_tv_src;
	const pxtnVOICEINSTANCE* p_vi_src;

	for( t = 0; t < _vc_max_tone; t++ ){ if( !_vc_tones[ t ].b_act ) break; }
	if( t == _vc_max_tone ) return false;

	if( odr.vol_pan  == -1 ) odr.vol_pan  =  64;
	if( odr.time_pan == -1 ) odr.time_pan =  64;

	for( int i = 0; i < pxtnMAX_UNITCONTROLVOICE; i++ )
	{
		if( odr.velos[ i ] == -1 ) odr.velos[ i ] = 104;
	}

	_vc_play_id_idx++; if( !_vc_play_id_idx ) _vc_play_id_idx = 1;

	p_tone = &_vc_tones[ t ];

	p_tone->play_id          = po->play_id;//_vc_play_id_idx; _vc_play_id_idx++;
	p_tone->b_on             = true     ;
	p_tone->b_act            = true     ;
	p_tone->freq_rate        = odr.freq_rate;
	p_tone->b_loop           = odr.b_loop;
	p_tone->min_sample_count = _dst_sps * 100 / 1000;
	p_tone->voice_num        = odr.p_woice->get_voice_num();

	for( int i = 0; i < pxtnMAX_UNITCONTROLVOICE; i++ )
	{
		p_tone->velos[ i ] = odr.velos[ i ];
	}

	// volume pan
	p_tone->pan_vols[ 0 ] = 64;
	p_tone->pan_vols[ 1 ] = 64;
	if( _dst_ch_num == 2 )
	{
		if( odr.vol_pan >= 64 ) p_tone->pan_vols[ 0 ] = 128 - odr.vol_pan;
		else                    p_tone->pan_vols[ 1 ] =       odr.vol_pan;
	}

	// time pan
	p_tone->pan_times[ 0 ] = 0;
	p_tone->pan_times[ 1 ] = 0;
	if( _dst_ch_num == 2 )
	{
		if( odr.time_pan >= 64 )
		{
			p_tone->pan_times[ 0 ] = ( odr.time_pan - 64 ); if( p_tone->pan_times[ 0 ] >= 64 ) p_tone->pan_times[ 0 ] = 63;
			p_tone->pan_times[ 0 ] = (int32_t)( ((double)p_tone->pan_times[ 0 ] * 44100) / _dst_sps );
		}
		else
		{
			p_tone->pan_times[ 1 ] = ( 64 - odr.time_pan ); if( p_tone->pan_times[ 1 ] >= 64 ) p_tone->pan_times[ 1 ] = 63;
			p_tone->pan_times[ 1 ] = (int32_t)( ((double)p_tone->pan_times[ 1 ] * 44100) / _dst_sps );
		}
	}

	for( v = 0; v < pxtnMAX_UNITCONTROLVOICE; v++ ) p_tone->voitones[ v ].life_count = 0;

	for( v = 0; v < p_tone->voice_num; v++ )
	{
		p_vt             = &p_tone->voitones        [ v ];
		p_vi             = &p_tone->voinsts         [ v ];
		p_vi_src         = odr.p_woice->get_instance( v );
		p_tv_src         = odr.p_woice->get_voice   ( v );

		p_vt->life_count = 1;
		p_vt->smp_pos    = 0;
		p_vt->smp_count  = 0;
		p_vt->env_pos    = 0;


		if( p_tv_src->voice_flags & PTV_VOICEFLAG_BEATFIT )
		{
			p_vt->offset_freq = (float)( ( (double)p_vi_src->smp_body_w * _pxtn->master->get_beat_tempo() ) / ( 44100.0 * 60 * p_tv_src->tuning ) );
		}
		else
		{
			p_vt->offset_freq = _vc_freq->Get( EVENTDEFAULT_BASICKEY - p_tv_src->basic_key ) * p_tv_src->tuning;
		}

		p_vi->p_smp_w     = (unsigned char *)p_vi_src->p_smp_w;
		p_vi->smp_head_w  = p_vi_src->smp_head_w;
		p_vi->smp_body_w  = p_vi_src->smp_body_w;
		p_vi->smp_tail_w  = p_vi_src->smp_tail_w;

		p_vi->env_size    = p_vi_src->env_size   ;
		p_vi->p_env       = p_vi_src->p_env      ;
		p_vi->env_release = p_vi_src->env_release;

		if( p_vi_src->env_size ){ p_vt->env_volume = p_vt->env_start  =   0; }
		else                    { p_vt->env_volume = p_vt->env_start  = 128; }

	}

	return true;
}


void pxtonewinXA2::_voice_sampling( LPVOID p1, int32_t size1 )
{
	if( !_b_init ) return;

	int32_t offset;
	int32_t work  ;

	_vc_mtx->lock();
	{
		pxtnxaVOICE_ORDER* po = _vc_odrs;

		for( int i = 0; i < _vc_odr_num; i++, po++ )
		{
			switch( po->type )
			{
			case pxtnxaVOICE_ORDER_none    : break;

			case pxtnxaVOICE_ORDER_new     :
				_log( _T("vc odr new %d proc\n"), po->play_id );
				_voice_set_new( po );
				break;

			case pxtnxaVOICE_ORDER_freq    :

				_log( _T("vc odr freq %d proc\n"), po->play_id );
				for( int t = 0; t < _vc_max_tone; t++ )
				{
					if( _vc_tones[ t ].b_act && _vc_tones[ t ].play_id == po->play_id )
					{
						_vc_tones[ t ].freq_rate = po->freq_rate; break;
					}
				}
				break;

			case pxtnxaVOICE_ORDER_stop    :

				_log( _T("vc odr stop %d proc\n"), po->play_id );

				for( int t = 0; t < _vc_max_tone; t++ )
				{
					if( _vc_tones[ t ].b_act && _vc_tones[ t ].play_id == po->play_id )
					{
						_vc_tones[ t ].b_on = false;
						if( po->b_force_stop )
						{
							_vc_tones[ t ].b_act = false;
							for( int v = 0; v < _vc_tones[ t ].voice_num; v++ ) _vc_tones[ t ].voitones[ v ].life_count = 0;
						}
						else
						{
							pxtnVOICETONE *p_vt;
							for( int v = 0; v < _vc_tones[ t ].voice_num; v++ )
							{
								p_vt            = &_vc_tones[ t ].voitones[ v ];
								p_vt->env_start = p_vt->env_volume;
								p_vt->env_pos   = 0;
							}
						}
					}
				}
				break;


			case pxtnxaVOICE_ORDER_mstr_vol:
				_vc_master_vol = po->mstr_vol;
				if( _vc_master_vol > 1 ) _vc_master_vol = 1;
				if( _vc_master_vol < 0 ) _vc_master_vol = 0;
				break;

			case pxtnxaVOICE_ORDER_stop_all:

				for( int t = 0; t < _vc_max_tone; t++ )
				{
					if( _vc_tones[ t ].b_act )
					{
						_vc_tones[ t ].b_on = false;
						for( int v = 0; v < _vc_tones[ t ].voice_num; v++ ) _vc_tones[ t ].voitones[ v ].life_count = 0;
					}
				}
				break;
			}

			po->type = pxtnxaVOICE_ORDER_none;
		}
	}
	_vc_mtx->unlock();

	int t = 0;

	// 16bits.

	short word[ 2 ];
	short *p;
	int32_t   size;

	p    = (short *)p1;
	size = size1 / 2;
	for( int b = 0; b < size; b++ )
	{
		offset = ( t % _dst_ch_num );
		if( !offset ){ _Voice_Envelope(); _Voice_MakeSample( word ); _Voice_ToNextSample(); }
		work = p[ b ] + word[ offset ];
		if( work  < -0x7ffe ) work = -0x7ffe;
		if( work  >  0x7ffe ) work =  0x7ffe;
		p[ b ] = (short)work;
		t++;
	}
}


void pxtonewinXA2::_Voice_Release()
{
	pxMem_free( (void**)&_vc_tones );
	pxMem_free( (void**)&_vc_odrs  );

	SAFE_DELETE( _vc_freq );
	SAFE_DELETE( _vc_mtx  );
	SAFE_DELETE( _tn_mtx  );

	_vc_max_tone = 0;
}

bool pxtonewinXA2::_Voice_Init( int32_t max_tone, int32_t max_order )
{
	bool b_ret = false;

	_vc_master_vol       =      1.0f;
	_vc_max_tone         = max_tone ;
	_vc_odr_num          = max_order;
	_vc_time_pan_idx     =         0;

	if( !(_vc_freq = new pxtnPulse_Frequency( _io_read, _io_write, _io_seek, _io_pos )) ||  !_vc_freq->Init() ) goto term;
	if( !(_vc_mtx  = new pxwMutex           ()) ||  !_vc_mtx->init () ) goto term;

	if( !pxMem_zero_alloc( (void**)&_vc_tones, sizeof(STREAMINGVOICETONE2) * max_tone  ) ) goto term;
	if( !pxMem_zero_alloc( (void**)&_vc_odrs , sizeof(pxtnxaVOICE_ORDER  ) * max_order ) ) goto term;

	if( !(_tn_mtx  = new pxwMutex           ()) ||  !_tn_mtx->init () ) goto term;

	b_ret   = true;
term:
	if( !b_ret ) _Voice_Release();

	return b_ret;
}

uint32_t pxtonewinXA2::Voice_order_new( const pxtnxaVOICE_ORDER* odr )
{
	if( !_b_init ) return 0;
	uint32_t play_id = 0;

	_vc_mtx->lock();

	pxtnxaVOICE_ORDER* po = _vc_odrs;

	for( int i = 0; i < _vc_odr_num; i++, po++ )
	{
		if( po->type == pxtnxaVOICE_ORDER_none )
		{
			_vc_play_id_idx++; if( !_vc_play_id_idx ) _vc_play_id_idx = 1;

			*po = *odr;

			po->play_id   = _vc_play_id_idx;
			po->type      = pxtnxaVOICE_ORDER_new;
			play_id       = po->play_id;
			break;
		}
	}
	_vc_mtx->unlock();

	return play_id;
}

bool pxtonewinXA2::Voice_order_freq        ( uint32_t play_id, float freq_rate )
{
	if( !_b_init ) return false;
	_vc_mtx->lock();

	pxtnxaVOICE_ORDER* po    = _vc_odrs;
	bool               b_ret = false   ;

	for( int i = 0; i < _vc_odr_num; i++, po++ )
	{
		if( po->type == pxtnxaVOICE_ORDER_none )
		{
			memset( po, 0, sizeof(pxtnxaVOICE_ORDER) );
			po->play_id   = play_id;
			po->freq_rate = freq_rate;
			po->type      =pxtnxaVOICE_ORDER_freq;
			b_ret = true;
			break;
		}
	}

	_vc_mtx->unlock();
	return b_ret;
}

bool pxtonewinXA2::Voice_order_stop        ( int32_t play_id, bool b_force )
{
	if( !_b_init ) return false;
	if( !play_id ) return false;
	_vc_mtx->lock();

	pxtnxaVOICE_ORDER* po    = _vc_odrs;
	bool               b_ret = false   ;

	for( int i = 0; i < _vc_odr_num; i++, po++ )
	{
		if( po->type == pxtnxaVOICE_ORDER_none  )
		{
			memset( po, 0, sizeof(pxtnxaVOICE_ORDER) );
			po->play_id      = play_id;
			po->type         = pxtnxaVOICE_ORDER_stop;
			po->b_force_stop = b_force;
			b_ret = true;
			break;
		}
	}
	_vc_mtx->unlock();
	return true;
}

bool pxtonewinXA2::Voice_order_stop_all()
{
	if( !_b_init ) return false;
	_vc_mtx->lock();

	pxtnxaVOICE_ORDER* po    = _vc_odrs;
	bool               b_ret = false   ;

	for( int i = 0; i < _vc_odr_num; i++, po++ )
	{
		if( po->type == pxtnxaVOICE_ORDER_none ) memset( po, 0, sizeof(pxtnxaVOICE_ORDER) );
	}

	_vc_odrs[ 0 ].type = pxtnxaVOICE_ORDER_stop_all;

	b_ret = true;

	_vc_mtx->unlock();
	return b_ret;
}

bool pxtonewinXA2::Voice_order_MasterVolume( float vol )
{
	if( !_b_init ) return false;
	_vc_mtx->lock();

	pxtnxaVOICE_ORDER* po    = _vc_odrs;
	bool               b_ret = false   ;

	for( int i = 0; i < _vc_odr_num; i++, po++ )
	{
		if( po->type == pxtnxaVOICE_ORDER_none  )
		{
			memset( po, 0, sizeof(pxtnxaVOICE_ORDER) );
			po->type     = pxtnxaVOICE_ORDER_mstr_vol;
			po->mstr_vol = vol ;
			b_ret        = true;
			break;
		}
	}
	_vc_mtx->unlock();
	return b_ret;
}
