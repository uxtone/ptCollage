
#include <pxMem.h>

#include <pxtnWoice.h>

#include "./ptVoice.h"
#include "./WoiceUnit.h"
#include "./ptvVelocity.h"

WoiceUnit:: WoiceUnit()
{
	_b_init           = false;
	_xa2              = NULL ;
	_woice            = NULL ;
	_channel          =     0;
	_zoom             =  1.0f;
	_frame_alte_flags =     0;

	_mode             = WOICEUNIT_MODE_rack;
	_b_sine_over      = false;

	for( int i = 0; i < pxtnMAX_UNITCONTROLVOICE; i++ ) _mutes[ i ] = false;
}

WoiceUnit::~WoiceUnit()
{
	_b_init = false;

	SAFE_DELETE( _woice );
}

WOICEUNIT_MODE WoiceUnit::mode_get(                     ) const
{
	return _mode;
}

void           WoiceUnit::mode_set( WOICEUNIT_MODE mode )
{
	_mode = mode;
}

void     WoiceUnit::mute_switch( int32_t channel )
{
	_mutes[ channel ] = _mutes[ channel ] ? false : true;
}
bool     WoiceUnit::mute_get   ( int32_t channel ) const
{
	return _mutes[ channel ];
}


void     WoiceUnit::frame_alte_set  ( uint32_t alte_flags, int32_t channel )
{
	int ch = _channel;
	if( channel >= 0 ) ch = channel;

	if( alte_flags & WOICEUNIT_ALTE_wave_screen )
	{
		switch( ch )
		{
		case 0: alte_flags |= WOICEUNIT_ALTE_wave_mini0; break;
		case 1: alte_flags |= WOICEUNIT_ALTE_wave_mini1; break;
		}
	}
	if( alte_flags & WOICEUNIT_ALTE_enve_screen )
	{
		switch( ch )
		{
		case 0: alte_flags |= WOICEUNIT_ALTE_enve_mini0; break;
		case 1: alte_flags |= WOICEUNIT_ALTE_enve_mini1; break;
		}
	}
	_frame_alte_flags |= alte_flags;
}
void     WoiceUnit::frame_alte_clear()      {        _frame_alte_flags  =          0; }
uint32_t WoiceUnit::frame_alte_get  () const{ return _frame_alte_flags;               }

void WoiceUnit::default_overtone( pxtnVOICEWAVE *p_wave, bool b_zero )
{
	int32_t o;
	p_wave->num = ptvFIXNUM_WAVE_POINT;
	for( o = 0; o < ptvFIXNUM_WAVE_POINT; o++ )
	{
		p_wave->points[ o ].x = o + 1;
		p_wave->points[ o ].y = 0;
	}
	if( !b_zero ) p_wave->points[ 0 ].y =  128;
	_frame_alte_flags |= WOICEUNIT_ALTE_WAVE;
}

void WoiceUnit::default_coodinate( pxtnVOICEWAVE *p_wave, bool b_zero )
{
	p_wave->num  = 1;
	p_wave->reso = ptvCOODINATERESOLUTION;
	memset( p_wave->points, 0, sizeof(POINT) * ptvFIXNUM_WAVE_POINT );

	if( !b_zero )
	{
		p_wave->num           =      3;
		p_wave->points[ 0 ].x =    0; p_wave->points[ 0 ].y =    0;
		p_wave->points[ 1 ].x =   64; p_wave->points[ 1 ].y =   64;
		p_wave->points[ 2 ].x =  192; p_wave->points[ 2 ].y =  -64;
	}
	_frame_alte_flags |= WOICEUNIT_ALTE_WAVE;
}

void WoiceUnit::_envelope_default( pxtnVOICEENVELOPE* pe )
{
	pe->fps           = ptvDEFAULT_ENVESPS_new;
	pe->head_num      = ptvMAX_ENVELOPEPOINT  ;
	pe->body_num      =                      0;
	pe->tail_num      = pxtnMAX_RELEASEPOINT  ;

	memset( pe->points, 0, sizeof(POINT) * ptvFIXNUM_ENVE_POINT );

	pe->points[ 0 ].y =  96;
	pe->points[ 0 ].x =   0;
	pe->points[ ptvMAX_ENVELOPEPOINT ].y =   0;
	pe->points[ ptvMAX_ENVELOPEPOINT ].x =   1;

	_frame_alte_flags |= WOICEUNIT_ALTE_ENVE;
}

void WoiceUnit::data_reset()
{
	for( int i = 0; i < pxtnMAX_UNITCONTROLVOICE; i++ )
	{
		pxtnVOICEUNIT* p_vc = _woice->get_voice_variable( i );

		p_vc->basic_key   = 0x4500 - 0x0c00 * 2;
		p_vc->volume      =  64;
		p_vc->pan         =  64;
		p_vc->tuning      =   1;
		p_vc->type        = pxtnVOICE_Coodinate;
		p_vc->voice_flags = PTV_VOICEFLAG_SMOOTH | PTV_VOICEFLAG_WAVELOOP;
		p_vc->data_flags  = PTV_DATAFLAG_WAVE    | PTV_DATAFLAG_ENVELOPE ;

		default_coodinate( &p_vc->wave, i ? true : false );
		_envelope_default( &p_vc->envelope );
		if( i ) p_vc->volume = 0;

		_mutes[ i ] = false;
	}
}

pxtnWoice *WoiceUnit::get_woice(){ return _woice; }

bool WoiceUnit::init( pxtonewinXA2* xa2 )
{
	if( _b_init ) return false;
	if( !xa2    ) return false;

	pxtnVOICEUNIT* p_vc  = NULL;
	int32_t        v     = 0;
	int32_t        num   = 0;

	_xa2   = xa2;
	_woice = new pxtnWoice( NULL, NULL, NULL, NULL );

	if( !_woice->Voice_Allocate( pxtnMAX_UNITCONTROLVOICE ) ) goto End;

	for( v = 0; v < pxtnMAX_UNITCONTROLVOICE; v++ )
	{
		p_vc = _woice->get_voice_variable( v );

		p_vc->wave.num          = ptvFIXNUM_WAVE_POINT        ;
		p_vc->envelope.head_num = ptvMAX_ENVELOPEPOINT   ;
		p_vc->envelope.body_num =                    0;
		p_vc->envelope.tail_num = pxtnMAX_RELEASEPOINT;

		num = ptvMAX_ENVELOPEPOINT + pxtnMAX_RELEASEPOINT;

		if( !pxMem_zero_alloc( (void **)&p_vc->wave.points    , sizeof(POINT) * ptvFIXNUM_WAVE_POINT ) ) goto End;
		if( !pxMem_zero_alloc( (void **)&p_vc->envelope.points, sizeof(POINT) * ptvFIXNUM_ENVE_POINT ) ) goto End;
	}
	data_reset();
	_b_init = true;
End:
	if( !_b_init ) _woice->Voice_Release();

	return _b_init;
}

void WoiceUnit::reset_overtone( pxtnVOICEWAVE *p_wave, int32_t index )
{
	int32_t o;
	p_wave->num = ptvFIXNUM_WAVE_POINT;
	for( o = 0; o < ptvFIXNUM_WAVE_POINT; o++ )
	{
		p_wave->points[ o ].x = o + 1;
		p_wave->points[ o ].y = 0;
	}
	if( !index ) p_wave->points[ 0 ].y =  128;
}

static bool _copy_voice( pxtnVOICEUNIT* p_dst, const pxtnVOICEUNIT* p_src )
{
	if( !p_dst || !p_src ) return false;

	p_dst->tuning            = p_src->tuning           ;
	p_dst->type              = p_src->type             ;
	p_dst->voice_flags       = p_src->voice_flags      ;
	p_dst->data_flags        = p_src->data_flags       ;
	p_dst->basic_key         = p_src->basic_key        ;
	p_dst->pan               = p_src->pan              ;
	p_dst->volume            = p_src->volume           ;

	p_dst->envelope.fps      = p_src->envelope.fps     ;
	p_dst->envelope.head_num = p_src->envelope.head_num;
	p_dst->envelope.body_num = p_src->envelope.body_num;
	p_dst->envelope.tail_num = p_src->envelope.tail_num;
	p_dst->wave.num          = p_src->wave.num         ;

	for( int i = 0; i < ptvFIXNUM_WAVE_POINT; i++ ) p_dst->wave    .points[ i ] = p_src->wave    .points[ i ];
	for( int i = 0; i < ptvFIXNUM_ENVE_POINT; i++ ) p_dst->envelope.points[ i ] = p_src->envelope.points[ i ];

	p_dst->p_pcm->copy_from( p_src->p_pcm ); // pcm isn't used on this version.

	return true;
}

bool WoiceUnit::copy_ptv_voice( int32_t src_idx, int32_t dst_idx )
{
	if( src_idx == dst_idx ) return true;

	if( !_copy_voice( _woice->get_voice_variable( dst_idx ), _woice->get_voice( src_idx ) ) ) return false;
	return true;
}

bool WoiceUnit::copy_ptv_woice_to( WoiceUnit* p_dst ) const
{
	if( _woice->get_voice_num() != 2 ) return false;

	for( int v = 0; v < 2; v++ )
	{
		if( !_copy_voice( p_dst->get_voice( v ), _woice->get_voice( v ) ) ) return false;
	}
	return true;
}


bool    WoiceUnit::channel_set( int32_t ch )
{
	if( ch == _channel ) return false;
	if( ch < 0                 ) ch = 0;
	if( ch > pxtnMAX_UNITCONTROLVOICE - 1 ) ch = pxtnMAX_UNITCONTROLVOICE - 1;
	_channel = ch;
	_frame_alte_flags |= WOICEUNIT_ALTE_enve_screen|WOICEUNIT_ALTE_wave_screen;
	return true;
}

void    WoiceUnit::channel_switch()
{
	_channel++;
	if( _channel >= pxtnMAX_UNITCONTROLVOICE ) _channel = 0;
	_frame_alte_flags |= WOICEUNIT_ALTE_channel;
}

int32_t WoiceUnit::channel_get() const
{
	return _channel;
}

bool    WoiceUnit::zoom_set( float zoom )
{
	if( _zoom == zoom ) return false;
	_zoom = zoom;
	_frame_alte_flags |= WOICEUNIT_ALTE_enve_screen;
	return true;
}
float   WoiceUnit::zoom_get(            ) const { return _zoom; }


pxtnVOICEUNIT* WoiceUnit::get_voice( int32_t index )
{
	return _woice->get_voice_variable( index );
}

pxtnVOICEUNIT* WoiceUnit::get_voice()
{
	return _woice->get_voice_variable( _channel );
}

bool WoiceUnit::IDM_INITIALIZE_()
{
	data_reset();
	int sps = 0; _xa2->stream_get_quality_safe( NULL, &sps, NULL );
	if( _woice->Tone_Ready( NULL, sps ) != pxtnOK ) return false;
	return true;
}

bool WoiceUnit::strm_ready_sample()
{
	_xa2->Voice_order_stop_all();
	if( _woice->Tone_Ready_sample( NULL ) != pxtnOK ) return false;
	return true;
}

bool WoiceUnit::strm_ready_envelope()
{
	_xa2->Voice_order_stop_all();

	int sps;
	_xa2->stream_get_quality_safe( NULL, &sps, NULL );

	if( _woice->Tone_Ready_envelope( sps ) != pxtnOK ) return false;

	return true;
}

bool WoiceUnit::strm_ready()
{
	_xa2->Voice_order_stop_all();

	int sps; _xa2->stream_get_quality_safe( NULL, &sps, NULL );

	if( _woice->Tone_Ready( NULL, sps ) != pxtnOK ) return false;

	return true;
}

int32_t WoiceUnit::strm_ON     ( float freq_rate                     )
{
	int32_t velo = ptvVelocity_Get();

	{
		pxtnxaVOICE_ORDER odr; memset( &odr, 0, sizeof(odr) );
		odr.p_woice   = _woice;
		odr.b_loop    = true  ;
		odr.freq_rate = freq_rate;
		odr.vol_pan   =     -1;
		odr.time_pan  =     -1;
		for( int i = 0; i < pxtnMAX_UNITCONTROLVOICE; i++ )
		{
			odr.velos[ i ] = _mutes[ i ] ? 0 : velo;
		}
		return _xa2->Voice_order_new( &odr );
	}
//	return _xa2->Voice_order_new ( _woice, true, freq_rate, -1, ptvVelocity_Get(), -1 );
}
int32_t WoiceUnit::strm_ON     ( float freq_rate, unsigned char velo )
{
	{
		pxtnxaVOICE_ORDER odr; memset( &odr, 0, sizeof(odr) );
		odr.p_woice   = _woice;
		odr.b_loop    = true  ;
		odr.freq_rate = freq_rate;
		odr.vol_pan   =     -1;
		odr.time_pan  =     -1;
		for( int i = 0; i < pxtnMAX_UNITCONTROLVOICE; i++ )
		{
			odr.velos[ i ] = _mutes[ i ] ? 0 : velo;
		}
		return _xa2->Voice_order_new( &odr );
	}
}
void    WoiceUnit::strm_OFF    ( int32_t id, bool bForce             ){        _xa2->Voice_order_stop( id, bForce    ); }
void    WoiceUnit::strm_CHANGE ( int32_t id, float freq_rate         ){        _xa2->Voice_order_freq( id, freq_rate ); }
void    WoiceUnit::strm_Off_All(                                     ){        _xa2->Voice_order_stop_all(); }



