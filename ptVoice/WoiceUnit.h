// '17/01/09 VoiceUnit クラス化.
// '17/01/14 WoiceUnit に変更。

#ifndef WoiceUnit_H
#define WoiceUnit_H

#include <pxStdDef.h>

#include <pxtnWoice.h>
#include <pxtonewinXA2.h>

#define WOICEUNIT_ALTE_wave_voice  0x0001
#define WOICEUNIT_ALTE_wave_screen 0x0002
#define WOICEUNIT_ALTE_WAVE (WOICEUNIT_ALTE_wave_voice|WOICEUNIT_ALTE_wave_screen)

#define WOICEUNIT_ALTE_enve_voice  0x0004
#define WOICEUNIT_ALTE_enve_screen 0x0008
#define WOICEUNIT_ALTE_ENVE (WOICEUNIT_ALTE_enve_voice|WOICEUNIT_ALTE_enve_screen)

#define WOICEUNIT_ALTE_channel     0x0010
#define WOICEUNIT_ALTE_etc         0x0020

#define WOICEUNIT_ALTE_wave_mini0  0x0100
#define WOICEUNIT_ALTE_wave_mini1  0x0200
#define WOICEUNIT_ALTE_enve_mini0  0x1000
#define WOICEUNIT_ALTE_enve_mini1  0x2000

#define WOICEUNIT_ALTE_all         0xffff

enum WOICEUNIT_MODE
{
	WOICEUNIT_MODE_rack = 0,
	WOICEUNIT_MODE_wave    ,
	WOICEUNIT_MODE_envelope,
	WOICEUNIT_MODE_num,
};

class WoiceUnit
{
private:

bool               _b_init ;
uint32_t           _frame_alte_flags;
uint32_t           _frame_post_flags;
pxtnWoice*         _woice  ;
pxtonewinXA2*      _xa2    ;


int32_t            _channel;
WOICEUNIT_MODE     _mode   ;
float              _zoom   ; // x1, x0.5, x0.1

bool               _b_sine_over;
bool               _mutes[ pxtnMAX_UNITCONTROLVOICE ];

void _envelope_default( pxtnVOICEENVELOPE* pe );

public:

WoiceUnit();
~WoiceUnit();

bool    init             ( pxtonewinXA2* xa2 );
bool    IDM_INITIALIZE_  ();
void    data_reset       ();

void	reset_overtone   ( pxtnVOICEWAVE *p_wave, int32_t index );

void    default_coodinate( pxtnVOICEWAVE* p_wave, bool b_zero );
void    default_overtone ( pxtnVOICEWAVE* p_wave, bool b_zero );

bool    copy_ptv_voice   ( int32_t src_idx, int32_t dst_idx );
bool    copy_ptv_woice_to( WoiceUnit* p_dst                 ) const;

bool    channel_set   ( int32_t ch );
int32_t channel_get   () const;
void    channel_switch();

bool    zoom_set      ( float zoom );
float   zoom_get      () const;

pxtnVOICEUNIT* get_voice( int32_t index );
pxtnVOICEUNIT* get_voice();
pxtnWoice*     get_woice();

void     frame_alte_set  ( uint32_t alte_flags, int32_t channel );
void     frame_alte_clear();
uint32_t frame_alte_get  () const;

WOICEUNIT_MODE mode_get(                     ) const;
void           mode_set( WOICEUNIT_MODE mode );

void     mute_switch( int32_t channel );
bool     mute_get   ( int32_t channel ) const;

bool     strm_ready_sample  ();
bool     strm_ready_envelope();
bool     strm_ready         ();

int32_t  strm_ON     ( float freq_rate );
int32_t  strm_ON     ( float freq_rate, unsigned char velo );
void     strm_OFF    ( int32_t id, bool bForce );
void     strm_Off_All();
void     strm_CHANGE ( int32_t id, float freq_rate );

bool     csv_attach( const TCHAR* path_csv );
bool     csv_output( const TCHAR* path_csv ) const;
};

#endif