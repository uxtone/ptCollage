
#include <pxStdDef.h>
#include <pxtnPulse_PCM.h>
#include "./NoiseTable.h"

bool PcmTable_Initialize();
void PcmTable_Release   ();
bool PcmTable_Wave_Load ( const uxDS& path );
bool PcmTable_Wave_Save ( const uxDS& path );

bool           PcmTable_Woice_Update();
void           PcmTable_Woice_Play  ( bool b_loop );
void           PcmTable_Woice_Stop  ( bool bForce );

pxtnPulse_PCM* PcmTable_Woice_GetPcmPointer();

bool           PcmTable_Woice_SetScoped( int start, int end );

void           PcmTable_Woice_PCM_ConvertVolume( float v );

void           PcmTable_Scoped_Play();
void           PcmTable_Scoped_Stop( bool bForce );

bool           PcmTable_BuildAndPlay( pxtnPulse_Noise *p_noise, const SAMPLINGQUALITY *p_quality );
