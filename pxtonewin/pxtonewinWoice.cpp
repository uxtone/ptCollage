
#include <stdio.h>
#include <uxStr.h>
#include <cstdint>

#pragma comment(lib, "shlwapi" )
#include <shlwapi.h>

#include <pxtnPulse_Frequency.h>
#include <pxtnWoice.h>
#include <pxtnPulse_NoiseBuilder.h>

#include "./pxtonewinWoice.h"
#include <pxPath.h>

// file name of a UTF-8 path (the whole path when it has no directory part)
static uxDS _file_name( const uxDS& path )
{
	const char* f = pxPath_find_filename( path );
	return uxDS( f ? f : *path );
}


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


void pxtonewinWoice::_release()
{
	_b_init = false;

	SAFE_DELETE( _woice    );
	SAFE_DELETE( _freq     );
	SAFE_DELETE( _ptn_bldr );

	_strm_xa2 = NULL;
	_wave_id  =   -1;
}


pxtonewinWoice:: pxtonewinWoice()
{
	_wave_id      = -1;
	memset( _status_text, 0, sizeof(_status_text ) );
	_b_init       = false;
	_sps          = 0;

	_ptn_bldr = NULL;
	_freq     = NULL;
	_woice    = NULL;
	_strm_xa2 = NULL;
}

pxtonewinWoice::~pxtonewinWoice()
{
	_release();
}


void pxtonewinWoice::set_sps( int sps )
{
	_sps = sps;
}


bool pxtonewinWoice::_load_and_play_PCM( const uxDS& path, bool b_loop, int key, bool* pb_timer )
{
	if( !_b_init ) return false;

	bool           b_ret = false;
	uxDS          p_name;
	float          sec   ;
	int            sps, bps;
	pxtnVOICEUNIT* p_vc  ;

	if( !_woice->Voice_Allocate( 1 ) ) goto term;

	p_vc = _woice->get_voice_variable( 0 );

	{
		FILE* fp = ux_fopen( path, "rb" ); if( !fp ) goto term;
		if( p_vc->p_pcm->read( fp ) != pxtnOK ) goto term;
	}

	p_vc->type = pxtnVOICE_Sampling;

	// コントロール表示
	p_name = _file_name( path );
	sec    = p_vc->p_pcm->get_sec();
	sps    = p_vc->p_pcm->get_sps();
	bps    = p_vc->p_pcm->get_bps();
	if( p_vc->p_pcm->get_ch() == 1 ) ux_sprintf_s( _status_text, MAX_PATH, "%s\r\n Mono\r\n %d Hz\r\n %d bps\r\n %0.2f sec", *p_name, sps, bps, sec );
	else                             ux_sprintf_s( _status_text, MAX_PATH, "%s\r\n Stereo\r\n %d Hz\r\n %d bps\r\n %0.2f sec", *p_name, sps, bps, sec );

	// 変換
	if( _woice->Tone_Ready_sample( _ptn_bldr ) != pxtnOK ) goto term;
	if( sec < 0.005 ) b_loop    = true ;

	{
		pxtnxaVOICE_ORDER odr; memset( &odr, 0, sizeof(odr) );
		odr.p_woice   = _woice      ;
		odr.b_loop    = b_loop   ;
		odr.freq_rate = _freq->Get( key );
		odr.vol_pan   = -1  ;
		odr.time_pan  = -1 ;
		for( int i = 0; i < pxtnMAX_UNITCONTROLVOICE; i++ ) odr.velos[ i ] = -1;
		_wave_id = _strm_xa2->Voice_order_new( &odr );
	}

	if( sec < 0.005 ) *pb_timer = true;

	b_ret = true;
term:

	return b_ret;
}


bool pxtonewinWoice::_load_and_play_PTV( const uxDS& path, int key, bool* pb_timer )
{
	if( !_b_init ) return false;

	bool    b_ret     = false;
	bool    b_new_fmt = false;
	uxDS   p_name    = NULL ;


	if( !_sps ) return false;

	// unit の準備
	{
		FILE* fp = ux_fopen( path, "rb" ); if( !fp ) goto term;
		if( _woice->PTV_Read( fp ) != pxtnOK ){ fclose( fp ); goto term; }
		fclose( fp );
	}

	// コントロール表示
	p_name = _file_name( path );
	ux_sprintf_s( _status_text, MAX_PATH, "%s\r\n%d voice", *p_name, _woice->get_voice_num() );

	// 変換
	if(  _woice->Tone_Ready_sample  ( _ptn_bldr ) != pxtnOK ) goto term;
	if(  _woice->Tone_Ready_envelope( _sps      ) != pxtnOK ) goto term;


	{
		pxtnxaVOICE_ORDER odr; memset( &odr, 0, sizeof(odr) );
		odr.p_woice   = _woice;
		odr.b_loop    = true  ;
		odr.freq_rate = _freq->Get( key );
		odr.vol_pan   =     -1;
		odr.time_pan  =     -1;
		for( int i = 0; i < pxtnMAX_UNITCONTROLVOICE; i++ ) odr.velos[ i ] = -1;
		_wave_id = _strm_xa2->Voice_order_new( &odr );
	}

	*pb_timer = true;

	b_ret     = true;
term:
	return b_ret;
}


bool pxtonewinWoice::_load_and_play_PTN( const uxDS& path, bool b_loop, int key, bool* pb_timer )
{
	if( !_b_init ) return false;

	bool           b_ret     = false;
	bool           b_new_fmt = false;
	uxDS          p_name;
	float          sec ;
	pxtnVOICEUNIT* p_vc;

	if( !_woice->Voice_Allocate( 1 ) ) goto term;

	p_vc = _woice->get_voice_variable( 0 );
	p_vc->type = pxtnVOICE_Noise;

	// unit の準備
	{
		FILE* fp = ux_fopen( path, "rb" ); if( !fp ) goto term;
		if( p_vc->p_ptn->read( fp ) != pxtnOK ){ fclose( fp ); goto term; }
		fclose( fp );
	}

	// コントロール表示
	p_name = _file_name( path );
	sec = p_vc->p_ptn->get_sec();
	ux_sprintf_s( _status_text, MAX_PATH, "%s\r\n%0.2f sec", *p_name, sec );

	if( _woice->Tone_Ready_sample( _ptn_bldr ) != pxtnOK ) goto term;

	if( sec < 0.005 ) b_loop    = true ;

	{
		pxtnxaVOICE_ORDER odr; memset( &odr, 0, sizeof(odr) );
		odr.p_woice   = _woice;
		odr.b_loop    = b_loop;
		odr.freq_rate = _freq->Get( key );
		odr.vol_pan   =     -1;
		odr.time_pan  =     -1;
		for( int i = 0; i < pxtnMAX_UNITCONTROLVOICE; i++ ) odr.velos[ i ] = -1;
		_wave_id = _strm_xa2->Voice_order_new( &odr );
	}

	if( sec < 0.005 )  *pb_timer = true;

	b_ret = true;
term:

	return b_ret;
}

bool pxtonewinWoice::_load_and_play_OGGV( const uxDS& path, bool b_loop, int key, bool* pb_timer )
{
	if( !_b_init ) return false;

	bool  b_ret = false;

#ifdef pxINCLUDE_OGGVORBIS

	uxDS  p_name;

	if( !_woice->Voice_Allocate( 1 ) ) goto term;

	static pxtnVOICEUNIT* p_vc;
	p_vc    = _woice->get_voice_variable( 0 );//&p_w->p_vcs[ 0 ];
	p_vc->type = pxtnVOICE_OggVorbis;

	static pxDescriptor *desc = nullptr;
	{
		static FILE* fp;
		fp = ux_fopen( path, "rb" ); if( !fp ) goto term;

		if(desc != nullptr)
			delete desc;

		desc = new pxDescriptor;

		desc->set_file_r( fp );
		if( !desc->set_file_r( fp ) || p_vc->p_oggv->ogg_read( &desc ) != pxtnOK ) goto term;
	}

	p_name = _file_name( path );

	static int ch, smp_num, sps;
	ch = {}; smp_num = {}; sps = {};

	if( !p_vc->p_oggv->GetInfo( &ch, &sps, &smp_num ) ) goto term;

	static float sec;
	sec = (float)smp_num / (float)sps;
	if( ch == 1 ) ux_sprintf_s( _status_text, MAX_PATH, "%s\r\n Mono\r\n %d Hz\r\n %0.2f sec", *p_name, sps, sec );
	else          ux_sprintf_s( _status_text, MAX_PATH, "%s\r\n Stereo\r\n %d Hz\r\n %0.2f sec", *p_name, sps, sec );

	if( _woice->Tone_Ready_sample( _ptn_bldr ) != pxtnOK ) goto term;

	{
		pxtnxaVOICE_ORDER odr; memset( &odr, 0, sizeof(odr) );
		odr.p_woice   = _woice;
		odr.b_loop    = b_loop;
		odr.freq_rate = _freq->Get( key );
		odr.vol_pan   =     -1;
		odr.time_pan  =     -1;
		for( int i = 0; i < pxtnMAX_UNITCONTROLVOICE; i++ ) odr.velos[ i ] = -1;
		_wave_id = _strm_xa2->Voice_order_new( &odr );
	}

	if( sec < 0.005 ) *pb_timer = true;
#else
	goto term;
#endif

	b_ret = true;
term:
	if(desc)
		delete desc;

	return b_ret;
}

bool pxtonewinWoice::init( pxtonewinXA2 *strm_xa2 )
{
	if( _b_init   ) return false;
	if( !strm_xa2 ) return false;

	if( !( _freq     = new pxtnPulse_Frequency   ( _io_read, _io_write, _io_seek, _io_pos ) ) || !_freq    ->Init() ) goto term;
	if( !( _woice    = new pxtnWoice             ( _io_read, _io_write, _io_seek, _io_pos ) )                       ) goto term;
	if( !( _ptn_bldr = new pxtnPulse_NoiseBuilder( _io_read, _io_write, _io_seek, _io_pos ) ) || !_ptn_bldr->Init() ) goto term;
	_b_init   = true;
	_strm_xa2 = strm_xa2;
term:
	if( !_b_init ) _release();
	return _b_init;
}


bool pxtonewinWoice::load_and_play( const uxDS& path, bool b_loop, int key, bool* pb_timer )
{
	if( !_b_init ) return false;

	bool         b_ret = false;
	const char* p_ext = pxPath_find_ext( path );
	if( !p_ext ) return false;

	if(      !ux_stricmp( p_ext, ".wav" ) ) _load_and_play_PCM ( path, b_loop, key, pb_timer );
	else if( !ux_stricmp( p_ext, ".ptvoice" ) ) _load_and_play_PTV ( path,         key, pb_timer );
	else if( !ux_stricmp( p_ext, ".ptnoise" ) ) _load_and_play_PTN ( path, b_loop, key, pb_timer );
	else if( !ux_stricmp( p_ext, ".ogg" ) ) _load_and_play_OGGV( path, b_loop, key, pb_timer );
	else goto End;

	b_ret = true;
End:

	return b_ret;
}

void pxtonewinWoice::stop( bool b_force )
{
	if( !_b_init ) return;
	_strm_xa2->Voice_order_stop( _wave_id, b_force );
}

bool pxtonewinWoice::get_text( uxDS& txt_info, bool b_jp ) const
{
	if( !_b_init      ) return false;
	if( !_status_text ) return false;
	txt_info = _status_text;
	return true;
}
