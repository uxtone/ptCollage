
#include "NoiseTable.h"


void NoiseTable_CopyUnit( pxNOISEDESIGN_UNIT *p_dst, pxNOISEDESIGN_UNIT *p_src )
{
	int32_t e;
	p_dst->bEnable  = p_src->bEnable ;
	p_dst->pan      = p_src->pan     ;
	p_dst->main     = p_src->main    ;
	p_dst->freq     = p_src->freq    ;
	p_dst->volu     = p_src->volu    ;
	p_dst->enve_num = p_src->enve_num;
	for( e = 0; e < p_src->enve_num; e++ ) p_dst->enves[ e ] = p_src->enves[ e ];
}


void NoiseTable_SetDefaultQuality( SAMPLINGQUALITY *p_quality )
{
	p_quality->ch  =     2;
	p_quality->sps = 44100;
	p_quality->bps =    16;
}

void NoiseTable_SetDefault( pxtnPulse_Noise *p_ptn )
{
	pxNOISEDESIGN_UNIT *p_unit;

	p_ptn->set_smp_num_44k( 44100 );

	int unit_num = p_ptn->get_unit_num();

	for( int u = 0; u < unit_num; u++ )
	{
		p_unit = p_ptn->get_unit( u );
		p_unit->bEnable = u ? false : true;

		{
			p_unit->enves[ 0 ].x  =    0;
			p_unit->enves[ 0 ].y  =  100;
		}
		if( p_unit->enve_num > 1 )
		{
			p_unit->enves[ 1 ].x  = 1000;
			p_unit->enves[ 1 ].y  =    0;
		}

		p_unit->main.type   = pxWAVETYPE_Sine;
		p_unit->main.freq   = 440;
		p_unit->main.volume = 50;
		p_unit->main.offset = 0;
		p_unit->main.b_rev  = false;

		p_unit->freq.type   = pxWAVETYPE_None;
		p_unit->freq.freq   = 1;
		p_unit->freq.volume = 0;
		p_unit->freq.offset = 0;
		p_unit->freq.b_rev  = false;

		p_unit->volu.type   = pxWAVETYPE_None;
		p_unit->volu.freq   = 1;
		p_unit->volu.volume = 100;
		p_unit->volu.offset = 0;
		p_unit->volu.b_rev  = false;
	}
}