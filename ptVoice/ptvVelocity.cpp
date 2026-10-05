
#include <pxStdDef.h>

#include <pxFile2.h>

#define _VELOCITY_MAX     128
#define _VELOCITY_DEFAULT 104

static int32_t        _velocity_volume  = _VELOCITY_DEFAULT;
static uxSS<13>          _file_name        = "ptv-velo.bin";
static const pxFile2* _ref_file_profile = NULL;

void ptvVelocity_init( const pxFile2* file_profile )
{
	_ref_file_profile = file_profile;
}

bool ptvVelocity_load()
{
	bool          b_ret = false;
	pxDescriptor* desc  = NULL ;

	if( !_ref_file_profile->open_r( &desc, _file_name, NULL, NULL ) ) goto term;
	if( !desc->r( &_velocity_volume, sizeof(_velocity_volume), 1 ) ) goto term;

	b_ret = true;
term:
	SAFE_DELETE( desc );

	if( !b_ret ) _velocity_volume = _VELOCITY_DEFAULT;

	return b_ret;
}

bool ptvVelocity_save()
{
	bool          b_ret = false;
	pxDescriptor* desc  = NULL ;

	if( !_ref_file_profile->open_w( &desc, _file_name, NULL, NULL ) ) goto term;
	if( !desc->w_asfile( &_velocity_volume, sizeof(_velocity_volume), 1 ) ) goto term;

	b_ret = true;
term:
	SAFE_DELETE( desc );

	return b_ret;
}

// 取得
int32_t ptvVelocity_Get()
{
	return _velocity_volume;
}

// 代入
void ptvVelocity_Set( int32_t volume )
{
	if( volume < 0   ) volume =   0;
	if( volume > 128 ) volume = 128;
	_velocity_volume = volume;
}
