// '16/01/19 pxRiffWav.cpp

#define _CRT_SECURE_NO_WARNINGS

#include "./pxRiffWav.h"



typedef struct
{
	unsigned short formatID;      // PCM:0x0001
	unsigned short ch;            // mono:1 / stereo:2
	unsigned long  sps;           // sampling per second
	unsigned long  byte_per_sec;
	unsigned short block_size;
	unsigned short bps;           // bit per sample

}
_RIFFHEADER;

pxRiffWav::pxRiffWav()
{
	_b_header = false;

	_smp_r    =     0;

	_ch_num   =     0;
	_sps      =     0;
	_bps      =     0;
	_smp_num  =     0;

	_byte_per_smp = 0;
}

static const char *_header_code_RIFF = "RIFF";
static const char *_header_code_WAVE = "WAVE";
static const char *_header_code_fmt  = "fmt ";
static const char *_header_code_data = "data";


bool pxRiffWav::read_header( pxDescriptor* desc, int32_t *p_ch_num, int32_t *p_sps, int32_t *p_bps, int32_t *p_smp_num )
{
	if( !desc    ) return false;

	_b_header = false;

	char          chunk_name[ 4 ] = {0};
	unsigned long chunk_size      =  0 ;

	_RIFFHEADER   fmt = {0};


	// 'RIFFxxxxWAVE'
	if( !desc->r( chunk_name, 1, 4 ) ) goto End;
	if( memcmp( chunk_name , _header_code_RIFF, 4 ) ) goto End;
	if( !desc->r( &chunk_size, 4, 1 ) ) goto End;
	if( !desc->r( chunk_name , 1, 4 ) ) goto End;
	if( memcmp( chunk_name , _header_code_WAVE, 4 ) ) goto End;

	// 'fmt '
	if( !desc->r( chunk_name , 1, 4 ) ) goto End;
	if( !desc->r( &chunk_size, 4, 1 ) ) goto End;
	if( memcmp( chunk_name , _header_code_fmt , 4 ) ) goto End;
	if( chunk_size < sizeof(fmt)                    ) goto End;
	if( !desc->r( &fmt, sizeof(fmt), 1 ) ) goto End;
	if( chunk_size > sizeof(fmt) )
		//fseek( fp, chunk_size - sizeof(fmt),SEEK_CUR );
		if( !desc->seek( pxSEEK_cur, chunk_size - sizeof(fmt) ) ) goto End;

	if( fmt.ch  != 1 && fmt.ch  !=  2 ) goto End;
	if( fmt.bps != 8 && fmt.bps != 16 ) goto End;

	_bps    = fmt.bps;
	_sps    = fmt.sps;
	_ch_num = fmt.ch ;

	_byte_per_smp = _bps / 8 * _ch_num;

	do
	{
		if( !desc->r( &chunk_name, 4, 1 ) ) goto End;
		if( !desc->r( &chunk_size, 4, 1 ) ) goto End;
		if( !memcmp( chunk_name, _header_code_data, 4 )  ) break;
		if( !desc->seek( pxSEEK_cur, chunk_size ) ) goto End;
	}
	while( 1 );

	_smp_num = chunk_size / fmt.ch / (fmt.bps/8);

	_smp_r  =     0;

	*p_ch_num  =    _ch_num ;
	*p_sps     =    _sps    ;
	*p_bps     =    _bps    ;
	*p_smp_num =    _smp_num;

	_b_header  = true ;
End:
	return _b_header;
}

int32_t  pxRiffWav::read_sample( pxDescriptor* desc, void *p_dst, int32_t smp_num )
{
	if( !_b_header ) return 0;
	if( !desc      ) return 0;

	bool b_ret = false;

	if( smp_num > _smp_num - _smp_r ) smp_num = _smp_num - _smp_r;
	if( smp_num > 0 )
	{
		int32_t req_size = smp_num * _byte_per_smp;
		if( !desc->r( p_dst, 1, req_size ) ) goto End;
		_smp_r += smp_num;
	}

	b_ret = true;
End:
	return b_ret ? smp_num : 0;
}

bool pxRiffWav_save ( const uxDS& path_dst, int32_t ch_num, int32_t sps, int32_t bps, int32_t smp_num, const void* p_src )
{
	bool b_ret = false;

	FILE *fp   = NULL ;
	unsigned long chunk_size = 0;

	_RIFFHEADER   fmt;
	int32_t       byte_per_smp = bps / 8 * ch_num;
	fpos_t        pos;

	fmt.formatID     = 0x0001;
	fmt.ch           = ch_num;
	fmt.sps          = sps   ;
	fmt.bps          = bps   ;
	fmt.byte_per_sec = bps / 8 * sps * ch_num;
	fmt.block_size   = bps / 8       * ch_num;

	if( !( fp = ux_fopen( path_dst, "wb" ) ) ) goto End;

	// 'RIFFxxxxWAVE'
	if( fwrite( _header_code_RIFF, 1, 4, (FILE*)fp ) != 4 ) goto End;
	if( fwrite( &chunk_size      , 4, 1, (FILE*)fp ) != 1 ) goto End;
	if( fwrite( _header_code_WAVE, 1, 4, (FILE*)fp ) != 4 ) goto End;

	// 'fmt '
	if( fwrite( _header_code_fmt , 1, 4, (FILE*)fp ) != 4 ) goto End;
	chunk_size = sizeof(fmt);
	if( fwrite( &chunk_size      , 4, 1, (FILE*)fp ) != 1 ) goto End;
	if( fwrite( &fmt, sizeof(fmt),    1, (FILE*)fp ) != 1 ) goto End;

	// 'data'
	if( fwrite( _header_code_data, 1, 4, (FILE*)fp ) != 4 ) goto End;
	chunk_size = byte_per_smp * smp_num;
	if( fwrite( &chunk_size      , 4, 1, (FILE*)fp ) != 1 ) goto End;
	if( fwrite( (uint8_t*)p_src, byte_per_smp, smp_num, fp ) != smp_num ) goto End;

	if( fgetpos( fp, &pos        )                        ) goto End;
	if( fseek  ( fp, 4, SEEK_SET )                        ) goto End;
	chunk_size = pxFPOS_OFFSET(pos) - 8;   // chunk_size = (int32_t)pos - 8
	if( fwrite( &chunk_size      , 4, 1, (FILE*)fp ) != 1 ) goto End;

	b_ret = true;
End:
	if( fp ) fclose( fp );
	return b_ret;
}

pxRiffWav_output:: pxRiffWav_output()
{
	_b_header      = false;
	_smp_w         =     0;
	_ch_num        =     0;
	_sps           =     0;
	_bps           =     0;
	_smp_num       =     0;
	_byte_per_smp  =     0;
	_ofs_data_size =     0;
}


bool pxRiffWav_output::write_header( pxDescriptor* desc, int32_t  ch_num, int32_t sps, int32_t bps, int32_t smp_num )
{
	if( !desc ) return false;

	_b_header = false;

	switch( ch_num )
	{
	case  1:
	case  2: break;
	default: return false;
	}

	switch( bps )
	{
	case  8:
	case 16: break;
	default: return false;
	}

	_ch_num  = ch_num ;
	_sps     = sps    ;
	_bps     = bps    ;
	_smp_num = smp_num;

	_byte_per_smp = (bps / 8) * ch_num;

	_RIFFHEADER fmt        = { 0 };
	uint32_t    chunk_size =   0  ;

	fmt.formatID     = 0x0001;
	fmt.ch           = ch_num;
	fmt.sps          = sps   ;
	fmt.byte_per_sec = _byte_per_smp * sps;
	fmt.block_size   = _byte_per_smp;
	fmt.bps          = bps   ; // bit per sample


	if( !desc->w_asfile( _header_code_RIFF, 1, 4 ) ) goto term; //  0
	if( !desc->w_asfile( &chunk_size      , 4, 1 ) ) goto term; // provisional 4
	if( !desc->w_asfile( _header_code_WAVE, 1, 4 ) ) goto term; //  8
	if( !desc->w_asfile( _header_code_fmt , 1, 4 ) ) goto term; // 12

	chunk_size = sizeof(fmt);

	if( !desc->w_asfile( &chunk_size      , 4, 1 ) ) goto term; // 16
	if( !desc->w_asfile( &fmt, sizeof(fmt),    1 ) ) goto term; // 16+sizeof(fmt)
	if( !desc->w_asfile( _header_code_data, 1, 4 ) ) goto term; // 20+sizeof(fmt)

	if( !desc->get_size_bytes( &_ofs_data_size )   ) goto term;

	chunk_size = _byte_per_smp * smp_num;

	if( !desc->w_asfile( &chunk_size,       4, 1 ) ) goto term; // 24+sizeof(fmt)

	_b_header = true;
term:

	return _b_header;
}

bool pxRiffWav_output::write_sample( pxDescriptor* desc, const void *p_src, int32_t smp_num )
{
	if( !desc || !p_src || !_b_header ) return false;
	if( !desc->w_asfile( p_src, _byte_per_smp, smp_num ) ) return false;
	_smp_w += smp_num;
	return true;
}

bool pxRiffWav_output::terminate( pxDescriptor* desc )
{
	if( !desc || !_b_header ) return false;

	int32_t size  =     0;

	if( !desc->get_size_bytes( &size )            ) return false;
	size -= 8;
	if( !desc->seek( pxSEEK_set,              4 ) ) return false;
	if( !desc->w_asfile( &size, sizeof(size), 1 ) ) return false;
	if( !desc->seek( pxSEEK_set, _ofs_data_size ) ) return false;
	size = _smp_w * _byte_per_smp;
	if( !desc->w_asfile( &size, sizeof(size), 1 ) ) return false;
	_b_header = false;
	return true;
}
