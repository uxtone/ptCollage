
#include <pxtnService.h>
extern pxtnService *g_pxtn;


#include <pxwDx09Draw.h>
extern pxwDx09Draw *g_dxdraw;

#include "../../Generic/if_Generic.h"
#include "../../Generic/if_gen_Scroll.h"

#include "../WoiceFocus.h"

#include "SurfaceNo.h"
#include "Interface.h"
#include "if_ToolPanel.h"
#include "if_VolumePanel.h"
#include "if_Player.h"

extern if_gen_Scroll g_ScrlWoiceV;

static fRECT    _rcField = {0};
static fRECT    _rc_view = {0};

static uint32_t _color_pcm = 0xfff0f0f0;
static uint32_t _color_ptv = 0xffffff80;
static uint32_t _color_ptn = 0xffd0d0ff;
static uint32_t _color_ogg = 0xffffdd66;

bool if_WoiceTray_init( const pxPalette* pal )
{
	if( !pal->get_color( COLORINDEX_font_pcm, &_color_pcm ) ) return false;
	if( !pal->get_color( COLORINDEX_font_ptv, &_color_ptv ) ) return false;
	if( !pal->get_color( COLORINDEX_font_ptn, &_color_ptn ) ) return false;
	if( !pal->get_color( COLORINDEX_font_ogg, &_color_ogg ) ) return false;
	return true;
}

void if_WoiceTray_SetRect( const fRECT *rc_view )
{
	_rc_view   = *rc_view;

	_rcField.l = rc_view->l;
	_rcField.r = rc_view->l + NAMETRAY_WIDTH;
	_rcField.t = rc_view->t + HEADER_H + TOOLPANEL_HEIGHT;

	if( if_ToolPanel_GetMode() == enum_ToolMode_W ) _rcField.b = rc_view->b - SCROLL_HEIGHT;
	else                                            _rcField.b = _rcField.t;

	g_ScrlWoiceV.SetSize( WOICE_HEIGHT * g_pxtn->Woice_Num() );
}

fRECT* if_WoiceTray_GetRect()
{
	return &_rcField;
}

void if_WoiceTray_RedrawName( int32_t w )
{
	fRECT            rc   = { 3, 0, WOICENAME_WIDTH, WOICENAME_HEIGHT };
	int32_t          surface;
	int32_t          y;
	const pxtnWoice* p_w = g_pxtn->Woice_Get( w ); if( !p_w ) return;
	uint32_t color;

	switch( p_w->get_type() )
	{
	case pxtnWOICE_PCM : color = _color_pcm; break;// 0xfff0f0f0; break;
	case pxtnWOICE_PTV : color = _color_ptv; break;// 0xffffff80; break;
	case pxtnWOICE_PTN : color = _color_ptn; break;// 0xffd0d0ff; break;
	case pxtnWOICE_OGGV: color = _color_ogg; break;// 0xffffdd66; break;
	}

	surface = SURF_WOICENAME + w / 10;
	y       = (w % 10) * WOICENAME_HEIGHT;
	rc.t    = y;
	rc.b    = rc.t + WOICENAME_HEIGHT;

	static const char *_default_name = "-PXTONE WOICE-";

	const char *p_name = p_w->get_name_buf( NULL );
	if( !p_name ) p_name = _default_name;
	g_dxdraw->tex_glyph_color( surface, color          );
	g_dxdraw->tex_glyph_text ( surface, p_name, &rc, 1 );
}

void if_WoiceTray_RedrawAllName( const uxDS& new_font_name )
{
	for( int32_t s = SURF_WOICENAME; s < SURF_WOICENAME+10; s++ )
	{
		if( new_font_name ) g_dxdraw->tex_glyph_another_font( s, new_font_name, 12, false );
		g_dxdraw->tex_glyph_clear( s );
		g_dxdraw->tex_Clear      ( s );
	}

	int32_t enable_num = g_pxtn->Woice_Num();
	for( int32_t i = 0; i < enable_num; i++ ) if_WoiceTray_RedrawName( i );
}

void if_WoiceTray_Put()
{
	float    xpos      ;
	float    ypos      ;
	float    offset    ;
	int32_t  enable_num;
	int32_t  focus     ;

	fRECT rcTile;
	fRECT rc_tray     = {368,  0,512,160};
	fRECT rcFocus     = {384,224,512,240};
	fRECT rc_name_box = {384,240,512,256};

	fRECT rcName      = {0,0,WOICENAME_WIDTH,0};
	int32_t  name_surf;

	// ネームトレイ
	rcTile   = _rcField;
	offset   = g_ScrlWoiceV.GetOffset();
	if_gen_tile_v( &rcTile, &rc_tray, 1, offset, SURF_FIELDS );

	enable_num = g_pxtn->Woice_Num();
	xpos       = _rcField.l + 16;
	focus      = WoiceFocus_Get(); if( focus < 0 ) return;

	for( int32_t w = 0; w < enable_num; w++ )
	{
		ypos = _rcField.t + w * UNIT_HEIGHT - offset;

		if( ypos > _rcField.t - UNIT_HEIGHT && ypos < _rcField.b )
		{
			g_dxdraw->tex_Put_View( xpos, ypos, &rc_name_box, SURF_FIELDS );

			if( focus == w ) g_dxdraw->tex_Put_View( xpos, ypos, &rcFocus,  SURF_FIELDS );

			rcName.t  = ( w % 10 ) * WOICENAME_HEIGHT;
			rcName.b  = rcName.t   + WOICENAME_HEIGHT - 2;
			name_surf = SURF_WOICENAME + w / 10;
			g_dxdraw->tex_Put_View( xpos + 24, ypos + 2, &rcName, name_surf );
			if_gen_num6_clip      ( xpos +  4, ypos + 4, w, 2, &_rcField    );
		}
	}
}

bool if_WoiceTray_GetNamePosition( int32_t w, float *px, float *py )
{
	float offset;

	offset = g_ScrlWoiceV.GetOffset();
	if( w >= g_pxtn->Woice_Num() ) return false;
	*px = _rcField.l + 16;
	*py = _rcField.t + w * WOICE_HEIGHT - offset;

	return true;
}

void if_WoiceTray_JustScroll()
{
	float scroll_y;

	if_WoiceTray_SetRect( &_rc_view );

	int32_t i     = WoiceFocus_Get(); if( i < 0 ) return;
	scroll_y  = UNIT_HEIGHT * i + WOICE_HEIGHT / 2;
	scroll_y -= ( _rcField.b - _rcField.t )    / 2;

	g_ScrlWoiceV.SetOffset( scroll_y );
}
