
#include <pxCSV2.h>

#include "./ptVoice.h"

#include "./WoiceUnit.h"

/*
ptvoice
version,0800
channel,0
volume,36
coodinate,5
 0, 0
29, 0,
30,64,
59,64,
60, 0,
envelope,5
10,20
10,40
10,20
10,40
20,20
*/

static const int32_t _version_value = 800;

static const char* _csv_head      = "ptvoice"  ;
static const char* _csv_ver       = "version"  ; // "1002" = v.1.002

static const char* _csv_channel   = "channel"  ;
static const char* _csv_volume    = "volume"   ;
static const char* _csv_pan       = "pan"      ;
static const char* _csv_tuning    = "tuning"   ;
static const char* _csv_coodinate = "coodinate";
static const char* _csv_overtone  = "overtone" ;
static const char* _csv_envelope  = "envelope" ;
static const char* _csv_release   = "release"  ;

bool WoiceUnit::csv_attach( const TCHAR* path_csv )
{
	if( !_b_init ) return false;

	bool           b_ret    = false;
	bool           b_update = false;

	FILE*          fp       = NULL ;
	int32_t        row      =     0;
	int32_t        r        =     0;
	const char*    pv       = NULL ;
	pxtnVOICEUNIT* p_vc     = NULL ;
	int32_t        channel  =     0;

	pxDescriptor   desc;
	pxCSV2         csv ;

	if( !( fp = _tfopen( path_csv, _T("rb") ) ) ) goto term;
	if( !desc.set_file_r( fp )     ) goto term;
	if( !csv.read( &desc, true )   ) goto term;
	if( !csv.get_row_count( &row ) ) goto term;

	if( !csv.get_value( &pv, 0, 0 ) || strcmp( pv, _csv_head  ) ) goto term;
	r++;

	while( csv.get_value( &pv, r, 0 ) )
	{
		// channel.
		if     ( !strcmp( pv, _csv_channel  ) )
		{
			if( !csv.get_value( &pv, r, 1 ) ) goto term;
			channel = atoi( pv );
			if( channel < 0 || channel >= pxtnMAX_UNITCONTROLVOICE ) goto term;
			p_vc = _woice->get_voice_variable( channel );
			r++;
		}
		// volume.
		else if( !strcmp( pv, _csv_volume   ) )
		{
			if( !p_vc ) goto term;
			if( !csv.get_value( &pv, r, 1 ) ) goto term;
			p_vc->volume = atoi( pv );
			b_update = true;
			r++;
		}
		// pan
		else if( !strcmp( pv, _csv_pan   ) )
		{
			if( !p_vc ) goto term;
			if( !csv.get_value( &pv, r, 1 ) ) goto term;
			p_vc->pan = atoi( pv );
			b_update = true;
			r++;
		}
		// tuning
		else if( !strcmp( pv, _csv_tuning   ) )
		{
			if( !p_vc ) goto term;
			if( !csv.get_value( &pv, r, 1 ) ) goto term;
			p_vc->tuning = (float)atof( pv );
			b_update = true;
			r++;
		}

		// envelope.
		else if( !strcmp( pv, _csv_envelope ) )
		{
			if( !p_vc ) goto term;
			_envelope_default( &p_vc->envelope ); b_update = true;

			int32_t enve_num = 0;
			int32_t e        = 0;
			int32_t x        = 0;
			int32_t v        = 0;
			if( !csv.get_value( &pv, r, 1 ) ) goto term;
			{
				enve_num = atoi( pv );
				if( enve_num < 0 || enve_num > ptvMAX_ENVELOPEPOINT ) return false;
				r++;
				for( int i = 0; i < enve_num; i++ )
				{
					if( !csv.get_value( &pv, r, 0 ) ) goto term; x = atoi( pv );
					if( !csv.get_value( &pv, r, 1 ) ) goto term; v = atoi( pv );
					if( x < 0 || v < 0 || v > 128   ) goto term;
					if( e == 0 || x || v )
					{
						p_vc->envelope.points[ e ].x = x;
						p_vc->envelope.points[ e ].y = v;
					}
					e++; r++;
				}
			}
		}
		// coodinate.
		else if( !strcmp( pv, _csv_coodinate ) )
		{
			if( !p_vc ) goto term;

			p_vc->type      = pxtnVOICE_Coodinate;

			// clear.
			p_vc->wave.num  = 1;
			p_vc->wave.reso = ptvCOODINATERESOLUTION;
			memset( p_vc->wave.points, 0, sizeof(POINT) * ptvFIXNUM_WAVE_POINT );
			b_update = true;


			int32_t pos_num = 0;
			int32_t e       = 0;
			int32_t x       = 0;
			int32_t v       = 0;

			if( !csv.get_value( &pv, r, 1 ) ) goto term;
			{
				pos_num = atoi( pv );
				if( pos_num < 0 || pos_num >= ptvFIXNUM_WAVE_POINT ) return false;
				r++;
				for( int i = 0; i < pos_num; i++ )
				{
					if( !csv.get_value( &pv, r, 0 )  ) goto term; x = atoi( pv );
					if( !csv.get_value( &pv, r, 1 )  ) goto term; v = atoi( pv );
					if( x < 0 || x >= ptvCOODINATERESOLUTION || v < -126 || v > 127 ) goto term;
					if( e && x == 0                  ) goto term;
					p_vc->wave.points[ e ].x = x;
					p_vc->wave.points[ e ].y = v;
					e++; r++;
				}
				p_vc->wave.num = pos_num;
			}
		}
		else
		{
			r++;
		}
	}

	b_ret = true;
term:
	if( fp ) fclose( fp );
	return b_ret;
}

bool _csv_output( FILE* fp, int32_t channel, const pxtnVOICEUNIT* pv )
{
	int32_t count = 0;

	// channel
	fprintf( fp, "%s,%d\n", _csv_channel, channel    );

	// volume
	fprintf( fp, "%s,%d\n", _csv_volume , pv->volume );

	// pan
	fprintf( fp, "%s,%d\n", _csv_pan    , pv->pan    );

	// tuning
	fprintf( fp, "%s,%f\n", _csv_tuning , pv->tuning );

	// coodinate / overtone-----
	switch( pv->type )
	{
	case pxtnVOICE_Coodinate:
		fprintf( fp, "%s,%d\n", _csv_coodinate, pv->wave.num );
		for( int c = 0; c < pv->wave.num; c++ ) fprintf( fp, "%4d,%4d\n", pv->wave.points[ c ].x, pv->wave.points[ c ].y );
		break;

	case pxtnVOICE_Overtone :
		fprintf( fp, "%s,%d\n", _csv_overtone, pv->wave.num );
		for( int c = 0; c < pv->wave.num; c++ ) fprintf( fp, "%4d,%4d\n", pv->wave.points[ c ].x, pv->wave.points[ c ].y );
		break;
	}

	// envelope  -----
	count = 0;
	for( int e = 0; e < pv->envelope.head_num; e++ )
	{
		if( pv->envelope.points[ e ].x || pv->envelope.points[ e ].y ) count++;
	}

	fprintf( fp, "%s,%d\n", _csv_envelope, count );
	for( int e = 0; e < pv->envelope.head_num; e++ )
	{
		if( pv->envelope.points[ e ].x || pv->envelope.points[ e ].y ) fprintf( fp, "%4d,%4d\n", pv->envelope.points[ e ].x, pv->envelope.points[ e ].y );
	}
	return true;
}

bool WoiceUnit::csv_output( const TCHAR* path_csv ) const
{
	bool    b_ret = false;
	FILE*   fp    = NULL ;
	int32_t count =     0;

	const pxtnVOICEUNIT* pv = NULL;

	if( !( fp = _tfopen( path_csv, _T("wt") ) ) ) goto term;

	// head.
	fprintf( fp, "%s\n", _csv_head );

	// version.
	fprintf( fp, "%s,%04d\n", _csv_ver, _version_value );

	for( int v = 0; v < pxtnMAX_UNITCONTROLVOICE; v++ )
	{
		pv    = _woice->get_voice( v );
		count = 0;
		if( pv && pv->volume > 0 )
		{
			for( int i = 0; i < pv->wave.num; i++ )
			{
				if( pv->wave.points[ i ].y ) count++;
			}
		}
		if( count ) _csv_output( fp, v, pv );
	}

	b_ret = true;
term:
	if( fp ) fclose( fp );

	return b_ret;
}

