
#ifdef _WIN32
#else
#include <stdio.h>
#endif
#include <uxStr.h>

#include <pxwrAppEtc.h>

#include "./pxStrT.h"
#include "./pxMem.h"

#include "./pxLocalize.h"

typedef struct
{
	const char*  _region_name;
	const uxDS  _dir_sub    ;
}
_REGION_LANGUAGE;

#define _DIR_REGION_SIZE 64

static const _REGION_LANGUAGE _region_tbl[ pxLOCALREGION_num ] =
{
	{"ENGLISH"  , "en"}, // pxLOCALREGION_en
	{"JAPANESE" , "ja"}, // pxLOCALREGION_ja
	{"CHINESE"  , "zh-Hans"}, // pxLOCALREGION_cn
	{"FRENCH"   , "fr"}, // pxLOCALREGION_fr
	{"ITALIAN"  , "it"}, // pxLOCALREGION_it
	{"GERMAN"   , "de"}, // pxLOCALREGION_de
	{"SPANISH"  , "es"}, // pxLOCALREGION_es
	{"PORTUGUES", "pt"}, // pxLOCALREGION_po
	{"RUSSIAN"  , "ru"}, // pxLOCALREGION_ru
};


void pxLocalize::_release()
{
	_b_init = false;
	pxStrT_free( _dir_localize );
	pxStrT_free( _dir_region   );
}

void pxLocalize::_update_dir_region()
{
	if( !_b_init ) return;
	uxSS<pxBUFSIZE_PATH> dir = {};
	ux_sprintf_s( dir, "%s/%s.lproj", *_dir_localize, *_region_tbl[ _region ]._dir_sub );
	pxStrT_free         ( _dir_region      );
	pxStrT_copy_allocate( _dir_region, dir );
}

pxLocalize::pxLocalize()
{
	_b_init       = false            ;
	_region       = pxLOCALREGION_num;
	_dir_localize = NULL             ;
	_dir_region   = NULL             ;
}

pxLocalize::~pxLocalize()
{
	_release();
}

bool pxLocalize::init( const uxDS& dir_localize )
{
	if( _b_init || !dir_localize ) return false;

	if( !pxStrT_copy_allocate( _dir_localize    , dir_localize     ) ) goto term;

//	if( !pxMem_zero_alloc( (void**)&_dir_region, _DIR_REGION_SIZE * sizeof(char) ) ) goto term;

	switch( pxwrAppEtc_Local() )
	{
	case  0: _region = pxLOCALREGION_en ; break;
	case  1: _region = pxLOCALREGION_ja ; break;
	case  2: _region = pxLOCALREGION_cn ; break;
	case  3: _region = pxLOCALREGION_fr ; break;
	case  4: _region = pxLOCALREGION_it ; break;
	case  5: _region = pxLOCALREGION_de ; break;
	case  6: _region = pxLOCALREGION_es ; break;
	case  7: _region = pxLOCALREGION_pt ; break;
	case  8: _region = pxLOCALREGION_ru ; break;
	default: _region = pxLOCALREGION_num; break;
	}
	_b_init = true;
	_update_dir_region();
term:
	if( !_b_init ) _release();
	return _b_init;
}

bool pxLocalize::read( pxDescriptor* desc )
{
	if( !desc ) return false;

	bool                    b_ret             = false;
	const _REGION_LANGUAGE* p_rgn             = NULL ;
	char                    region_name[ 32 ] = {   };

	int32_t len = 0;
	if( !desc->r( &len       , sizeof(len),   1 ) ) goto term;
	if( !desc->r( region_name,           1, len ) ) goto term;

	for( int i = 0; i < pxLOCALREGION_num; i++ )
	{
		if( !strcmp( _region_tbl[ i ]._region_name, region_name ) )
		{
			_region = (pxLOCALREGION)i  ;
			p_rgn   = &_region_tbl[  i ];
			break;
		}
	}
	if( !p_rgn ) goto term;

	_update_dir_region();

	b_ret = true;
term:

	return b_ret;
}

bool pxLocalize::set( pxLOCALREGION region )
{
	if( !_b_init ) return false;
	if( region < 0 || region >= pxLOCALREGION_num ) return false;

	bool   b_ret = false;
	_region = region;
	_update_dir_region();

	return true;
}

bool pxLocalize::set_and_write( pxLOCALREGION region, pxDescriptor* desc )
{
	if( !_b_init || !desc ) return false;
	if( !set( region )    ) return false;

	bool  b_ret = false;
	const _REGION_LANGUAGE* p_rgn = &_region_tbl[ _region ];

	int32_t len = strlen( p_rgn->_region_name );
	if( !desc->w_asfile( &len               , sizeof(len),   1 ) ) goto term;
	if( !desc->w_asfile( p_rgn->_region_name,           1, len ) ) goto term;

	b_ret = true;
term:
	return b_ret;
}

pxLOCALREGION pxLocalize::get() const{ return _region; }

const uxDS  pxLocalize::get_region_dir() const
{
	if( !_b_init ) return NULL;
	return _dir_region;
}
