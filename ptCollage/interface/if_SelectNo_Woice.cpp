
#include <pxtnService.h>
extern pxtnService*  g_pxtn    ;

#include <pxtonewinXA2.h>
extern pxtonewinXA2* g_strm_xa2;

#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../../Generic/if_Generic.h"

#include "Interface.h"
#include "SurfaceNo.h"
#include "if_Cursor.h"

typedef struct
{
	bool     bActive;
	float   left, bottom;
	int32_t no;
	int32_t old_no;
}
_SELECTNOSTRUCT;

static _SELECTNOSTRUCT _select = {0};
static int32_t         _activetone_id = 0;

static void _VoiceON( int32_t w )
{
	g_strm_xa2->Voice_order_stop( _activetone_id, true );

	const pxtnWoice     *p_w = g_pxtn->Woice_Get( w ); if( !p_w  ) return;
	const pxtnVOICEUNIT *p_vc = p_w->get_voice( 0 );   if( !p_vc ) return;
	bool                b_loop;

	if( p_vc->voice_flags & PTV_VOICEFLAG_WAVELOOP ) b_loop = true ;
	else                                             b_loop = false;

	{
		pxtnxaVOICE_ORDER odr; memset( &odr, 0, sizeof(odr) );
		odr.p_woice   = p_w   ;
		odr.b_loop    = b_loop;
		odr.freq_rate =      1;
		odr.vol_pan   =     -1;
		odr.time_pan  =     -1;
		for( int i = 0; i < pxtnMAX_UNITCONTROLVOICE; i++ ) odr.velos[ i ] = -1;
		_activetone_id = g_strm_xa2->Voice_order_new( &odr );
	}
}

static void _VoiceOFF()
{
	g_strm_xa2->Voice_order_stop( _activetone_id, false );
}


void if_SelectNo_Woice_Initialize()
{
	_select.bActive = false;
}


// 座標
void if_SelectNo_Woice_Start( float left, float bottom )
{
	if( !g_pxtn->Woice_Num() ) return;
	_select.bActive = true;
	_select.left    = left;
	_select.bottom  = bottom;
	_select.no      = 0;
	_select.old_no  = 0;
	_VoiceON(         0 );
}

unsigned short if_SelectNo_Woice_End( float cur_y )
{
	if( !_select.bActive ) return 0;

	_select.bActive = false;
	_VoiceOFF();

	return (unsigned short)_select.no;
}

void if_SelectNo_Woice_Update( float cur_x, float cur_y )
{
	if( !_select.bActive ) return;

	int32_t enable_num = g_pxtn->Woice_Num();
	int32_t v_num      = (int32_t)_select.bottom / NAMEBOX_HEIGHT;

	_select.no  = (int32_t)( cur_x - _select.left       ) / NAMEBOX_WIDTH  * v_num +
		(int32_t)( _select.bottom - cur_y - 1 ) / NAMEBOX_HEIGHT % v_num;

	if( _select.no <           0 ) _select.no = 0;
	if( _select.no >= enable_num ) _select.no = enable_num - 1;

	if( _select.no != _select.old_no )
	{
		_select.old_no = _select.no;
		_VoiceON( _select.no );
	}
}

void if_SelectNo_Woice_Put()
{
	float xpos, ypos;
	int32_t v_num;
	int32_t enable_num = g_pxtn->Woice_Num();

	fRECT   rcFocus     = {384,224,512,240};
	fRECT   rc_name_box = {384,240,512,256};

	fRECT   rcName = {0,0,WOICENAME_WIDTH,0};
	int32_t name_surf;

	if( !_select.bActive ) return;

	v_num  = (int32_t)_select.bottom / WOICE_HEIGHT;

	for( int32_t w = 0; w < enable_num; w++ )
	{
		ypos = (float)( _select.bottom - ( w % v_num ) * NAMEBOX_HEIGHT - NAMEBOX_HEIGHT );
		xpos = (float)( _select.left   + ( w / v_num ) * NAMEBOX_WIDTH                   );

		g_dxdraw->tex_Put_View( xpos, ypos, &rc_name_box, SURF_FIELDS );

		if( _select.no == w ) g_dxdraw->tex_Put_View( xpos, ypos, &rcFocus,  SURF_FIELDS );

		rcName.t  = (float)( ( w % 10 ) * WOICENAME_HEIGHT     );
		rcName.b  = (float)( rcName.t   + WOICENAME_HEIGHT - 2 );
		name_surf = SURF_WOICENAME + w / 10;
		g_dxdraw->tex_Put_View( xpos + 24, ypos + 2, &rcName, name_surf );
		if_gen_num6           ( xpos +  4, ypos + 4, w, 2 );
	}
}
