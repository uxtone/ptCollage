
#include <pxtnPulse_NoiseBuilder.h>
#include <uxStr.h>
extern pxtnPulse_NoiseBuilder* g_noise_bldr;

#include <pxtonewinXA2.h>
extern pxtonewinXA2*           g_strm_xa2  ;

#include <pxtnWoice.h>

#include "PcmTable.h"

static bool      _b_init  = false;
static pxtnWoice *_woice  = NULL;
static pxtnWoice *_scoped = NULL;

int32_t GetCompileVersion( int32_t *p1, int32_t *p2, int32_t *p3, int32_t *p4 );

/////////////////
// グローバル
/////////////////

// PCM取得

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


bool PcmTable_Initialize()
{
	_woice  = new pxtnWoice( _io_read, _io_write, _io_seek, _io_pos );
	_scoped = new pxtnWoice( _io_read, _io_write, _io_seek, _io_pos );

	if( !_woice ->Voice_Allocate( 1 ) ) return false;
	if( !_scoped->Voice_Allocate( 1 ) ) return false;

	_woice ->get_voice_variable( 0 )->type = pxtnVOICE_Sampling;
	_scoped->get_voice_variable( 0 )->type = pxtnVOICE_Sampling;

	_b_init = true;
	return true;
}

void PcmTable_Release()
{
	_b_init = false;
	SAFE_DELETE( _woice  );
	SAFE_DELETE( _scoped );
}

// ロード
bool PcmTable_Wave_Load( const uxDS& path )
{
	if( !_b_init ) return false;

	bool  b_ret = false;
	FILE* fp    = ux_fopen( path, "rb" ); if( !fp ) goto End;

	if( !_woice->Voice_Allocate( 1 ) ) goto End;
	_woice->get_voice_variable( 0 )->type = pxtnVOICE_Sampling;

	if( _woice->get_voice_variable( 0 )->p_pcm->read( fp ) != pxtnOK ) goto End;

	b_ret = true;
End:
	if( fp ) fclose( fp );

	return b_ret;
}

// セーブ
bool PcmTable_Wave_Save( const uxDS& path )
{
	char text[ 64 ] = {0};
	sprintf( text, "ptNoise v%04d ", GetCompileVersion( 0, 0, 0, 0 ) ); // 文字偶数
	text[ 0 ] = '\0';

	FILE* fp = ux_fopen( path, "wb" ); if( !fp ) return false;
	if( !_woice->get_voice( 0 )->p_pcm->write( fp, text ) )
	{
		fclose( fp );
		return false;
	}
	fclose( fp );
	return true;
}


// Woice
static int32_t _id_woice = 0;

pxtnPulse_PCM* PcmTable_Woice_GetPcmPointer()
{
	if( !_woice->get_voice_num() ) return NULL;
	return _woice->get_voice_variable( 0 )->p_pcm;
}

bool PcmTable_Woice_Update()
{
	int sps;
	g_strm_xa2->stream_get_quality_safe( NULL, &sps, NULL );
	if( _woice ->Tone_Ready( g_noise_bldr, sps ) != pxtnOK ) return false;
	if( _scoped->Tone_Ready( g_noise_bldr, sps ) != pxtnOK ) return false;
	return true;
}

void PcmTable_Woice_PCM_ConvertVolume( float v )
{
	_woice->get_voice_variable( 0 )->p_pcm->Convert_Volume( v );
}

void PcmTable_Woice_Play( bool b_loop )
{
	if( !_woice->get_voice( 0 )->p_pcm->get_p_buf() ) return;
	{
		pxtnxaVOICE_ORDER odr; memset( &odr, 0, sizeof(odr) );
		odr.p_woice   = _woice;
		odr.b_loop    = b_loop;
		odr.freq_rate =      1;
		odr.vol_pan   =     -1;
		odr.time_pan  =     -1;
		for( int i = 0; i < pxtnMAX_UNITCONTROLVOICE; i++ ) odr.velos[ i ] = -1;
		_id_woice = g_strm_xa2->Voice_order_new( &odr );
	}
}

void PcmTable_Woice_Stop( bool b_force )
{
	if( _id_woice ) g_strm_xa2->Voice_order_stop( _id_woice, b_force );
	_id_woice = 0;
}

void PcmTable_Scoped_Play()
{
	if( !_scoped->get_voice( 0 )->p_pcm->get_p_buf() ) return;

	{
		pxtnxaVOICE_ORDER odr; memset( &odr, 0, sizeof(odr) );
		odr.p_woice   = _scoped;
		odr.b_loop    = true   ;
		odr.freq_rate =       1;
		odr.vol_pan   =      -1;
		odr.time_pan  =      -1;
		for( int i = 0; i < pxtnMAX_UNITCONTROLVOICE; i++ ) odr.velos[ i ] = -1;
		_id_woice = g_strm_xa2->Voice_order_new( &odr );
	}
}

void PcmTable_Scoped_Stop( bool b_force )
{
	if( _id_woice ) g_strm_xa2->Voice_order_stop( _id_woice, b_force );
	_id_woice = 0;
}

bool PcmTable_Woice_SetScoped( int start, int end )
{
	if( end - start <= 0 ) return false;

	if( !_woice->get_voice(0)->p_pcm->Copy_( _scoped->get_voice_variable(0)->p_pcm, start, end ) ) return false;

	int sps;
	g_strm_xa2->stream_get_quality_safe( NULL, &sps, NULL );
	if( _scoped->Tone_Ready( g_noise_bldr, sps ) != pxtnOK ) return false;
	return true;
}

extern HWND g_hWnd_Main;

bool PcmTable_BuildAndPlay( pxtnPulse_Noise *p_noise, const SAMPLINGQUALITY *p_quality )
{
	PcmTable_Woice_Stop( true );
	pxtnPulse_PCM *p_pcm = NULL;
	if( !( p_pcm = g_noise_bldr->BuildNoise( p_noise, p_quality->ch, p_quality->sps, p_quality->bps ) ) ) return false;

	if( !PcmTable_Woice_GetPcmPointer()->copy_from( p_pcm ) ) return false;

	PcmTable_Woice_Update();

	PostMessage( g_hWnd_Main, WM_PAINT, 0, 0 );
	SAFE_DELETE( p_pcm );

	PcmTable_Woice_Play( false );

	return true;
}
