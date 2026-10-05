// '16/01/19 pxOggVorbis.cpp
// '16/12/02 ifdef pxINCLUDE_OGGVORBIS.

#include <pxStdDef.h>

#ifdef pxINCLUDE_OGGVORBIS
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#endif

#include "./pxMem.h"
#include "./pxOggVorbis.h"

pxOggVorbis::pxOggVorbis()
{
	_b_init       = false;
	_b_header     = false;

	_vb_file      = NULL ;
	_vb_read_buf  = NULL ;
	_smp_r        =     0;
	_ov_cur       =     0;

	_byte_per_smp =     0;
	_buf_size     =     0;

	_ch_num       =     0;
	_sps          =     0;
	_bps          =     0;
	_smp_num      =     0;
}

void pxOggVorbis::_release()
{
	_b_init   = false;
	_b_header = false;
	vb_close();
	pxMem_free( (void**)&_vb_file     );
	pxMem_free( (void**)&_vb_read_buf );
}


bool pxOggVorbis::init()
{
	if( _b_init ) return false;

#ifdef pxINCLUDE_OGGVORBIS
	if( !pxMem_zero_alloc(         &_vb_file    , sizeof(OggVorbis_File) ) ) goto term;
	if( !pxMem_zero_alloc( (void**)&_vb_read_buf, _BUFSIZE_VB_READ       ) ) goto term;
#else
	goto term;
#endif

	_b_init = true;
term:
	if( !_b_init ) _release();
	return _b_init;
}

pxOggVorbis::~pxOggVorbis()
{
	_release();
}

bool pxOggVorbis::read_header( pxDescriptor* desc, int32_t *p_ch_num, int32_t *p_sps, int32_t *p_bps, int32_t *p_smp_num )
{
	if( !_b_init ) return false;

	_b_header = false;

#ifdef pxINCLUDE_OGGVORBIS
	if( ov_clear( (OggVorbis_File*)_vb_file ) ) goto term;
	if( ov_open( (FILE*)desc->get_file_r(),   (OggVorbis_File*)_vb_file, NULL, 0 ) < 0 ) goto term;
	{
		vorbis_info *p_info = ov_info         ( (OggVorbis_File*)_vb_file, -1 );
		if( !(_smp_num = (int32_t)ov_pcm_total( (OggVorbis_File*)_vb_file, -1 ) ) ) goto term;
		_ch_num = p_info->channels;
		_sps    = p_info->rate    ;
	}
#else
	goto term;
#endif

	_bps          = 16;
	_byte_per_smp = _bps / 8 * _ch_num;
	_buf_size     = _byte_per_smp *_smp_num;

	_smp_r        =        0;
	_ov_cur       =        0;

	*p_ch_num     = _ch_num ;
	*p_sps        = _sps    ;
	*p_bps        = _bps    ;
	*p_smp_num    = _smp_num;

	_b_header  = true    ;
term:
	if( !_b_header ) vb_close();

	return _b_header;
}

int32_t pxOggVorbis::read_sample( pxDescriptor* desc, void *p_dst, int32_t smp_num )
{
	if( !_b_init   ) return 0;
	if( !_b_header ) return 0;

	int read_size = 0;
	int smp_total = 0;

	uint8_t  *p_d = (uint8_t*)p_dst;

#ifdef pxINCLUDE_OGGVORBIS
	do
	{
		OggVorbis_File *ppp = (OggVorbis_File*)_vb_file;
		read_size = ov_read(  (OggVorbis_File*)_vb_file, (char*)_vb_read_buf, _BUFSIZE_VB_READ, 0, 2, 1, &_ov_cur );
		if( read_size > 0 )
		{
			memcpy( p_d, _vb_read_buf, read_size );
			p_d       += read_size;
			_smp_r    += read_size / _byte_per_smp;
			smp_total += read_size / _byte_per_smp;
		}
	}
	while( read_size > 0 && smp_total < smp_num );
#else
	return 0;
#endif

	if( _smp_r >= _smp_num ) vb_close();

	return smp_total;
}

void pxOggVorbis::vb_close()
{
	if( !_b_init ) return;
#ifdef pxINCLUDE_OGGVORBIS
	if( _vb_file ) ov_clear( (OggVorbis_File*)_vb_file );
#endif
}

bool pxOggVorbis::sttc_save ( const TCHAR *path_dst, int32_t ch_num, int32_t sps, int32_t bps, int32_t smp_num, const void* p_src )
{
	return false;
}
