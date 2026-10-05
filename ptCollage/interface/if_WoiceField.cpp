
#include <pxtnPulse_Oggv.h>

#include <pxtnService.h>
extern pxtnService *g_pxtn;


#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../../Generic/if_gen_Scroll.h"
#include "../../Generic/if_Generic.h"

#include "../WoiceFocus.h"

#include "if_ToolPanel.h"
#include "if_VolumePanel.h"
#include "Interface.h"
#include "SurfaceNo.h"

extern if_gen_Scroll  g_ScrlWoiceV;
extern if_gen_Scroll  g_ScrlWoiceH;

static fRECT    _rcField = {0};
static uint32_t _color_field = 0xff000010;

void if_WoiceField_Initialize()
{
}

void if_WoiceField_SetRect( const fRECT *rc_view )
{
	_rcField.l = rc_view->l + WOICE_WIDTH;
	_rcField.r = rc_view->r - SCROLLBUTTONSIZE;
	_rcField.t = rc_view->t + HEADER_H + TOOLPANEL_HEIGHT;
	_rcField.b = rc_view->b - SCROLL_HEIGHT;

	g_ScrlWoiceV.SetRect( &_rcField );
	g_ScrlWoiceH.SetRect( &_rcField );
}

fRECT* if_WoiceField_GetRect()
{
	return &_rcField;
}

// ユニットボディを表示
void if_WoiceField_Put()
{
	float offset;
	int32_t focus;
	int32_t num, w;
	float   x, y;
	int32_t i;

	fRECT rcDummy     = {144, 96,160,112};

	fRECT rcType[]    =
	{
		{  0,320, 32,336},
		{  0,336, 32,352},
		{  0,352, 32,368},
		{  0,368, 32,384},
	};

	fRECT rcChannel[] =
	{
		{120,320,144,336},
		{120,336,144,352},
	};

	fRECT rcBPS[] =
	{
		{ 88,320,120,336},
		{ 88,336,120,352},
	};

	fRECT rcSPS[] =
	{
		{ 32,320, 88,336}, // 11k
		{ 32,336, 88,352}, // 22k
		{ 32,352, 88,368}, // 44k
		{ 32,368, 88,384}, // 48k
		{ 32,384, 88,400}, // 24k
		{ 32,400, 88,416}, // 32k
		{ 32,416, 88,432}, //  8k
		{200,320,208,328}, //   x
	};

	fRECT rcPTV[] =
	{
		{144,320,160,336},
		{144,336,160,352},
	};

	fRECT rcSample = { 88,352,120,368};

	g_dxdraw->FillRect_view( &_rcField, _color_field );

	g_ScrlWoiceV.Put();

	focus     = WoiceFocus_Get(); if( focus < 0 ) return;
	offset    = g_ScrlWoiceV.GetOffset();
	num       = g_pxtn->Woice_Num();
	x         = _rcField.l;

	for( w = 0; w < num; w++ )
	{
		y = _rcField.t + w * WOICE_HEIGHT - offset;
		const pxtnWoice     *p_w = g_pxtn->Woice_Get( w );
		const pxtnVOICEUNIT *p_v = p_w->get_voice( 0 );
		if( p_w )
		{
			switch( p_w->get_type() )
			{
			case pxtnWOICE_PCM :
			{
				int ch       = p_v->p_pcm->get_ch      ();
				int bps      = p_v->p_pcm->get_bps     ();
				int sps      = p_v->p_pcm->get_sps     ();
				int smp_body = p_v->p_pcm->get_smp_body();

				g_dxdraw->tex_Put_Clip( x      , y, &rcType   [      0 ], SURF_FIELDS, &_rcField );
				g_dxdraw->tex_Put_Clip( x +  32, y, &rcChannel[ ch - 1 ], SURF_FIELDS, &_rcField );
				switch( bps )
				{
				case     8: i = 0; break;
				case    16: i = 1; break;
				}
				g_dxdraw->tex_Put_Clip( x +  56, y, &rcBPS    [ i      ], SURF_FIELDS, &_rcField );
				switch( sps )
				{
				case 11025: i = 0; break;
				case 22050: i = 1; break;
				case 44100: i = 2; break;
				case 48000: i = 3; break;
				}
				g_dxdraw->tex_Put_Clip( x +  88, y, &rcSPS    [ i      ], SURF_FIELDS, &_rcField );

				if_gen_num6_clip      ( x + 148, y + 4, p_v->p_pcm->get_smp_body(), 6, &_rcField );
				g_dxdraw->tex_Put_Clip( x + 196, y, &rcSample           , SURF_FIELDS, &_rcField );

				break;
			}

			case pxtnWOICE_PTV :
				g_dxdraw->tex_Put_Clip( x      , y, &rcType[                       1 ], SURF_FIELDS, &_rcField );
				g_dxdraw->tex_Put_Clip( x +  32, y, &rcPTV[ p_w->get_voice_num() - 1 ], SURF_FIELDS, &_rcField );
				break;

			case pxtnWOICE_PTN :
				g_dxdraw->tex_Put_Clip( x      , y, &rcType[                  2 ], SURF_FIELDS, &_rcField );
				if_gen_num6_clip      ( x + 148, y + 4, p_v->p_ptn->get_smp_num_44k(), 6      , &_rcField );
				g_dxdraw->tex_Put_Clip( x + 196, y, &rcSample                    , SURF_FIELDS, &_rcField );
				break;

			case pxtnWOICE_OGGV:
#ifdef pxINCLUDE_OGGVORBIS
			{
				int ch, sps, smp_num;
				p_v->p_oggv->GetInfo( &ch, &sps, &smp_num );
				g_dxdraw->tex_Put_Clip( x      , y, &rcType   [      3 ], SURF_FIELDS, &_rcField );
				g_dxdraw->tex_Put_Clip( x +  32, y, &rcChannel[ ch - 1 ], SURF_FIELDS, &_rcField );
				switch( sps )
				{
				case 11025: i = 0; break;
				case 22050: i = 1; break;
				case 44100: i = 2; break;
				case 48000: i = 3; break;
				case 24000: i = 4; break;
				case 32000: i = 5; break;
				case  8000: i = 6; break;
				default   : i = 7; break;
				}
				g_dxdraw->tex_Put_Clip( x +  88, y, &rcSPS[           i ], SURF_FIELDS, &_rcField );
				if_gen_num6_clip      ( x + 148, y + 4, smp_num, 6                    , &_rcField );
				g_dxdraw->tex_Put_Clip( x + 196, y, &rcSample            , SURF_FIELDS, &_rcField );
			}
#endif
				break;
			}


		}
	}

}