
#include <pxwAlteration.h>
extern pxwAlteration* g_alte  ;

#include "../ptVoice.h"

#include "../WoiceUnit.h"
extern WoiceUnit*     g_vunit ;

#include "../CursorKeyCtrl.h"
extern CursorKeyCtrl* g_curkey;


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

bool VoiceFile_Save( const TCHAR *path )
{
	pxtnWoice  slim( _io_read, _io_write, _io_seek, _io_pos );
	bool       b_ret = false;
	pxtnPOINT* p_point;
	int32_t    count;
	int32_t    v, i, i2;

	if( !g_vunit->get_woice()->Copy( &slim ) ) goto End;

	slim.Slim(); if( !slim.get_voice_num() ) goto End;

	for( v = 0; v < slim.get_voice_num(); v++ )
	{
		// 倍音(未使用ポイントを詰める)
		pxtnVOICEUNIT *p_vc = slim.get_voice_variable( v );
		if( p_vc->type == pxtnVOICE_Overtone )
		{
			p_point = p_vc->wave.points;;

			for( i = 0; i < p_vc->wave.num; i++ )
			{
				if( !p_point[ i ].y )
				{
					for( i2 = i + 1; i2 < p_vc->wave.num; i2++ )
					{
						if( p_point[ i2 ].y ) break;
					}
					if( i2 < p_vc->wave.num )
					{
						p_point[ i  ].x = p_point[ i2 ].x;
						p_point[ i  ].y = p_point[ i2 ].y;
						p_point[ i2 ].x = 0;
						p_point[ i2 ].y = 0;
					}
				}
			}
			count = 0;
			for( i = 0; i < p_vc->wave.num; i++ )
			{
				if( p_point[ i ].y ) count++;
			}
			p_vc->wave.num = count;
		}

		// エンベロープ(未使用ポイントを詰める)
		p_point = p_vc->envelope.points;
		for( i = 1; i < p_vc->envelope.head_num; i++ )
		{
			if( !p_point[ i ].x && !p_point[ i ].y )
			{
				for( i2 = i + 1; i2 < p_vc->envelope.head_num; i2++ )
				{
					if( p_point[ i2 ].x || p_point[ i2 ].y ) break;
				}
				if( i2 < p_vc->envelope.head_num )
				{
					p_point[ i  ].x = p_point[ i2 ].x;
					p_point[ i  ].y = p_point[ i2 ].y;
					p_point[ i2 ].x = 0;
					p_point[ i2 ].y = 0;
				}
			}
		}
		count = 1;
		for( i = 1; i < p_vc->envelope.head_num; i++ )
		{
			if( p_point[ i ].x || p_point[ i ].y ) count++;
		}
		i2 = p_vc->envelope.head_num + p_vc->envelope.body_num;
		p_point[ count ].x = p_point[ i2 ].x;
		p_point[ count ].y = p_point[ i2 ].y;
		p_vc->envelope.head_num = count;

		// 無効エンベロープ
		if( count == 1 &&
			p_point[ 0 ].x == 0 && p_point[ 0 ].y == 128 &&
			p_point[ 1 ].x == 1 && p_point[ 1 ].y ==   0 )
		{
			p_vc->data_flags  &= ~PTV_DATAFLAG_ENVELOPE;
		}

	}

	{
		FILE* fp = _tfopen( path, _T("wb") ); if( !fp ) goto End;
		if( !slim.PTV_Write( fp, NULL ) ){ fclose( fp ); goto End; }
		fclose( fp );
	}

	g_alte->off();

	b_ret = true;
End:

	return b_ret;
}


static bool _convert_sps( pxtnVOICEUNIT* p_vc )
{
	pxtnPOINT* points    = p_vc->envelope.points;
	int32_t    point_num = p_vc->envelope.head_num;
	float      rate = (float)ptvDEFAULT_ENVESPS_new / (float)p_vc->envelope.fps;

	if( ptvDEFAULT_ENVESPS_new == p_vc->envelope.fps ) return true ;
	for( int i = 0; i < point_num; i++ )
	{
		points[ i ].x = ( int32_t)( (float)points[ i ].x * rate );
	}
	p_vc->envelope.fps =  ptvDEFAULT_ENVESPS_new;
	return true;
}

bool VoiceFile_Load( const TCHAR *path )
{
	bool           b_ret = false;
	FILE*          fp    = NULL;
	pxtnWoice*     p_dst = g_vunit->get_woice();
	pxtnWoice*     p_src = NULL;

	pxtnWoice      woice_temp( _io_read, _io_write, _io_seek, _io_pos );

	p_src = &woice_temp;

	g_curkey->clear();

	if( !( fp = _tfopen( path, _T("rb") ) ) ) goto End;

	g_vunit->data_reset();

	// load a woice..
	if( p_src->PTV_Read( fp ) != pxtnOK ) goto End;

	if( p_src->get_voice_num() > pxtnMAX_UNITCONTROLVOICE ) goto End; // unknown..

	// copy to work..
	for( int v = 0; v < p_src->get_voice_num(); v++ )
	{
		pxtnVOICEUNIT       *p_d = p_dst->get_voice_variable( v );
		const pxtnVOICEUNIT *p_s = p_src->get_voice         ( v );

		// wave..
		switch( p_s->type )
		{

		case pxtnVOICE_Overtone:
			if( p_s->wave.num > ptvFIXNUM_WAVE_POINT  ) goto End;
			g_vunit->reset_overtone( &p_d->wave, v );
			for( int o = 0; o < p_s->wave.num; o++ )
			{
				int x = p_s->wave.points[ o ].x;
				if( x < 1 || x > ptvFIXNUM_WAVE_POINT ) goto End;
				p_d->wave.points[ x - 1 ].y = p_s->wave.points[ o ].y;
			}
			break;

		case pxtnVOICE_Coodinate:

			if( p_s->wave.num > ptvFIXNUM_WAVE_POINT ) goto End;

			// new resolution
			{
				int32_t new_reso = ptvCOODINATERESOLUTION;
				int32_t src_reso = p_s->wave.reso        ;

				for( int o = 0; o < p_s->wave.num; o++ )
				{
					int x = p_s->wave.points[ o ].x;
					x = x * new_reso / src_reso;
					if( x < 0 || x >= new_reso ) goto End;
					p_d->wave.points[ o ].x = x;
					p_d->wave.points[ o ].y = p_s->wave.points[ o ].y;
				}
				p_d->wave.num  = p_s->wave.num;
				p_d->wave.reso = new_reso     ;
			}
			break;

		default: goto End;
		}

		p_d->tuning      = p_s->tuning     ;
		p_d->voice_flags = p_s->voice_flags;
		p_d->basic_key   = p_s->basic_key  ;
		p_d->pan         = p_s->pan        ;
		p_d->type        = p_s->type       ;
		p_d->volume      = p_s->volume     ;

		// envelope..
		if( p_s->data_flags & PTV_DATAFLAG_ENVELOPE )
		{
			switch( p_s->envelope.fps )
			{
			case ptvDEFAULT_ENVESPS_new:
			case ptvDEFAULT_ENVESPS_old: break;
			default: goto End; // unknown..
			}

			if( p_s->envelope.head_num > ptvMAX_ENVELOPEPOINT  ) goto End; // unknown..
			if( p_s->envelope.body_num                         ) goto End; // unknown..
			if( p_s->envelope.tail_num != pxtnMAX_RELEASEPOINT ) goto End; // unknown..

			p_d->envelope.fps = p_s->envelope.fps;
			int e;
			for( e = 0; e < p_s->envelope.head_num; e++ )
			{
				p_d->envelope.points[ e ].x = p_s->envelope.points[ e ].x;
				p_d->envelope.points[ e ].y = p_s->envelope.points[ e ].y;
			}
			// release point.
			p_d->envelope.points[ ptvMAX_ENVELOPEPOINT ].x = p_s->envelope.points[ e ].x;
			p_d->envelope.points[ ptvMAX_ENVELOPEPOINT ].y = p_s->envelope.points[ e ].y;
		}

		_convert_sps( p_d );
	}

	g_alte->off();

	b_ret = true;
End:
	if( fp ) fclose( fp );

	if( !b_ret )
	{
		g_vunit->data_reset();
	}
	return b_ret;
}
