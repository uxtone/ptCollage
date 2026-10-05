// '16/06/24 pxtoneStreamXA2.
// '16/07/05 pxtoneStreamXA2_tune/voice.cpp
// '16/10/17 -> pxtonwinXA2.

#ifndef pxtonewinXA2_H
#define pxtonewinXA2_H

#include <pxwXAudio2.h>
#include <pxtnService.h>
#include <pxwMutex.h>

typedef struct
{
	bool              b_on  ;
	bool              b_act ;
	bool              b_loop;
	uint32_t          play_id;
	int32_t           pan_vols     [ pxtnMAX_CHANNEL ];
	int32_t           pan_times    [ pxtnMAX_CHANNEL ];
	int32_t           pan_time_bufs[ pxtnMAX_CHANNEL ][ pxtnBUFSIZE_TIMEPAN ];
	int32_t           velos   [ pxtnMAX_UNITCONTROLVOICE ];
	float             freq_rate;
	int32_t           min_sample_count;

	int32_t           voice_num;
	pxtnVOICEINSTANCE voinsts [ pxtnMAX_UNITCONTROLVOICE ];
	pxtnVOICETONE     voitones[ pxtnMAX_UNITCONTROLVOICE ];
}
STREAMINGVOICETONE2;


enum pxtnxaVOICE_ORDER_TYPE
{
	pxtnxaVOICE_ORDER_none = 0,
	pxtnxaVOICE_ORDER_new     ,
	pxtnxaVOICE_ORDER_freq    ,
	pxtnxaVOICE_ORDER_stop    ,
	pxtnxaVOICE_ORDER_mstr_vol,
	pxtnxaVOICE_ORDER_stop_all,
};

typedef struct
{
	pxtnxaVOICE_ORDER_TYPE type;

	const pxtnWoice* p_woice  ;
	bool             b_loop   ;
	float            freq_rate;
	int32_t          vol_pan  ;

	int32_t          velos[ pxtnMAX_UNITCONTROLVOICE ];

	int32_t          time_pan ;
	uint32_t         play_id  ;

	bool             b_force_stop;
	float            mstr_vol    ;
}
pxtnxaVOICE_ORDER;

#define pxtnxaTUNE_ORDERFLAG_prep      0x001
#define pxtnxaTUNE_ORDERFLAG_volume    0x002
#define pxtnxaTUNE_ORDERFLAG_stop      0x004
#define pxtnxaTUNE_ORDERFLAG_loop      0x010
#define pxtnxaTUNE_ORDERFLAG_unit_mute 0x020

typedef struct
{
	uint32_t             flags      ;

	pxtnVOMITPREPARATION prep       ;
	float                volume     ;
	float                fadeout_sec;
	bool                 b_loop     ;
	bool                 b_unit_mute;
}
pxtnxaTUNE_ORDER;

class pxtonewinXA2
{
private:
void operator = (const pxtonewinXA2& src){}
pxtonewinXA2    (const pxtonewinXA2& src){}

bool                 _b_init        ;

pxwXAudio2*          _audio         ;
pxtnService*         _pxtn          ;

int32_t              _safe_ch_num   ;
int32_t              _safe_sps      ;
float                _safe_buf_sec  ;
int32_t              _safe_smp_count;

int32_t              _dst_ch_num    ;
int32_t              _dst_sps       ;
float                _dst_buf_sec   ;

int32_t              _thrd_priority ;

    // Voice..
float                _vc_master_vol  ;


pxwMutex*            _vc_mtx         ;
STREAMINGVOICETONE2* _vc_tones       ;
int32_t              _vc_max_tone    ;
int32_t              _vc_play_id_idx ;
int32_t              _vc_top         ;
int32_t              _vc_smooth      ;
double               _vc_sample_skip ;
int32_t              _vc_time_pan_idx;

pxtnPulse_Frequency* _vc_freq        ;

pxtnxaVOICE_ORDER*   _vc_odrs        ;
int32_t                  _vc_odr_num     ;

pxwMutex*            _tn_mtx         ;
pxtnxaTUNE_ORDER     _tn_odr         ;
bool                 _tn_b_sampling  ;

bool _Voice_Init     ( int32_t max_tone, int32_t max_order );
void _Voice_Release  ();

void _Voice__Envelope( pxtnVOICEINSTANCE* p_vi, pxtnVOICETONE* p_vt, bool bON );
void _Voice_Envelope ();

void _Voice_ToNextSample();
void _Voice_MakeSample( void* p_buf );
bool _Voice_SetConfig();

bool _voice_set_new( const pxtnxaVOICE_ORDER* po );


bool _release();

void _tune_sampling ( LPVOID p1, int32_t size1 );
void _voice_sampling( LPVOID p1, int32_t size1 );

static bool _sampling_proc_static( void *user, void *buf, int *p_res_size, int *p_req_size );
bool        _sampling_proc       (             void *buf, int *p_res_size, int *p_req_size );

public:
pxtonewinXA2();
~pxtonewinXA2();

bool init( pxtnService* pxtn, int32_t max_voice, int32_t max_vc_odr );

bool stream_start           ( int32_t    ch_num, int32_t    sps, float    buf_sec );
bool stream_get_quality_safe( int32_t* p_ch_num, int32_t* p_sps, float* p_buf_sec );
bool stream_order_stop();
bool stream_is_working();
bool stream_join      ( float timeout_sec, bool* p_b_success );
bool stream_finalize  ( float interval, int retry, float timeout_sec );

bool set_quality( int32_t ch_num, int32_t sps, int32_t bps, float buf_sec );

uint32_t  Voice_order_new( const pxtnxaVOICE_ORDER* odr );
        //( const pxtnWoice *p_w , bool b_loop, float freq_rate, int32_t vol_pan, int32_t velo, int32_t time_pan );

bool Voice_order_freq        ( uint32_t play_id,                        float freq_rate                                      );
bool Voice_order_stop        ( int32_t play_id, bool b_force );
bool Voice_order_MasterVolume( float vol );
bool Voice_order_stop_all    ();

bool tune_order_start        ( const pxtnVOMITPREPARATION *p_prep );
bool tune_order_volume       ( float vol );
bool tune_order_stop         ( float fadeout_sec );
bool tune_order_loop         ( bool  b   );
bool tune_order_unit_mute    ( bool  b   );
bool tune_is_sampling        (           );

int32_t tune_get_sampling_count_safe() const;
};

#endif
